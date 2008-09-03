/* $Id: rmutil.c,v 1.162 2008/09/03 00:00:35 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2008 by Timothy P. Hunter
| Name:     rmutil.c
| Author:   Tim Hunter
| Purpose:  Utility functions for RMagick
\============================================================================*/

#include "rmagick.h"
#include <errno.h>

static const char *ComplianceType_name(ComplianceType *);
static const char *StyleType_name(StyleType);
static const char *StretchType_name(StretchType);
static void Color_Name_to_PixelPacket(PixelPacket *, VALUE);
static VALUE Enum_type_values(VALUE);
static VALUE Enum_type_inspect(VALUE);
static void handle_exception(ExceptionInfo *, Image *, ErrorRetention);
static VALUE Pixel_from_MagickPixelPacket(const MagickPixelPacket *);

#define ENUMERATORS_CLASS_VAR "@@enumerators"

/*
    Extern:     magick_safe_malloc, magick_malloc, magick_free, magick_realloc
    Purpose:    ImageMagick versions of standard memory routines.
    Notes:      use when managing memory that ImageMagick may have
                allocated or may free.

                If malloc fails, it raises an exception.

                magick_safe_malloc and magick_safe_realloc prevent exceptions
                caused by integer overflow. Added in 6.3.5-9 but backwards
                compatible with prior releases.
*/
void *
magick_safe_malloc(const size_t count, const size_t quantum)
{
#if defined(HAVE_ACQUIREQUANTUMMEMORY)
    void *ptr;

    ptr = AcquireQuantumMemory(count, quantum);
    if (!ptr)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    return ptr;
#else

    // Provide an implementation of AcquireQuantumMemory in releases prior to 6.3.5-9.
    size_t size = count * quantum;

    if (count == 0 || quantum != (size/count))
    {
        rb_raise(rb_eRuntimeError, "integer overflow detected in memory size computation. "
               "Probable image corruption.");
    }
    return magick_malloc(size);
#endif
}


void *
magick_malloc(const size_t size)
{
    void *ptr;
    ptr = AcquireMagickMemory(size);
    if (!ptr)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    return ptr;
}


void
magick_free(void *ptr)
{
    (void) RelinquishMagickMemory(ptr);
}


void *
magick_safe_realloc(void *memory, const size_t count, const size_t quantum)
{
#if defined(HAVE_RESIZEQUANTUMMEMORY)
    void *v;
    v = ResizeQuantumMemory(memory, count, quantum);
    if (!v)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    return v;
#else
    // Provide an implementation of ResizeQuantumMemory in releases prior to 6.3.5-9.
    size_t size = count * quantum;
    if (count == 0 || quantum != (size/count))
    {
        rb_raise(rb_eRuntimeError, "integer overflow detected in memory size computation. "
               "Probable image corruption.");
    }
    return magick_realloc(memory, size);
#endif
}


void *
magick_realloc(void *ptr, const size_t size)
{
    void *v;
    v = ResizeMagickMemory(ptr, size);
    if (!v)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    return v;
}


/*
    Extern:     magick_clone_string
    Purpose:    make a copy of a string in malloc'd memory
    Notes:      Any existing string pointed to by *new_str is freed.
                CloneString asserts if no memory. No need to check
                its return value.

*/
void
 magick_clone_string(char **new_str, const char *str)
{
    (void) CloneString(new_str, str);
}


/*
 *  Extern:     rm_strcasecmp(s1, s2)
 *  Purpose:    compare s1 and s2 ignoring case
 *  Returns:    same as strcmp(3)
*/
int
rm_strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if (toupper(*s1) != toupper(*s2))
        {
            break;
        }
        s1 += 1;
        s2 += 1;
    }
    return (int)(*s1 - *s2);
}


/*
 *  Extern:     rm_strncasecmp(s1, s2, n)
 *  Purpose:    compare s1 and s2 ignoring case
 *  Returns:    same as strcmp(3)
*/
int
rm_strncasecmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
    {
        return 0;
    }
    while (toupper(*s1) == toupper(*s2))
    {
        if (--n == 0 || *s1 == '\0')
        {
            return 0;
        }
        s1 += 1;
        s2 += 1;
    }
    return (int)(*s1 - *s2);
}


/*
 *  Extern:     rm_check_ary_len(ary, len)
 *  Purpose:    raise exception if array too short
*/
void
rm_check_ary_len(VALUE ary, long len)
{
    if (RARRAY_LEN(ary) < len)
    {
        rb_raise(rb_eIndexError, "not enough elements in array - expecting %ld, got %ld",
                        len, (long)RARRAY_LEN(ary));
    }
}


/*
    Extern:     rm_check_destroyed
    Purpose:    raise an error if the image has been destroyed
*/
Image *
rm_check_destroyed(VALUE obj)
{
    Image *image;

    Data_Get_Struct(obj, Image, image);
    if (!image)
    {
        rb_raise(Class_DestroyedImageError, "destroyed image");
    }

    return image;
}


/*
    Extern:     rm_check_frozen
    Purpose:    raise an error if the image has been destroyed or is frozen
*/
Image *
rm_check_frozen(VALUE obj)
{
    Image *image = rm_check_destroyed(obj);
    rb_check_frozen(obj);
    return image;
}


/*
    Extern:     rm_no_freeze(obj)
    Purpose:    overrides freeze in classes that can't be frozen.
*/
VALUE
rm_no_freeze(VALUE obj)
{
    rb_raise(rb_eTypeError, "can't freeze %s", rb_class2name(CLASS_OF(obj)));
    return (VALUE)0;
}


/*
    Extern:     rm_to_s
    Purpose:    return obj.to_s, or obj if obj is already a string.
*/
VALUE
rm_to_s(VALUE obj)
{

    if (TYPE(obj) != T_STRING)
    {
        return rb_funcall(obj, rm_ID_to_s, 0);
    }
    return obj;
}


/*
    Extern:     rm_str2cstr(str, &len);
    Purpose:    Supply our own version of the "obsolete" rb_str2cstr.
*/
char *
rm_str2cstr(VALUE str, long *len)
{
    StringValue(str);
    if (len)
    {
        *len = RSTRING_LEN(str);
    }
    return RSTRING_PTR(str);
}


/*
 *  Static:     arg_is_number
 *  Purpose:    Try to convert the argument to a double,
 *              raise an exception if fail.
*/
static VALUE
arg_is_number(VALUE arg)
{
    double d;
    d = NUM2DBL(arg);
    d = d;      // satisfy icc
    return arg;
}


/*
 *  Static:     rescue_not_str
 *  Purpose:    called when `rb_str_to_str' raised an exception below
*/
static VALUE
rescue_not_str(VALUE arg)
{
    rb_raise(rb_eTypeError, "argument must be a number or a string in the form 'NN%%' (%s given)",
            rb_class2name(CLASS_OF(arg)));
    return (VALUE)0;
}


/*
 *  Extern:     rm_percentage(obj)
 *  Purpose:    Return a double between 0.0 and 1.0, inclusive.
 *              If the argument is a number convert to a Float object,
 *              otherwise it's supposed to be a string in the form "NN%".
 *              Convert to a number and then to a Float.
*/
double
rm_percentage(VALUE arg)
{
    double pct;
    long pct_long;
    char *pct_str, *end;
    int not_num;

    // Try to convert the argument to a number. If failure, sets not_num to non-zero.
    (void) rb_protect(arg_is_number, arg, &not_num);

    if (not_num)
    {
        arg = rb_rescue(rb_str_to_str, arg, rescue_not_str, arg);
        pct_str = StringValuePtr(arg);
        errno = 0;
        pct_long = strtol(pct_str, &end, 10);
        if (errno == ERANGE)
        {
            rb_raise(rb_eRangeError, "`%s' out of range", pct_str);
        }
        if (*end != '\0' && *end != '%')
        {
            rb_raise(rb_eArgError, "expected percentage, got `%s'", pct_str);
        }

        if (*end == '%' && pct_long != 0)
        {
            pct = ((double)pct_long) / 100.0;
        }
        else
        {
            pct = (double) pct_long;
        }
        if (pct < 0.0)
        {
            rb_raise(rb_eArgError, "percentages may not be negative (got `%s')", pct_str);
        }
    }
    else
    {
        pct = NUM2DBL(arg);
        if (pct < 0.0)
        {
            rb_raise(rb_eArgError, "percentages may not be negative (got `%g')", pct);
        }
    }

    return pct;
}


/*
    Static:     check_num2dbl
    Purpose:    return 0 if rb_num2dbl doesn't raise an exception
 */
static VALUE
check_num2dbl(VALUE obj)
{
    (void) rb_num2dbl(obj);
    return INT2FIX(1);
}


/*
    Static:     rescue_not_dbl
    Purpose:    called if rb_num2dbl raises an exception
 */
static VALUE
rescue_not_dbl(VALUE ignored)
{
    ignored = ignored;      // defeat gcc message
    return INT2FIX(0);
}


/*
    Extern:     rm_check_num2dbl
    Purpose:    Return 1 if the object can be converted to a double, 0 otherwise.
*/
int
rm_check_num2dbl(VALUE obj)
{
    return FIX2INT(rb_rescue(check_num2dbl, obj, rescue_not_dbl, (VALUE)0));
}


/*
 *  Extern:     rm_str_to_pct
 *  Purpose:    Given a string in the form NN% return the corresponding double.
 *
*/
double
rm_str_to_pct(VALUE str)
{
    long pct;
    char *pct_str, *end;

    str = rb_rescue(rb_str_to_str, str, rescue_not_str, str);
    pct_str = StringValuePtr(str);
    errno = 0;
    pct = strtol(pct_str, &end, 10);

    if (errno == ERANGE)
    {
        rb_raise(rb_eRangeError, "`%s' out of range", pct_str);
    }
    if (*end != '%')
    {
        rb_raise(rb_eArgError, "expected percentage, got `%s'", pct_str);
    }
    if (pct < 0L)
    {
        rb_raise(rb_eArgError, "percentages may not be negative (got `%s')", pct_str);
    }

    return pct / 100.0;
}


/*
 *  Extern:     rm_fuzz_to_dbl(obj)
 *  Purpose:    If the argument is a number, convert it to a double.
 *              Otherwise it's supposed to be a string in the form 'NN%'.
 *              Return a percentage of QuantumRange.
 *  Notes:      Called from Image#fuzz= and Info#fuzz=
*/
double
rm_fuzz_to_dbl(VALUE fuzz_arg)
{
    double fuzz;
    char *fuzz_str, *end;
    int not_num;

    // Try to convert the argument to a number. If failure, sets not_num to non-zero.
    (void) rb_protect(arg_is_number, fuzz_arg, &not_num);

    if (not_num)
    {
        // Convert to string, issue error message if failure.
        fuzz_arg = rb_rescue(rb_str_to_str, fuzz_arg, rescue_not_str, fuzz_arg);
        fuzz_str = StringValuePtr(fuzz_arg);
        errno = 0;
        fuzz = strtod(fuzz_str, &end);
        if (errno == ERANGE)
        {
            rb_raise(rb_eRangeError, "`%s' out of range", fuzz_str);
        }
        if(*end == '%')
        {
            if (fuzz < 0.0)
            {
                rb_raise(rb_eArgError, "percentages may not be negative (got `%s')", fuzz_str);
            }
            fuzz = (fuzz * QuantumRange) / 100.0;
        }
        else if(*end != '\0')
        {
            rb_raise(rb_eArgError, "expected percentage, got `%s'", fuzz_str);
        }
    }
    else
    {
        fuzz = NUM2DBL(fuzz_arg);
        if (fuzz < 0.0)
        {
            rb_raise(rb_eArgError, "fuzz may not be negative (got `%g')", fuzz);
        }
    }

    return fuzz;
}


/*
    Extern:     rm_app2quantum
    Purpose:    Convert a application-supplied number to a Quantum. If the object
                is a Float, truncate it before converting.
    Notes:      Ruby says that 2147483647.5 doesn't fit into an unsigned long.
                If you truncate it, it works.
                Should use this only when the input value is possibly subject
                to this problem.
*/
Quantum
rm_app2quantum(VALUE obj)
{
    volatile VALUE v = obj;

    if (TYPE(obj) == T_FLOAT)
    {
        v = rb_funcall(obj, rm_ID_to_i, 0);
    }

    return NUM2QUANTUM(v);
}


/*
    Extern:     rm_cur_image
    Purpose:    Sends the "cur_image" method to the object. If 'img'
                is an ImageList, then cur_image is self[@scene].
                If 'img' is an image, then cur_image is simply
                'self'.
    Returns:    the return value from "cur_image"
*/
VALUE
rm_cur_image(VALUE img)
{
    return rb_funcall(img, rm_ID_cur_image, 0);
}


/*
    Method:     Magick::PrimaryInfo#to_s
    Purpose:    Create a string representation of a Magick::PrimaryInfo
*/
VALUE
PrimaryInfo_to_s(VALUE self)
{
    PrimaryInfo pi;
    char buff[100];

    PrimaryInfo_to_PrimaryInfo(&pi, self);
    sprintf(buff, "x=%g, y=%g, z=%g", pi.x, pi.y, pi.z);
    return rb_str_new2(buff);
}


/*
    Method:     Magick::Chromaticity#to_s
    Purpose:    Create a string representation of a Magick::Chromaticity
*/
VALUE
ChromaticityInfo_to_s(VALUE self)
{
    ChromaticityInfo ci;
    char buff[200];

    ChromaticityInfo_to_ChromaticityInfo(&ci, self);
    sprintf(buff, "red_primary=(x=%g,y=%g) "
                  "green_primary=(x=%g,y=%g) "
                  "blue_primary=(x=%g,y=%g) "
                  "white_point=(x=%g,y=%g) ",
                  ci.red_primary.x, ci.red_primary.y,
                  ci.green_primary.x, ci.green_primary.y,
                  ci.blue_primary.x, ci.blue_primary.y,
                  ci.white_point.x, ci.white_point.y);
    return rb_str_new2(buff);
}


/*
    Method:     Magick::Pixel#to_s
    Purpose:    Create a string representation of a Magick::Pixel
*/
VALUE
Pixel_to_s(VALUE self)
{
    Pixel *pixel;
    char buff[100];

    Data_Get_Struct(self, Pixel, pixel);
    sprintf(buff, "red=" QuantumFormat ", green=" QuantumFormat ", blue=" QuantumFormat ", opacity=" QuantumFormat
          , pixel->red, pixel->green, pixel->blue, pixel->opacity);
    return rb_str_new2(buff);
}


/*
    Method:     Magick::Pixel.from_color(string)
    Purpose:    Construct an Magick::Pixel corresponding to the given color name.
    Notes:      the "inverse" is Image *#to_color, b/c the conversion of a pixel
                to a color name requires both a color depth and if the opacity
                value has meaning (i.e. whether image->matte == True or not).

                Also see Magick::Pixel#to_color, below.
*/
VALUE
Pixel_from_color(VALUE class, VALUE name)
{
    PixelPacket pp;
    ExceptionInfo exception;
    MagickBooleanType okay;

    class = class;      // defeat "never referenced" message from icc

    GetExceptionInfo(&exception);
    okay = QueryColorDatabase(StringValuePtr(name), &pp, &exception);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    if (!okay)
    {
        rb_raise(rb_eArgError, "invalid color name: %s", StringValuePtr(name));
    }

    return Pixel_from_PixelPacket(&pp);
}


/*
    Static:     rm_set_magick_pixel_packet
    Purpose:    Convert a PixelPacket to a MagickPixelPacket
    Notes:      Same code as the private function SetMagickPixelPacket
                in ImageMagick.
*/
static void
rm_set_magick_pixel_packet(Pixel *pixel, IndexPacket *index_packet, MagickPixelPacket *pp)
{
    pp->red     = (MagickRealType) pixel->red;
    pp->green   = (MagickRealType) pixel->green;
    pp->blue    = (MagickRealType) pixel->blue;
    pp->opacity = (MagickRealType) (pp->matte ? pixel->opacity : OpaqueOpacity);
    pp->index   = (MagickRealType) ((pp->colorspace == CMYKColorspace) && (index_packet ? *index_packet : 0));
}


/*
    Method:     Magick::Pixel#to_color(compliance=AllCompliance, matte=false,
                                       depth=QuantumDepth, hex=false)
    Purpose:    return the color name corresponding to the pixel values
    Notes:      the conversion respects the value of the 'opacity' field
                in the Pixel.
*/
VALUE
Pixel_to_color(int argc, VALUE *argv, VALUE self)
{
    Info *info;
    Image *image;
    Pixel *pixel;
    MagickPixelPacket mpp;
    MagickBooleanType hex = MagickFalse;
    char name[MaxTextExtent];
    ExceptionInfo exception;
    ComplianceType compliance = AllCompliance;
    unsigned int matte = MagickFalse;
    unsigned int depth = QuantumDepth;

    switch (argc)
    {
        case 4:
            hex = RTEST(argv[3]);
        case 3:
            depth = NUM2UINT(argv[2]);

            // Ensure depth is appropriate for the way xMagick was compiled.
            switch (depth)
            {
                case 8:
#if QuantumDepth == 16 || QuantumDepth == 32
                case 16:
#endif
#if QuantumDepth == 32
                case 32:
#endif
                    break;
                default:
                    rb_raise(rb_eArgError, "invalid depth (%d)", depth);
                    break;
            }
       case 2:
            matte = RTEST(argv[1]);
        case 1:
            VALUE_TO_ENUM(argv[0], compliance, ComplianceType);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
    }

    Data_Get_Struct(self, Pixel, pixel);

    info = CloneImageInfo(NULL);
    image = AllocateImage(info);
    image->depth = depth;
    image->matte = matte;
    (void) DestroyImageInfo(info);

    GetMagickPixelPacket(image, &mpp);
    rm_set_magick_pixel_packet(pixel, NULL, &mpp);

    GetExceptionInfo(&exception);

#if defined(HAVE_NEW_QUERYMAGICKCOLORNAME)
    // Support for hex-format color names moved out of QueryMagickColorname
    // in 6.4.1-9. The 'hex' argument was removed as well.
    if (hex)
    {
        if (compliance == XPMCompliance)
        {
            mpp.matte = MagickFalse;
            mpp.depth = (unsigned long) min(1.0 * image->depth, 16.0);
        }
        (void) GetColorTuple(&mpp, MagickTrue, name);
    }
    else
    {
        (void) QueryMagickColorname(image, &mpp, compliance, name, &exception);
    }
#else
    (void) QueryMagickColorname(image, &mpp, compliance, hex, name, &exception);
#endif
    (void) DestroyImage(image);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    // Always return a string, even if it's ""
    return rb_str_new2(name);
}


/*
    Method:     Pixel#to_HSL    *** DEPRECATED ***
    Purpose:    Converts an RGB pixel to the array
                [hue, saturation, luminosity].
*/
VALUE
Pixel_to_HSL(VALUE self)
{
    Pixel *pixel;
    double hue, saturation, luminosity;
    volatile VALUE hsl;

    Data_Get_Struct(self, Pixel, pixel);
#if defined(HAVE_CONVERTRGBTOHSL)
    rb_warning("Pixel#to_HSL is deprecated; use to_hsla");
    ConvertRGBToHSL(pixel->red, pixel->green, pixel->blue, &hue, &saturation, &luminosity);
#else
    TransformHSL(pixel->red, pixel->green, pixel->blue, &hue, &saturation, &luminosity);
#endif

    hsl = rb_ary_new3(3, rb_float_new(hue), rb_float_new(saturation),
                      rb_float_new(luminosity));

    return hsl;
}


/*
    Method:     Pixel.from_HSL  *** DEPRECATED ***
    Purpose:    Constructs an RGB pixel from the array
                [hue, saturation, luminosity].
*/
VALUE
Pixel_from_HSL(VALUE class, VALUE hsl)
{
    PixelPacket rgb;
    double hue, saturation, luminosity;

    class = class;      // defeat "never referenced" message from icc
    memset(&rgb, 0, sizeof(rgb));

    hsl = rb_Array(hsl);    // Ensure array
    if (RARRAY_LEN(hsl) < 3)
    {
        rb_raise(rb_eArgError, "array argument must have at least 3 elements");
    }

    hue        = NUM2DBL(rb_ary_entry(hsl, 0));
    saturation = NUM2DBL(rb_ary_entry(hsl, 1));
    luminosity = NUM2DBL(rb_ary_entry(hsl, 2));

#if defined(HAVE_CONVERTHSLTORGB)
    rb_warning("Pixel#from_HSL is deprecated; use from_hsla");
    ConvertHSLToRGB(hue, saturation, luminosity,
                 &rgb.red, &rgb.green, &rgb.blue);
#else
    HSLTransform(hue, saturation, luminosity,
                 &rgb.red, &rgb.green, &rgb.blue);
#endif
    return Pixel_from_PixelPacket(&rgb);
}


/*
    Method:     Pixel#from_hsla(hue, saturation, lightness, alpha=1)
    Purpose:    Replace brain-dead from_HSL, above.
    Notes:      0 <= hue < 360, 0 <= saturation <= 1, 0 <= lightness <= 1
                0 <= alpha <= 1 (0 is transparent, 1 is opaque)
*/
VALUE
Pixel_from_hsla(int argc, VALUE *argv, VALUE class)
{
    double h, s, l, a = 1.0;
    MagickPixelPacket pp;
    ExceptionInfo exception;
    char name[50];
    MagickBooleanType alpha = MagickFalse;

    class = class;          // defeat "unused parameter" message.

    switch (argc)
    {
        case 4:
            a = NUM2DBL(argv[3]);
            alpha = MagickTrue;
        case 3:
            l = NUM2DBL(argv[2]);
            s = NUM2DBL(argv[1]);
            h = NUM2DBL(argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 3 or 4)", argc);
            break;
    }

    if (alpha && (a < 0.0 || a > 1.0))
    {
        rb_raise(rb_eRangeError, "alpha %g out of range [0.0, 1.0]", a);
    }
    if (l < 0.0 || l > 100.0)
    {
        rb_raise(rb_eRangeError, "lightness %g out of range [0.0, 100.0]", l);
    }
    if (s < 0.0 || s > 100.0)
    {
        rb_raise(rb_eRangeError, "saturation %g out of range [0.0, 100.0]", s);
    }
    if (h < 0.0 || h >= 360.0)
    {
        rb_raise(rb_eRangeError, "hue %g out of range [0.0, 360.0)", h);
    }

    memset(name, 0, sizeof(name));
    if (alpha)
    {
        sprintf(name, "hsla(%-2.1f,%-2.1f,%-2.1f,%-2.1f)", h, s, l, a);
    }
    else
    {
        sprintf(name, "hsl(%-2.1f,%-2.1f,%-2.1f)", h, s, l);
    }

    GetExceptionInfo(&exception);

    (void) QueryMagickColor(name, &pp, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return Pixel_from_MagickPixelPacket(&pp);
}


/*
    Method:     Pixel#to_hsla()
    Purpose:    Replace brain-dead to_HSL, above.
    Notes:      Returns [hue, saturation, lightness, alpha] in the same ranges as from_hsla()
*/
VALUE
Pixel_to_hsla(VALUE self)
{
    double hue, sat, lum, alpha;
    Pixel *pixel;
    volatile VALUE hsla;

    Data_Get_Struct(self, Pixel, pixel);

#if defined(HAVE_CONVERTRGBTOHSL)
    ConvertRGBToHSL(pixel->red, pixel->green, pixel->blue, &hue, &sat, &lum);
#else
    TransformHSL(pixel->red, pixel->green, pixel->blue, &hue, &sat, &lum);
#endif
    hue *= 360.0;
    sat *= 100.0;
    lum *= 100.0;

    if (pixel->opacity == OpaqueOpacity)
    {
        alpha = 1.0;
    }
    else if (pixel->opacity == TransparentOpacity)
    {
        alpha = 0.0;
    }
    else
    {
        alpha = ROUND_TO_QUANTUM(QuantumRange - (pixel->opacity / QuantumRange));
    }

    hsla = rb_ary_new3(4, rb_float_new(hue), rb_float_new(sat), rb_float_new(lum), rb_float_new(alpha));
    return hsla;
}


/*
    Method:     Pixel#eql?
    Purpose:    For use with Hash
*/
VALUE
Pixel_eql_q(VALUE self, VALUE other)
{
    return NUM2INT(Pixel_spaceship(self, other)) == 0 ? Qtrue : Qfalse;
}


/*
    Method:  Pixel#fcmp(other[, fuzz[, colorspace]])
    Purpose: Compare pixel values for equality
*/
VALUE
Pixel_fcmp(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    Info *info;

    Pixel *this, *that;
    ColorspaceType colorspace = RGBColorspace;
    double fuzz = 0.0;
    unsigned int equal;

    switch (argc)
    {
        case 3:
            VALUE_TO_ENUM(argv[2], colorspace, ColorspaceType);
        case 2:
            fuzz = NUM2DBL(argv[1]);
        case 1:
            // Allow 1 argument
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 3)", argc);
            break;
    }

    Data_Get_Struct(self, Pixel, this);
    Data_Get_Struct(argv[0], Pixel, that);

    // The IsColorSimilar function expects to get the
    // colorspace and fuzz parameters from an Image structure.

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    image = AllocateImage(info);

    // Delete Info now in case we have to raise an exception
    (void) DestroyImageInfo(info);

    if (!image)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    image->colorspace = colorspace;
    image->fuzz = fuzz;

    equal = IsColorSimilar(image, this, that);
    (void) DestroyImage(image);

    return equal ? Qtrue : Qfalse;
}


/*
    Method:     Pixel#hash
    Notes:      INT2FIX left-shifts 1 bit. Sacrifice 1 bit
                from the opacity attribute to the FIXNUM_FLAG.
*/
VALUE
Pixel_hash(VALUE self)
{
    Pixel *pixel;
    unsigned int hash;

    Data_Get_Struct(self, Pixel, pixel);

    hash  = ScaleQuantumToChar(pixel->red)   << 24;
    hash += ScaleQuantumToChar(pixel->green) << 16;
    hash += ScaleQuantumToChar(pixel->blue)  << 8;
    hash += ScaleQuantumToChar(pixel->opacity);
    hash >>= 1;

    return INT2FIX(hash);

}


/*
    Method:  Pixel#intensity
    Purpose: Return the "intensity" of a pixel
*/
VALUE
Pixel_intensity(VALUE self)
{
    Pixel *pixel;
    Quantum intensity;

    Data_Get_Struct(self, Pixel, pixel);

    intensity = ROUND_TO_QUANTUM((0.299*pixel->red)
                                + (0.587*pixel->green)
                                + (0.114*pixel->blue));

    return QUANTUM2NUM((unsigned long) intensity);
}


/*
    Methods:    Pixel RGBA attribute accessors
    Purpose:    Get/set Pixel attributes
    Note:       Pixel is Observable. Setters call changed, notify_observers
    Note:       Setters return their argument values for backward compatibility
                to when Pixel was a Struct class.
*/

DEF_ATTR_READER(Pixel, red, int)
DEF_ATTR_READER(Pixel, green, int)
DEF_ATTR_READER(Pixel, blue, int)
DEF_ATTR_READER(Pixel, opacity, int)
DEF_PIXEL_CHANNEL_WRITER(red)
DEF_PIXEL_CHANNEL_WRITER(green)
DEF_PIXEL_CHANNEL_WRITER(blue)
DEF_PIXEL_CHANNEL_WRITER(opacity)


/*
    Methods:    Pixel CMYK attribute accessors
    Purpose:    Get/set Pixel attributes
    Note:       Pixel is Observable. Setters call changed, notify_observers
    Note:       Setters return their argument values for backward compatibility
                to when Pixel was a Struct class.
*/
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(cyan, red)
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(magenta, green)
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(yellow, blue)
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(black, opacity)


/*
    Method:     Pixel#<=>
    Purpose:    Support Comparable mixin
*/
VALUE
Pixel_spaceship(VALUE self, VALUE other)
{
    Pixel *this, *that;

    Data_Get_Struct(self, Pixel, this);
    Data_Get_Struct(other, Pixel, that);

    if (this->red != that->red)
    {
        return INT2NUM((this->red - that->red)/abs(this->red - that->red));
    }
    else if(this->green != that->green)
    {
        return INT2NUM((this->green - that->green)/abs(this->green - that->green));
    }
    else if(this->blue != that->blue)
    {
        return INT2NUM((this->blue - that->blue)/abs(this->blue - that->blue));
    }
    else if(this->opacity != that->opacity)
    {
        return INT2NUM((this->opacity - that->opacity)/abs(this->opacity - that->opacity));
    }

    // Values are equal, check class.

    return rb_funcall(CLASS_OF(self), rb_intern("<=>"), 1, CLASS_OF(other));

}


/*
    Static:     destroy_Pixel
    Purpose:    Free the storage associated with a Pixel object
*/
static void
destroy_Pixel(Pixel *pixel)
{
    xfree(pixel);
}


/*
    Extern:     Pixel_alloc
    Purpose:    Allocate a Pixel object
*/
VALUE
Pixel_alloc(VALUE class)
{
    Pixel *pixel;

    pixel = ALLOC(Pixel);
    memset(pixel, '\0', sizeof(Pixel));
    return Data_Wrap_Struct(class, NULL, destroy_Pixel, pixel);
}


/*
    Method:     Pixel#initialize(red=0,green=0,blue=0,opacity=0)
    Notes:      For backward compatibility, arguments may be nil.
*/
VALUE
Pixel_initialize(int argc, VALUE *argv, VALUE self)
{
    Pixel *pixel;

    Data_Get_Struct(self, Pixel, pixel);

    switch(argc)
    {
        case 4:
            if (argv[3] != Qnil)
            {
                pixel->opacity = APP2QUANTUM(argv[3]);
            }
        case 3:
            if (argv[2] != Qnil)
            {
                pixel->blue = APP2QUANTUM(argv[2]);
            }
        case 2:
            if (argv[1] != Qnil)
            {
                pixel->green = APP2QUANTUM(argv[1]);
            }
        case 1:
            if (argv[0] != Qnil)
            {
                pixel->red = APP2QUANTUM(argv[0]);
            }
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 4)", argc);
    }

    return self;
}


/*
    Method: Pixel#===
    Purpose:    "Case equal" operator for Pixel
*/

VALUE
Pixel_case_eq(VALUE self, VALUE other)
{
    Pixel *this, *that;

    if (CLASS_OF(self) == CLASS_OF(other))
    {
        Data_Get_Struct(self, Pixel, this);
        Data_Get_Struct(other, Pixel, that);
        return (this->red == that->red
            && this->blue == that->blue
            && this->green == that->green
            && this->opacity == that->opacity) ? Qtrue : Qfalse;
    }

    return Qfalse;
}


VALUE
Pixel_dup(VALUE self)
{
    Pixel *pixel;
    volatile VALUE dup;

    pixel = ALLOC(Pixel);
    memset(pixel, '\0', sizeof(Pixel));
    dup = Data_Wrap_Struct(CLASS_OF(self), NULL, destroy_Pixel, pixel);
    if (rb_obj_tainted(self))
    {
        (void) rb_obj_taint(dup);
    }
    return rb_funcall(dup, rm_ID_initialize_copy, 1, self);
}


/*
    Method:     Pixel#clone
    Notes:      see dup, init_copy
*/
VALUE
Pixel_clone(VALUE self)
{
    volatile VALUE clone;

    clone = Pixel_dup(self);
    if (OBJ_FROZEN(self))
    {
        OBJ_FREEZE(clone);
    }

    return clone;
}


/*
    Method:     Pixel#initialize_copy
    Purpose:    initialize clone, dup methods
*/
VALUE
Pixel_init_copy(VALUE self, VALUE orig)
{
    Pixel *copy, *original;

    Data_Get_Struct(orig, Pixel, original);
    Data_Get_Struct(self, Pixel, copy);

    *copy = *original;

    return self;
}


/*
    Method:     Magick::Rectangle#to_s
    Purpose:    Create a string representation of a Magick::Rectangle
*/
VALUE
RectangleInfo_to_s(VALUE self)
{
    RectangleInfo rect;
    char buff[100];

    Rectangle_to_RectangleInfo(&rect, self);
    sprintf(buff, "width=%lu, height=%lu, x=%ld, y=%ld"
          , rect.width, rect.height, rect.x, rect.y);
    return rb_str_new2(buff);
}


/*
    Method:     Magick::SegmentInfo#to_s
    Purpose:    Create a string representation of a Magick::Segment
*/
VALUE
SegmentInfo_to_s(VALUE self)
{
    SegmentInfo segment;
    char buff[100];

    Segment_to_SegmentInfo(&segment, self);
    sprintf(buff, "x1=%g, y1=%g, x2=%g, y2=%g"
          , segment.x1, segment.y1, segment.x2, segment.y2);
    return rb_str_new2(buff);
}


/*
    Extern:     PixelPacket_to_Color_Name
    Purpose:    Map the color intensity to a named color
    Returns:    the named color as a String
    Notes:      See below for the equivalent function that accepts an Info
                structure instead of an Image.
*/
VALUE
PixelPacket_to_Color_Name(Image *image, PixelPacket *color)
{
    char name[MaxTextExtent];
    ExceptionInfo exception;

    GetExceptionInfo(&exception);

    (void) QueryColorname(image, color, X11Compliance, name, &exception);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    return rb_str_new2(name);
}


/*
    Extern:     PixelPacket_to_Color_Name_Info
    Purpose:    Map the color intensity to a named color
    Returns:    the named color as a String
    Notes:      Accepts an Info structure instead of an Image (see above).
                Simply create an Image from the Info, call QueryColorname,
                and then destroy the Image.
                If the Info structure is NULL, creates a new one.

                Note that the default depth is always used, and the matte
                value is set to False, which means "don't use the alpha channel".
*/
VALUE
PixelPacket_to_Color_Name_Info(Info *info, PixelPacket *color)
{
    Image *image;
    Info *my_info;
    volatile VALUE color_name;

    my_info = info ? info : CloneImageInfo(NULL);

    image = AllocateImage(info);
    image->matte = MagickFalse;
    color_name = PixelPacket_to_Color_Name(image, color);
    (void) DestroyImage(image);
    if (!info)
    {
        (void) DestroyImageInfo(my_info);
    }

    return color_name;
}


/*
    Static:     Color_Name_to_PixelPacket
    Purpose:    Convert a color name to a PixelPacket
    Raises:     ArgumentError
*/
static void
Color_Name_to_PixelPacket(PixelPacket *color, VALUE name_arg)
{
    MagickBooleanType okay;
    char *name;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    name = StringValuePtr(name_arg);
    okay = QueryColorDatabase(name, color, &exception);
    (void) DestroyExceptionInfo(&exception);
    if (!okay)
    {
        rb_raise(rb_eArgError, "invalid color name %s", name);
    }
}


/*
    Extern:     AffineMatrix_to_AffineMatrix
    Purpose:    Convert a Magick::AffineMatrix object to a AffineMatrix structure.
    Notes:      If not initialized, the defaults are [sx,rx,ry,sy,tx,ty] = [1,0,0,1,0,0]
*/
void
AffineMatrix_to_AffineMatrix(AffineMatrix *am, VALUE st)
{
    volatile VALUE values, v;

    if (CLASS_OF(st) != Class_AffineMatrix)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(st)));
    }
    values = rb_funcall(st, rm_ID_values, 0);
    v = rb_ary_entry(values, 0);
    am->sx = v == Qnil ? 1.0 : NUM2DBL(v);
    v = rb_ary_entry(values, 1);
    am->rx = v == Qnil ? 0.0 : NUM2DBL(v);
    v = rb_ary_entry(values, 2);
    am->ry = v == Qnil ? 0.0 : NUM2DBL(v);
    v = rb_ary_entry(values, 3);
    am->sy = v == Qnil ? 1.0 : NUM2DBL(v);
    v = rb_ary_entry(values, 4);
    am->tx = v == Qnil ? 0.0 : NUM2DBL(v);
    v = rb_ary_entry(values, 5);
    am->ty = v == Qnil ? 0.0 : NUM2DBL(v);
}


/*
    Extern:     ClassType_new
    Purpose:    Construct a ClassType enum object for the specified value
*/
VALUE
ClassType_new(ClassType cls)
{
    const char *name;

    switch(cls)
    {
        default:
        case UndefinedClass:
            name = "UndefineClass";
            break;
        case DirectClass:
            name = "DirectClass";
            break;
        case PseudoClass:
            name = "PseudoClass";
            break;
    }

    return rm_enum_new(Class_ClassType, ID2SYM(rb_intern(name)), INT2FIX(cls));
}


/*
    Extern:     ColorspaceType_new
    Purpose:    construct a ColorspaceType enum object for the specified value
*/
VALUE
ColorspaceType_new(ColorspaceType cs)
{
    const char *name;

    switch(cs)
    {
        default:
        case UndefinedColorspace:
            name = "UndefinedColorspace";
            break;
        case RGBColorspace:
            name = "RGBColorspace";
            break;
        case GRAYColorspace:
            name = "GRAYColorspace";
            break;
        case TransparentColorspace:
            name = "TransparentColorspace";
            break;
        case OHTAColorspace:
            name = "OHTAColorspace";
            break;
        case XYZColorspace:
            name = "XYZColorspace";
            break;
        case YCbCrColorspace:
            name = "YCbCrColorspace";
            break;
        case YCCColorspace:
            name = "YCCColorspace";
            break;
        case YIQColorspace:
            name = "YIQColorspace";
            break;
        case YPbPrColorspace:
            name = "YPbPrColorspace";
            break;
        case YUVColorspace:
            name = "YUVColorspace";
            break;
        case CMYKColorspace:
            name = "CMYKColorspace";
            break;
        case sRGBColorspace:
            name = "sRGBColorspace";
            break;
        case HSLColorspace:
            name = "HSLColorspace";
            break;
        case HWBColorspace:
            name = "HWBColorspace";
            break;
        case HSBColorspace:
            name = "HSBColorspace";
            break;
        case LABColorspace:
            name = "LABColorspace";
            break;
        case Rec601YCbCrColorspace:
            name = "Rec601YCbCrColorspace";
            break;
        case Rec601LumaColorspace:
            name = "Rec601LumaColorspace";
            break;
        case Rec709LumaColorspace:
            name = "Rec709LumaColorspace";
            break;
        case Rec709YCbCrColorspace:
            name = "Rec709YCbCrColorspace";
            break;
        case LogColorspace:
            name = "LogColorspace";
            break;
#if defined(HAVE_ENUM_CMYCOLORSPACE)
        case CMYColorspace:
            name = "CMYColorspace";
            break;
#endif
    }

    return rm_enum_new(Class_ColorspaceType, ID2SYM(rb_intern(name)), INT2FIX(cs));

}


/*
 *  Static:     ComplianceType_new
    Purpose:    construct a ComplianceType enum object for the specified value
*/
static VALUE
ComplianceType_new(ComplianceType compliance)
{
    const char *name;

    // Turn off undefined bits
    compliance &= (SVGCompliance|X11Compliance|XPMCompliance);
    name = ComplianceType_name(&compliance);
    return rm_enum_new(Class_ComplianceType, ID2SYM(rb_intern(name)), INT2FIX(compliance));
}


/*
    Static:     CompositeOperator_new
    Purpose:    return the name of a CompositeOperator enum as a string
*/
static const char *
CompositeOperator_name(CompositeOperator op)
{
    switch (op)
    {
        ENUM_TO_NAME(UndefinedCompositeOp)
        ENUM_TO_NAME(NoCompositeOp)
        ENUM_TO_NAME(AddCompositeOp)
        ENUM_TO_NAME(AtopCompositeOp)
        ENUM_TO_NAME(BumpmapCompositeOp)
#if defined(HAVE_ENUM_CHANGEMASKCOMPOSITEOP)
        ENUM_TO_NAME(ChangeMaskCompositeOp)
#endif
        ENUM_TO_NAME(ClearCompositeOp)
        ENUM_TO_NAME(ColorBurnCompositeOp)
        ENUM_TO_NAME(BlendCompositeOp)
        ENUM_TO_NAME(ColorDodgeCompositeOp)
        ENUM_TO_NAME(ExclusionCompositeOp)
        ENUM_TO_NAME(HardLightCompositeOp)
        ENUM_TO_NAME(SoftLightCompositeOp)
        ENUM_TO_NAME(ColorizeCompositeOp)
        ENUM_TO_NAME(CopyBlueCompositeOp)
        ENUM_TO_NAME(CopyCompositeOp)
        ENUM_TO_NAME(CopyCyanCompositeOp)
        ENUM_TO_NAME(CopyMagentaCompositeOp)
        ENUM_TO_NAME(CopyYellowCompositeOp)
        ENUM_TO_NAME(CopyBlackCompositeOp)
        ENUM_TO_NAME(CopyGreenCompositeOp)
        ENUM_TO_NAME(CopyOpacityCompositeOp)
        ENUM_TO_NAME(CopyRedCompositeOp)
        ENUM_TO_NAME(DarkenCompositeOp)
#if defined(HAVE_ENUM_DIVIDECOMPOSITEOP)
        ENUM_TO_NAME(DivideCompositeOp)
#endif
        ENUM_TO_NAME(DstAtopCompositeOp)
        ENUM_TO_NAME(DstCompositeOp)
        ENUM_TO_NAME(DstInCompositeOp)
        ENUM_TO_NAME(DstOutCompositeOp)
        ENUM_TO_NAME(DstOverCompositeOp)
        ENUM_TO_NAME(DifferenceCompositeOp)
        ENUM_TO_NAME(DisplaceCompositeOp)
        ENUM_TO_NAME(DissolveCompositeOp)
        ENUM_TO_NAME(HueCompositeOp)
        ENUM_TO_NAME(InCompositeOp)
        ENUM_TO_NAME(LightenCompositeOp)
#if defined(HAVE_ENUM_LINEARLIGHTCOMPOSITEOP)
        ENUM_TO_NAME(LinearLightCompositeOp)
#endif
        ENUM_TO_NAME(LuminizeCompositeOp)
        ENUM_TO_NAME(MinusCompositeOp)
        ENUM_TO_NAME(ModulateCompositeOp)
        ENUM_TO_NAME(MultiplyCompositeOp)
        ENUM_TO_NAME(OutCompositeOp)
        ENUM_TO_NAME(OverCompositeOp)
        ENUM_TO_NAME(OverlayCompositeOp)
        ENUM_TO_NAME(PlusCompositeOp)
        ENUM_TO_NAME(ReplaceCompositeOp)
        ENUM_TO_NAME(SaturateCompositeOp)
        ENUM_TO_NAME(ScreenCompositeOp)
        ENUM_TO_NAME(SrcAtopCompositeOp)
        ENUM_TO_NAME(SrcCompositeOp)
        ENUM_TO_NAME(SrcInCompositeOp)
        ENUM_TO_NAME(SrcOutCompositeOp)
        ENUM_TO_NAME(SrcOverCompositeOp)
        ENUM_TO_NAME(SubtractCompositeOp)
        ENUM_TO_NAME(ThresholdCompositeOp)
        ENUM_TO_NAME(XorCompositeOp)
    }

    return "UndefinedCompositeOp";
}


/*
   External:    CompositeOperator_new
   Purpose:     Construct a CompositeOperator enum object for the specified value
*/
VALUE
CompositeOperator_new(CompositeOperator op)
{
    const char *name = CompositeOperator_name(op);
    return rm_enum_new(Class_CompositeOperator, ID2SYM(rb_intern(name)), INT2FIX(op));
}


/*
    Static:     CompressionType_name
    Purpose:    Return the name of a CompressionType enum as a string
*/
static const char *
CompressionType_name(CompressionType ct)
{
    switch (ct)
    {
        ENUM_TO_NAME(UndefinedCompression)
        ENUM_TO_NAME(NoCompression)
        ENUM_TO_NAME(BZipCompression)
#if defined(HAVE_ENUM_DXT1COMPRESSION)
        ENUM_TO_NAME(DXT1Compression)
#endif
#if defined(HAVE_ENUM_DXT3COMPRESSION)
        ENUM_TO_NAME(DXT3Compression)
#endif
#if defined(HAVE_ENUM_DXT5COMPRESSION)
        ENUM_TO_NAME(DXT5Compression)
#endif
        ENUM_TO_NAME(FaxCompression)
        ENUM_TO_NAME(Group4Compression)
        ENUM_TO_NAME(JPEGCompression)
        ENUM_TO_NAME(JPEG2000Compression)
        ENUM_TO_NAME(LosslessJPEGCompression)
        ENUM_TO_NAME(LZWCompression)
        ENUM_TO_NAME(RLECompression)
        ENUM_TO_NAME(ZipCompression)
    }

    return "UndefinedCompression";
}


/*
 * External:    CompressionType_new
   Purpose:     Construct a CompressionType enum object for the specified value
*/
VALUE
CompressionType_new(CompressionType ct)
{
    const char *name = CompressionType_name(ct);
    return rm_enum_new(Class_CompressionType, ID2SYM(rb_intern(name)), INT2FIX(ct));
}


/*
    Static:     DisposeType_name
    Purpose:    Return the name of a DisposeType enum as a string
*/
static const char *
DisposeType_name(DisposeType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedDispose)
        ENUM_TO_NAME(BackgroundDispose)
        ENUM_TO_NAME(NoneDispose)
        ENUM_TO_NAME(PreviousDispose)
    }

    return "UndefinedDispose";
}


/*
    External:   DisposeType.new
    Purpose:    Construct a DisposeType enum object for the specified value..new
*/
VALUE
DisposeType_new(DisposeType type)
{
    const char *name = DisposeType_name(type);
    return rm_enum_new(Class_DisposeType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/*
    Static:     FilterTypes_name
    Purpose:    Return the name of a FilterTypes enum as a string
*/
static const char *
FilterTypes_name(FilterTypes type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedFilter)
        ENUM_TO_NAME(PointFilter)
        ENUM_TO_NAME(BoxFilter)
        ENUM_TO_NAME(TriangleFilter)
        ENUM_TO_NAME(HermiteFilter)
        ENUM_TO_NAME(HanningFilter)
        ENUM_TO_NAME(HammingFilter)
        ENUM_TO_NAME(BlackmanFilter)
        ENUM_TO_NAME(GaussianFilter)
        ENUM_TO_NAME(QuadraticFilter)
        ENUM_TO_NAME(CubicFilter)
        ENUM_TO_NAME(CatromFilter)
        ENUM_TO_NAME(MitchellFilter)
        ENUM_TO_NAME(LanczosFilter)
        ENUM_TO_NAME(BesselFilter)
        ENUM_TO_NAME(SincFilter)
#if defined(HAVE_ENUM_KAISERFILTER)
        ENUM_TO_NAME(KaiserFilter)
#endif
#if defined(HAVE_ENUM_WELSHFILTER)
        ENUM_TO_NAME(WelshFilter)
#endif
#if defined(HAVE_ENUM_PARZENFILTER)
        ENUM_TO_NAME(ParzenFilter)
#endif
#if defined(HAVE_ENUM_LAGRANGEFILTER)
        ENUM_TO_NAME(LagrangeFilter)
#endif
#if defined(HAVE_ENUM_BOHMANFILTER)
        ENUM_TO_NAME(BohmanFilter)
#endif
#if defined(HAVE_ENUM_BARTLETTFILTER)
        ENUM_TO_NAME(BartlettFilter)
#endif
#if defined(HAVE_ENUM_SENTINELFILTER)
        // not a real filter name - defeat gcc warning message
        case SentinelFilter:
            break;
#endif
    }

    return "UndefinedFilter";
}


/*
    External:  FilterTypes.new
    Purpose: Construct an FilterTypes enum object for the specified value
*/
VALUE
FilterTypes_new(FilterTypes type)
{
    const char *name = FilterTypes_name(type);
    return rm_enum_new(Class_FilterTypes, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/*
    Static:     EndianType_name
    Purpose:    Return the name of a EndianType enum as a string
*/
static const char *
EndianType_name(EndianType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedEndian)
        ENUM_TO_NAME(LSBEndian)
        ENUM_TO_NAME(MSBEndian)
    }
    return "UndefinedEndian";
}


/*
    External:   EndianType.new
    Purpose:    Construct an EndianType enum object
*/
VALUE
EndianType_new(EndianType type)
{
    const char *name = EndianType_name(type);
    return rm_enum_new(Class_EndianType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/*
    Static:     GravityType_name
    Purpose:    Return the name of a GravityType enum as a string
*/
static const char *
GravityType_name(GravityType type)
{
    switch(type)
    {
        ENUM_TO_NAME(ForgetGravity)
        ENUM_TO_NAME(NorthWestGravity)
        ENUM_TO_NAME(NorthGravity)
        ENUM_TO_NAME(NorthEastGravity)
        ENUM_TO_NAME(WestGravity)
        ENUM_TO_NAME(CenterGravity)
        ENUM_TO_NAME(EastGravity)
        ENUM_TO_NAME(SouthWestGravity)
        ENUM_TO_NAME(SouthGravity)
        ENUM_TO_NAME(SouthEastGravity)
        ENUM_TO_NAME(StaticGravity)
    }

    // Defeat "duplicate case value" error.
    return "UndefinedGravity";
}


/*
    External:  GravityType.new
    Purpose: Construct an GravityType enum object for the specified value
*/
VALUE
GravityType_new(GravityType type)
{
    const char *name = GravityType_name(type);
    return rm_enum_new(Class_GravityType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/*
    Static:     ImageType_name
    Purpose:    Return the name of a ImageType enum as a string
*/
static const char *
ImageType_name(ImageType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedType)
        ENUM_TO_NAME(BilevelType)
        ENUM_TO_NAME(GrayscaleType)
        ENUM_TO_NAME(GrayscaleMatteType)
        ENUM_TO_NAME(PaletteType)
        ENUM_TO_NAME(PaletteMatteType)
        ENUM_TO_NAME(TrueColorType)
        ENUM_TO_NAME(TrueColorMatteType)
        ENUM_TO_NAME(ColorSeparationType)
        ENUM_TO_NAME(ColorSeparationMatteType)
        ENUM_TO_NAME(OptimizeType)
        ENUM_TO_NAME(PaletteBilevelMatteType)
    }

    return "UndefinedType";
}


/*
    External:   ImageType.new
    Purpose:    Construct an ImageType enum object for the specified value
*/
VALUE
ImageType_new(ImageType type)
{
    const char *name = ImageType_name(type);
    return rm_enum_new(Class_ImageType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/*
    Static:     InterlaceType_name
    Purpose:    Return the name of a InterlaceType enum as a string
*/
static const char *
InterlaceType_name(InterlaceType interlace)
{
    switch(interlace)
    {
        ENUM_TO_NAME(UndefinedInterlace)
#if defined(HAVE_ENUM_GIFINTERLACE)
        ENUM_TO_NAME(GIFInterlace)
#endif
#if defined(HAVE_ENUM_JPEGINTERLACE)
        ENUM_TO_NAME(JPEGInterlace)
#endif
#if defined(HAVE_ENUM_PNGINTERLACE)
        ENUM_TO_NAME(PNGInterlace)
#endif
        ENUM_TO_NAME(NoInterlace)
        ENUM_TO_NAME(LineInterlace)
        ENUM_TO_NAME(PlaneInterlace)
        ENUM_TO_NAME(PartitionInterlace)
    }

    return "UndefinedInterlace";
}


/*
    External:   InterlaceType_new
    Purpose:    Construct an InterlaceType enum object for the specified value.
*/
VALUE
InterlaceType_new(InterlaceType interlace)
{
    const char *name = InterlaceType_name(interlace);
    return rm_enum_new(Class_InterlaceType, ID2SYM(rb_intern(name)), INT2FIX(interlace));
}


/*
    Static:     InterpolatePixelMethod_name
    Purpose:    Return the name of a InterpolatePixelMethod enum as a string
*/
static const char *
InterpolatePixelMethod_name(InterpolatePixelMethod interpolate)
{
    switch(interpolate)
    {
        ENUM_TO_NAME(UndefinedInterpolatePixel)
        ENUM_TO_NAME(AverageInterpolatePixel)
        ENUM_TO_NAME(BicubicInterpolatePixel)
        ENUM_TO_NAME(BilinearInterpolatePixel)
        ENUM_TO_NAME(FilterInterpolatePixel)
        ENUM_TO_NAME(IntegerInterpolatePixel)
        ENUM_TO_NAME(MeshInterpolatePixel)
        ENUM_TO_NAME(NearestNeighborInterpolatePixel)
#if defined(HAVE_ENUM_SPLINEINTERPOLATEPIXEL)
        ENUM_TO_NAME(SplineInterpolatePixel)
#endif
    }

    return "UndefinedInterpolatePixel";
}


/*
    External:   InterpolatePixelMethod_new
    Purpose:    Construct an InterpolatePixelMethod enum object for the specified value.
*/
VALUE
InterpolatePixelMethod_new(InterpolatePixelMethod interpolate)
{
    const char *name = InterpolatePixelMethod_name(interpolate);
    return rm_enum_new(Class_InterpolatePixelMethod, ID2SYM(rb_intern(name)), INT2FIX(interpolate));
}


/*
    External:   MagickLayerMethod_new
    Purpose:    Construct an MagickLayerMethod enum object for the specified value.
*/
static const char *
LAYERMETHODTYPE_NAME(LAYERMETHODTYPE method)
{
    switch(method)
    {
        ENUM_TO_NAME(UndefinedLayer)
        ENUM_TO_NAME(CompareAnyLayer)
        ENUM_TO_NAME(CompareClearLayer)
        ENUM_TO_NAME(CompareOverlayLayer)
        ENUM_TO_NAME(OptimizeLayer)
        ENUM_TO_NAME(OptimizePlusLayer)
        ENUM_TO_NAME(CoalesceLayer)
        ENUM_TO_NAME(DisposeLayer)
#if defined(HAVE_ENUM_OPTIMIZETRANSLAYER)
        ENUM_TO_NAME(OptimizeTransLayer)
#endif
#if defined(HAVE_ENUM_OPTIMIZEIMAGELAYER)
        ENUM_TO_NAME(OptimizeImageLayer)
#endif
#if defined(HAVE_ENUM_REMOVEDUPSLAYER)
        ENUM_TO_NAME(RemoveDupsLayer)
#endif
#if defined(HAVE_ENUM_REMOVEZEROLAYER)
        ENUM_TO_NAME(RemoveZeroLayer)
#endif
#if defined(HAVE_ENUM_COMPOSITELAYER)
        ENUM_TO_NAME(CompositeLayer)
#endif
#if defined(HAVE_ENUM_MERGELAYER)
        ENUM_TO_NAME(MergeLayer)
#endif
#if defined(HAVE_ENUM_MOSAICLAYER)
        ENUM_TO_NAME(MosaicLayer)
#endif
#if defined(HAVE_ENUM_FLATTENLAYER)
        ENUM_TO_NAME(FlattenLayer)
#endif

    }

    return "UndefinedLayer";
}


VALUE
LAYERMETHODTYPE_NEW(LAYERMETHODTYPE method)
{
    const char *name = LAYERMETHODTYPE_NAME(method);
    return rm_enum_new(CLASS_LAYERMETHODTYPE, ID2SYM(rb_intern(name)), INT2FIX(method));
}


/*
    Static:     RenderingIntent_name
    Purpose:    Return the name of a RenderingIntent enum as a string
*/
static const char *
RenderingIntent_name(RenderingIntent intent)
{
    switch(intent)
    {
        ENUM_TO_NAME(UndefinedIntent)
        ENUM_TO_NAME(SaturationIntent)
        ENUM_TO_NAME(PerceptualIntent)
        ENUM_TO_NAME(AbsoluteIntent)
        ENUM_TO_NAME(RelativeIntent)
    }

    return "UndefinedIntent";
}


/*
    External:   RenderingIntent_new
    Purpose:    Construct an RenderingIntent enum object for the specified value.
*/
VALUE
RenderingIntent_new(RenderingIntent intent)
{
    const char *name = RenderingIntent_name(intent);
    return rm_enum_new(Class_RenderingIntent, ID2SYM(rb_intern(name)), INT2FIX(intent));
}


/*
    Static:     ResolutionType_name
    Purpose:    Return the name of a ResolutionType enum as a string
*/
static const char *
ResolutionType_name(ResolutionType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedResolution)
        ENUM_TO_NAME(PixelsPerInchResolution)
        ENUM_TO_NAME(PixelsPerCentimeterResolution)
    }

    return "UndefinedResolution";
}


/*
    External:   ResolutionType_new
    Purpose:    Construct an ResolutionType enum object for the specified value.
*/
VALUE
ResolutionType_new(ResolutionType type)
{
    const char *name = ResolutionType_name(type);
    return rm_enum_new(Class_ResolutionType, ID2SYM(rb_intern(name)), INT2FIX(type));
}



/*
    Static:     OrientationType_name
    Purpose:    Return the name of a OrientationType enum as a string
*/
static const char *
OrientationType_name(OrientationType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedOrientation)
        ENUM_TO_NAME(TopLeftOrientation)
        ENUM_TO_NAME(TopRightOrientation)
        ENUM_TO_NAME(BottomRightOrientation)
        ENUM_TO_NAME(BottomLeftOrientation)
        ENUM_TO_NAME(LeftTopOrientation)
        ENUM_TO_NAME(RightTopOrientation)
        ENUM_TO_NAME(RightBottomOrientation)
        ENUM_TO_NAME(LeftBottomOrientation)
    }

    return "UndefinedOrientation";
}


/*
    External:   OrientationType_new
    Purpose:    Construct an OrientationType enum object for the specified value.
*/
VALUE
OrientationType_new(OrientationType type)
{
    const char *name = OrientationType_name(type);
    return rm_enum_new(Class_OrientationType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/*
    External:   Color_from_ColorInfo
    Purpose:    Convert a ColorInfo structure to a Magick::Color
*/
VALUE
Color_from_ColorInfo(const ColorInfo *ci)
{
    ComplianceType compliance_type;
    volatile VALUE name;
    volatile VALUE compliance;
    volatile VALUE color;

    name       = rb_str_new2(ci->name);

    compliance_type = ci->compliance;
    compliance = ComplianceType_new(compliance_type);
    color      = Pixel_from_MagickPixelPacket(&(ci->color));

    return rb_funcall(Class_Color, rm_ID_new, 3
                    , name, compliance, color);
}


/*
    External:   Color_to_ColorInfo
    Purpose:    Convert a Magick::Color to a ColorInfo structure
*/
void
Color_to_ColorInfo(ColorInfo *ci, VALUE st)
{
    Pixel *pixel;
    volatile VALUE members, m;

    if (CLASS_OF(st) != Class_Color)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(st)));
    }

    memset(ci, '\0', sizeof(ColorInfo));

    members = rb_funcall(st, rm_ID_values, 0);

    m = rb_ary_entry(members, 0);
    if (m != Qnil)
    {
        (void) CloneString((char **)&(ci->name), StringValuePtr(m));
    }
    m = rb_ary_entry(members, 1);
    if (m != Qnil)
    {
        VALUE_TO_ENUM(m, ci->compliance, ComplianceType);
    }
    m = rb_ary_entry(members, 2);
    if (m != Qnil)
    {
        Data_Get_Struct(m, Pixel, pixel);
        // For >= 6.3.0, ColorInfo.color is a MagickPixelPacket so we have to
        // convert the PixelPacket.
        GetMagickPixelPacket(NULL, &ci->color);
        ci->color.red = (MagickRealType) pixel->red;
        ci->color.green = (MagickRealType) pixel->green;
        ci->color.blue = (MagickRealType) pixel->blue;
        ci->color.opacity = (MagickRealType) OpaqueOpacity;
        ci->color.index = (MagickRealType) 0;
    }
}


/*
    Static:     destroy_ColorInfo
    Purpose:    free the storage allocated by Color_to_ColorInfo, above.
*/
static void
destroy_ColorInfo(ColorInfo *ci)
{
    magick_free((void*)ci->name);
    ci->name = NULL;
}


/*
    Method:     Color#to_s
    Purpose:    Return a string representation of a Magick::Color object
*/
VALUE
Color_to_s(VALUE self)
{
    ColorInfo ci;
    char buff[1024];

    Color_to_ColorInfo(&ci, self);

    sprintf(buff, "name=%s, compliance=%s, "
#if (QuantumDepth == 32 || QuantumDepth == 64) && defined(HAVE_TYPE_LONG_DOUBLE)
                  "color.red=%Lg, color.green=%Lg, color.blue=%Lg, color.opacity=%Lg ",
#else
                  "color.red=%g, color.green=%g, color.blue=%g, color.opacity=%g ",
#endif
                  ci.name,
                  ComplianceType_name(&ci.compliance),
                  ci.color.red, ci.color.green, ci.color.blue, ci.color.opacity);

    destroy_ColorInfo(&ci);
    return rb_str_new2(buff);
}


/*
    Extern:     Pixel_from_PixelPacket
    Purpose:    Create a Magick::Pixel object from a PixelPacket structure.
    Notes:      bypasses normal Pixel.new, Pixel#initialize methods
*/
VALUE
Pixel_from_PixelPacket(const PixelPacket *pp)
{
    Pixel *pixel;

    pixel = ALLOC(Pixel);
    *pixel = *pp;
    return Data_Wrap_Struct(Class_Pixel, NULL, destroy_Pixel, pixel);
}


/*
    Static:     Pixel_from_MagickPixelPacket
    Purpose:    Create a Magick::Pixel object from a MagickPixelPacket structure.
    Notes:      bypasses normal Pixel.new, Pixel#initialize methods
*/
static VALUE
Pixel_from_MagickPixelPacket(const MagickPixelPacket *pp)
{
    Pixel *pixel;

    pixel          = ALLOC(Pixel);
    pixel->red     = ROUND_TO_QUANTUM(pp->red);
    pixel->green   = ROUND_TO_QUANTUM(pp->green);
    pixel->blue    = ROUND_TO_QUANTUM(pp->blue);
    pixel->opacity = ROUND_TO_QUANTUM(pp->opacity);

    return Data_Wrap_Struct(Class_Pixel, NULL, destroy_Pixel, pixel);
}


/*
 *  Static:     color_arg_rescue
 *  Purpose:    raise ArgumentError if the color name cannot be converted
 *              to a string via rb_str_to_str.
*/
static VALUE
color_arg_rescue(VALUE arg)
{
    rb_raise(rb_eTypeError, "argument must be color name or pixel (%s given)",
            rb_class2name(CLASS_OF(arg)));
    return (VALUE)0;
}


/*
    Extern:     Color_to_PixelPacket
    Purpose:    Convert either a String color name or
                a Magick::Pixel to a PixelPacket
*/
void
Color_to_PixelPacket(PixelPacket *pp, VALUE color)
{
    Pixel *pixel;

    // Allow color name or Pixel
    if (CLASS_OF(color) == Class_Pixel)
    {
        Data_Get_Struct(color, Pixel, pixel);
        *pp = *pixel;
    }
    else
    {
        // require 'to_str' here instead of just 'to_s'.
        color = rb_rescue(rb_str_to_str, color, color_arg_rescue, color);
        Color_Name_to_PixelPacket(pp, color);
    }
}


/*
    Extern:     Color_to_MagickPixelPacket
    Purpose:    Convert either a String color name or
                a Magick::Pixel to a MagickPixelPacket
    Notes:      The channel values in a MagickPixelPacket are doubles.
*/
void
Color_to_MagickPixelPacket(Image *image, MagickPixelPacket *mpp, VALUE color)
{
    PixelPacket pp;

    // image can be NULL
    GetMagickPixelPacket(image, mpp);

    memset(&pp, '\0', sizeof(pp));
    Color_to_PixelPacket(&pp, color);
    mpp->red = (MagickRealType) pp.red;
    mpp->green = (MagickRealType) pp.green;
    mpp->blue = (MagickRealType) pp.blue;
    mpp->opacity = (MagickRealType) pp.opacity;
}


/*
    Extern:     PrimaryInfo_from_PrimaryInfo(pp)
    Purpose:    Create a Magick::PrimaryInfo object from a PrimaryInfo structure.
*/
VALUE
PrimaryInfo_from_PrimaryInfo(PrimaryInfo *p)
{
    return rb_funcall(Class_Primary, rm_ID_new, 3
                    , INT2FIX(p->x), INT2FIX(p->y), INT2FIX(p->z));
}


/*
    Extern:     PrimaryInfo_to_PrimaryInfo
    Purpose:    Convert a Magick::PrimaryInfo object to a PrimaryInfo structure
*/
void
PrimaryInfo_to_PrimaryInfo(PrimaryInfo *pi, VALUE sp)
{
    volatile VALUE members, m;

    if (CLASS_OF(sp) != Class_Primary)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(sp)));
    }
    members = rb_funcall(sp, rm_ID_values, 0);
    m = rb_ary_entry(members, 0);
    pi->x = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 1);
    pi->y = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 2);
    pi->z = m == Qnil ? 0.0 : NUM2DBL(m);
}


/*
    Extern:     PointInfo_to_Point(pp)
    Purpose:    Create a Magick::Point object from a PointInfo structure.
*/
VALUE
PointInfo_to_Point(PointInfo *p)
{
    return rb_funcall(Class_Point, rm_ID_new, 2
                    , INT2FIX(p->x), INT2FIX(p->y));
}


/*
    Extern:     Point_to_PointInfo
    Purpose:    Convert a Magick::Point object to a PointInfo structure
*/
void
Point_to_PointInfo(PointInfo *pi, VALUE sp)
{
    volatile VALUE members, m;

    if (CLASS_OF(sp) != Class_Point)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(sp)));
    }
    members = rb_funcall(sp, rm_ID_values, 0);
    m = rb_ary_entry(members, 0);
    pi->x = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 1);
    pi->y = m == Qnil ? 0.0 : NUM2DBL(m);
}


/*
    Extern:     ChromaticityInfo_new(pp)
    Purpose:    Create a Magick::ChromaticityInfo object from a
                ChromaticityInfo structure.
*/
VALUE
ChromaticityInfo_new(ChromaticityInfo *ci)
{
    volatile VALUE red_primary;
    volatile VALUE green_primary;
    volatile VALUE blue_primary;
    volatile VALUE white_point;

    red_primary   = PrimaryInfo_from_PrimaryInfo(&ci->red_primary);
    green_primary = PrimaryInfo_from_PrimaryInfo(&ci->green_primary);
    blue_primary  = PrimaryInfo_from_PrimaryInfo(&ci->blue_primary);
    white_point   = PrimaryInfo_from_PrimaryInfo(&ci->white_point);

    return rb_funcall(Class_Chromaticity, rm_ID_new, 4
                    , red_primary, green_primary, blue_primary, white_point);
}


/*
    Extern:     ChromaticityInfo_to_ChromaticityInfo
    Purpose:    Extract the elements from a Magick::ChromaticityInfo
                and store in a ChromaticityInfo structure.
*/
void
ChromaticityInfo_to_ChromaticityInfo(ChromaticityInfo *ci, VALUE chrom)
{
    volatile VALUE chrom_members;
    volatile VALUE red_primary, green_primary, blue_primary, white_point;
    volatile VALUE entry_members, x, y;
    ID values_id;

    if (CLASS_OF(chrom) != Class_Chromaticity)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(chrom)));
    }
    values_id = rm_ID_values;

    // Get the struct members in an array
    chrom_members = rb_funcall(chrom, values_id, 0);
    red_primary   = rb_ary_entry(chrom_members, 0);
    green_primary = rb_ary_entry(chrom_members, 1);
    blue_primary  = rb_ary_entry(chrom_members, 2);
    white_point = rb_ary_entry(chrom_members, 3);

    // Get the red_primary PrimaryInfo members in an array
    entry_members = rb_funcall(red_primary, values_id, 0);
    x = rb_ary_entry(entry_members, 0);         // red_primary.x
    ci->red_primary.x = x == Qnil ? 0.0 : NUM2DBL(x);
    y = rb_ary_entry(entry_members, 1);         // red_primary.y
    ci->red_primary.y = y == Qnil ? 0.0 : NUM2DBL(y);
    ci->red_primary.z = 0.0;

    // Get the green_primary PrimaryInfo members in an array
    entry_members = rb_funcall(green_primary, values_id, 0);
    x = rb_ary_entry(entry_members, 0);         // green_primary.x
    ci->green_primary.x = x == Qnil ? 0.0 : NUM2DBL(x);
    y = rb_ary_entry(entry_members, 1);         // green_primary.y
    ci->green_primary.y = y == Qnil ? 0.0 : NUM2DBL(y);
    ci->green_primary.z = 0.0;

    // Get the blue_primary PrimaryInfo members in an array
    entry_members = rb_funcall(blue_primary, values_id, 0);
    x = rb_ary_entry(entry_members, 0);         // blue_primary.x
    ci->blue_primary.x = x == Qnil ? 0.0 : NUM2DBL(x);
    y = rb_ary_entry(entry_members, 1);         // blue_primary.y
    ci->blue_primary.y = y == Qnil ? 0.0 : NUM2DBL(y);
    ci->blue_primary.z = 0.0;

    // Get the white_point PrimaryInfo members in an array
    entry_members = rb_funcall(white_point, values_id, 0);
    x = rb_ary_entry(entry_members, 0);         // white_point.x
    ci->white_point.x = x == Qnil ? 0.0 : NUM2DBL(x);
    y = rb_ary_entry(entry_members, 1);         // white_point.y
    ci->white_point.y = y == Qnil ? 0.0 : NUM2DBL(y);
    ci->white_point.z = 0.0;
}


/*
    External:   Rectangle_from_RectangleInfo
    Purpose:    Convert a RectangleInfo structure to a Magick::Rectangle
*/
VALUE
Rectangle_from_RectangleInfo(RectangleInfo *rect)
{
    volatile VALUE width;
    volatile VALUE height;
    volatile VALUE x, y;

    width  = UINT2NUM(rect->width);
    height = UINT2NUM(rect->height);
    x      = INT2NUM(rect->x);
    y      = INT2NUM(rect->y);
    return rb_funcall(Class_Rectangle, rm_ID_new, 4
                    , width, height, x, y);
}


/*
    External:   Rectangle_to_RectangleInfo
    Purpose:    Convert a Magick::Rectangle to a RectangleInfo structure.
*/
void
Rectangle_to_RectangleInfo(RectangleInfo *rect, VALUE sr)
{
    volatile VALUE members, m;

    if (CLASS_OF(sr) != Class_Rectangle)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(sr)));
    }
    members = rb_funcall(sr, rm_ID_values, 0);
    m = rb_ary_entry(members, 0);
    rect->width  = m == Qnil ? 0 : NUM2ULONG(m);
    m = rb_ary_entry(members, 1);
    rect->height = m == Qnil ? 0 : NUM2ULONG(m);
    m = rb_ary_entry(members, 2);
    rect->x      = m == Qnil ? 0 : NUM2LONG (m);
    m = rb_ary_entry(members, 3);
    rect->y      = m == Qnil ? 0 : NUM2LONG (m);
}


/*
    External:   Segment_from_SegmentInfo
    Purpose:    Convert a SegmentInfo structure to a Magick::Segment
*/
VALUE
Segment_from_SegmentInfo(SegmentInfo *segment)
{
    volatile VALUE x1, y1, x2, y2;

    x1 = rb_float_new(segment->x1);
    y1 = rb_float_new(segment->y1);
    x2 = rb_float_new(segment->x2);
    y2 = rb_float_new(segment->y2);
    return rb_funcall(Class_Segment, rm_ID_new, 4, x1, y1, x2, y2);
}


/*
    External:   Segment_to_SegmentInfo
    Purpose:    Convert a Magick::Segment to a SegmentInfo structure.
*/
void
Segment_to_SegmentInfo(SegmentInfo *segment, VALUE s)
{
    volatile VALUE members, m;

    if (CLASS_OF(s) != Class_Segment)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(s)));
    }

    members = rb_funcall(s, rm_ID_values, 0);
    m = rb_ary_entry(members, 0);
    segment->x1 = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 1);
    segment->y1 = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 2);
    segment->x2 = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 3);
    segment->y2 = m == Qnil ? 0.0 : NUM2DBL(m);
}


/*
    Static:     StretchType_new
    Purpose:    Construct a StretchType enum for a specified StretchType value
*/
static VALUE
StretchType_new(StretchType stretch)
{
    const char *name = StretchType_name(stretch);
    return rm_enum_new(Class_StretchType, ID2SYM(rb_intern(name)), INT2FIX(stretch));
}


/*
    Static:     StyleType_new
    Purpose:    Construct a StyleType enum for a specified StyleType value
*/
static VALUE
StyleType_new(StyleType style)
{
    const char *name = StyleType_name(style);
    return rm_enum_new(Class_StyleType, ID2SYM(rb_intern(name)), INT2FIX(style));
}


/*
    External:   Font_from_TypeInfo
    Purpose:    Convert a TypeInfo structure to a Magick::Font
*/
VALUE
Font_from_TypeInfo(const TypeInfo *ti)
{
    volatile VALUE name, description, family;
    volatile VALUE style, stretch, weight;
    volatile VALUE encoding, foundry, format;

    name        = rb_str_new2(ti->name);
    family      = rb_str_new2(ti->family);
    style       = StyleType_new(ti->style);
    stretch     = StretchType_new(ti->stretch);
    weight      = UINT2NUM(ti->weight);
    description = ti->description ? rb_str_new2(ti->description) : Qnil;
    encoding    = ti->encoding    ? rb_str_new2(ti->encoding) : Qnil;
    foundry     = ti->foundry     ? rb_str_new2(ti->foundry)  : Qnil;
    format      = ti->format      ? rb_str_new2(ti->format)   : Qnil;

    return rb_funcall(Class_Font, rm_ID_new, 9
                    , name, description, family, style
                    , stretch, weight, encoding, foundry, format);
}


/*
    External:   Font_to_TypeInfo
    Purpose:    Convert a Magick::Font to a TypeInfo structure
*/
void
Font_to_TypeInfo(TypeInfo *ti, VALUE st)
{
    volatile VALUE members, m;

    if (CLASS_OF(st) != Class_Font)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(st)));
    }

    memset(ti, '\0', sizeof(TypeInfo));

    members = rb_funcall(st, rm_ID_values, 0);
    m = rb_ary_entry(members, 0);
    if (m != Qnil)
    {
        (void) CloneString((char **)&(ti->name), StringValuePtr(m));
    }
    m = rb_ary_entry(members, 1);
    if (m != Qnil)
    {
        (void) CloneString((char **)&(ti->description), StringValuePtr(m));
    }
    m = rb_ary_entry(members, 2);
    if (m != Qnil)
    {
        (void) CloneString((char **)&(ti->family), StringValuePtr(m));
    }
    m = rb_ary_entry(members, 3); ti->style   = m == Qnil ? 0 : FIX2INT(m);
    m = rb_ary_entry(members, 4); ti->stretch = m == Qnil ? 0 : FIX2INT(m);
    m = rb_ary_entry(members, 5); ti->weight  = m == Qnil ? 0 : FIX2INT(m);

    m = rb_ary_entry(members, 6);
    if (m != Qnil)
        (void) CloneString((char **)&(ti->encoding), StringValuePtr(m));
    m = rb_ary_entry(members, 7);
    if (m != Qnil)
        (void) CloneString((char **)&(ti->foundry), StringValuePtr(m));
    m = rb_ary_entry(members, 8);
    if (m != Qnil)
        (void) CloneString((char **)&(ti->format), StringValuePtr(m));
}


/*
    Static:     destroy_TypeInfo
    Purpose:    free the storage allocated by Font_to_TypeInfo, above.
*/
static void
destroy_TypeInfo(TypeInfo *ti)
{
    magick_free((void*)ti->name);
    ti->name = NULL;
    magick_free((void*)ti->description);
    ti->description = NULL;
    magick_free((void*)ti->family);
    ti->family = NULL;
    magick_free((void*)ti->encoding);
    ti->encoding = NULL;
    magick_free((void*)ti->foundry);
    ti->foundry = NULL;
    magick_free((void*)ti->format);
    ti->format = NULL;
}


/*
    External:   Font_to_s
    Purpose:    implement the Font#to_s method
*/
VALUE
Font_to_s(VALUE self)
{
    TypeInfo ti;
    char weight[20];
    char buff[1024];

    Font_to_TypeInfo(&ti, self);

    switch (ti.weight)
    {
        case 400:
            strcpy(weight, "NormalWeight");
            break;
        case 700:
            strcpy(weight, "BoldWeight");
            break;
        default:
            sprintf(weight, "%lu", ti.weight);
            break;
    }

    sprintf(buff, "name=%s, description=%s, "
                  "family=%s, style=%s, stretch=%s, weight=%s, "
                  "encoding=%s, foundry=%s, format=%s",
                  ti.name,
                  ti.description,
                  ti.family,
                  StyleType_name(ti.style),
                  StretchType_name(ti.stretch),
                  weight,
                  ti.encoding ? ti.encoding : "",
                  ti.foundry ? ti.foundry : "",
                  ti.format ? ti.format : "");

    destroy_TypeInfo(&ti);
    return rb_str_new2(buff);

}


/*
    External:   TypeMetric_from_TypeMetric
    Purpose:    Convert a TypeMetric structure to a Magick::TypeMetric
*/
VALUE
TypeMetric_from_TypeMetric(TypeMetric *tm)
{
    volatile VALUE pixels_per_em;
    volatile VALUE ascent, descent;
    volatile VALUE width, height, max_advance;
    volatile VALUE bounds, underline_position, underline_thickness;

    pixels_per_em       = PointInfo_to_Point(&tm->pixels_per_em);
    ascent              = rb_float_new(tm->ascent);
    descent             = rb_float_new(tm->descent);
    width               = rb_float_new(tm->width);
    height              = rb_float_new(tm->height);
    max_advance         = rb_float_new(tm->max_advance);
    bounds              = Segment_from_SegmentInfo(&tm->bounds);
    underline_position  = rb_float_new(tm->underline_position);
    underline_thickness = rb_float_new(tm->underline_position);

    return rb_funcall(Class_TypeMetric, rm_ID_new, 9
                    , pixels_per_em, ascent, descent, width
                    , height, max_advance, bounds
                    , underline_position, underline_thickness);
}


/*
    External:   TypeMetric_to_TypeMetric
    Purpose:    Convert a Magick::TypeMetric to a TypeMetric structure.
*/
void
TypeMetric_to_TypeMetric(TypeMetric *tm, VALUE st)
{
    volatile VALUE members, m;
    volatile VALUE pixels_per_em;

    if (CLASS_OF(st) != Class_TypeMetric)
    {
        rb_raise(rb_eTypeError, "type mismatch: %s given",
                 rb_class2name(CLASS_OF(st)));
    }
    members = rb_funcall(st, rm_ID_values, 0);

    pixels_per_em   = rb_ary_entry(members, 0);
    Point_to_PointInfo(&tm->pixels_per_em, pixels_per_em);

    m = rb_ary_entry(members, 1);
    tm->ascent      = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 2);
    tm->descent     = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 3);
    tm->width       = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 4);
    tm->height      = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 5);
    tm->max_advance = m == Qnil ? 0.0 : NUM2DBL(m);

    m = rb_ary_entry(members, 6);
    Segment_to_SegmentInfo(&tm->bounds, m);

    m = rb_ary_entry(members, 7);
    tm->underline_position  = m == Qnil ? 0.0 : NUM2DBL(m);
    m = rb_ary_entry(members, 8);
    tm->underline_thickness = m == Qnil ? 0.0 : NUM2DBL(m);
}


/*
    Method:     Magick::TypeMetric#to_s
    Purpose:    Create a string representation of a Magick::TypeMetric
*/
VALUE
TypeMetric_to_s(VALUE self)
{
    TypeMetric tm;
    char buff[200];

    TypeMetric_to_TypeMetric(&tm, self);
    sprintf(buff, "pixels_per_em=(x=%g,y=%g) "
                  "ascent=%g descent=%g width=%g height=%g max_advance=%g "
                  "bounds.x1=%g bounds.y1=%g bounds.x2=%g bounds.y2=%g "
                  "underline_position=%g underline_thickness=%g",
                  tm.pixels_per_em.x, tm.pixels_per_em.y,
                  tm.ascent, tm.descent, tm.width, tm.height, tm.max_advance,
                  tm.bounds.x1, tm.bounds.y1, tm.bounds.x2, tm.bounds.y2,
                  tm.underline_position, tm.underline_thickness);
    return rb_str_new2(buff);
}


/*
    Static:     VirtualPixelMethod_name
    Purpose:    Return the string representation of a VirtualPixelMethod value
*/
static const char *
VirtualPixelMethod_name(VirtualPixelMethod method)
{
    switch (method)
    {
        ENUM_TO_NAME(UndefinedVirtualPixelMethod)
        ENUM_TO_NAME(EdgeVirtualPixelMethod)
        ENUM_TO_NAME(MirrorVirtualPixelMethod)
        ENUM_TO_NAME(TileVirtualPixelMethod)
        ENUM_TO_NAME(TransparentVirtualPixelMethod)
        ENUM_TO_NAME(BackgroundVirtualPixelMethod)
        ENUM_TO_NAME(DitherVirtualPixelMethod)
        ENUM_TO_NAME(RandomVirtualPixelMethod)
        ENUM_TO_NAME(ConstantVirtualPixelMethod)
#if defined(HAVE_ENUM_MASKVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(MaskVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_BLACKVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(BlackVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_GRAYVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(GrayVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_WHITEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(WhiteVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_HORIZONTALTILEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(HorizontalTileVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_VERTICALTILEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(VerticalTileVirtualPixelMethod)
#endif
    }

    return "UndefinedVirtualPixelMethod";
}


/*
    Static:     VirtualPixelMethod_new
    Purpose:    Construct a VirtualPixelMethod enum for a specified VirtualPixelMethod value
*/
VALUE
VirtualPixelMethod_new(VirtualPixelMethod style)
{
    const char *name = VirtualPixelMethod_name(style);
    return rm_enum_new(Class_VirtualPixelMethod, ID2SYM(rb_intern(name)), INT2FIX(style));
}


/*
 *  Extern:     rm_define_enum_type
 *  Purpose:    set up a subclass of Enum
*/
VALUE
rm_define_enum_type(const char *tag)
{
    VALUE class;

    class = rb_define_class_under(Module_Magick, tag, Class_Enum);\

    rb_define_singleton_method(class, "values", Enum_type_values, 0);
    rb_define_method(class, "initialize", Enum_type_initialize, 2);
    rb_define_method(class, "inspect", Enum_type_inspect, 0);
    return class;
}


/*
    Extern:  rm_enum_new (1.8)
    Purpose: Construct a new Enum subclass instance
*/
VALUE
rm_enum_new(VALUE class, VALUE sym, VALUE val)
{
    VALUE argv[2];

    argv[0] = sym;
    argv[1] = val;
    return rb_obj_freeze(rb_class_new_instance(2, argv, class));
}


/*
    Extern:  Enum_alloc (1.8)
    Purpose: Enum class alloc function
*/
VALUE
Enum_alloc(VALUE class)
{
   MagickEnum *magick_enum;
   volatile VALUE enumr;

   enumr = Data_Make_Struct(class, MagickEnum, NULL, NULL, magick_enum);
   rb_obj_freeze(enumr);
   return enumr;
}


/*
    Method:  Enum#initialize
    Purpose: Initialize a new Enum instance
*/
VALUE
Enum_initialize(VALUE self, VALUE sym, VALUE val)
{
   MagickEnum *magick_enum;

   Data_Get_Struct(self, MagickEnum, magick_enum);
   magick_enum->id = rb_to_id(sym); /* convert symbol to ID */
   magick_enum->val = NUM2INT(val);

   return self;
}


/*
    Method:  Enum#to_s
    Purpose: Return the name of an enum
*/
VALUE
Enum_to_s(VALUE self)
{
   MagickEnum *magick_enum;

   Data_Get_Struct(self, MagickEnum, magick_enum);
   return rb_str_new2(rb_id2name(magick_enum->id));
}


/*
    Method:  Enum#to_i
    Purpose: Return the value of an enum
*/
VALUE
Enum_to_i(VALUE self)
{
   MagickEnum *magick_enum;

   Data_Get_Struct(self, MagickEnum, magick_enum);
   return INT2NUM(magick_enum->val);
}


/*
    Method:  Enum#<=>
    Purpose: Support Comparable module in Enum
    Returns: -1, 0, 1, or nil
    Notes:   Enums must be instances of the same class to be equal.
*/
VALUE
Enum_spaceship(VALUE self, VALUE other)
{
    MagickEnum *this, *that;

    Data_Get_Struct(self, MagickEnum, this);
    Data_Get_Struct(other, MagickEnum, that);

    if (this->val > that->val)
    {
        return INT2FIX(1);
    }
    else if (this->val < that->val)
    {
        return INT2FIX(-1);
    }

    // Values are equal, check class.

    return rb_funcall(CLASS_OF(self), rm_ID_spaceship, 1, CLASS_OF(other));
}


/*
    Method:  Enum#===
    Purpose: "Case equal" operator for Enum
    Returns: true or false
    Notes:   Yes, I know "case equal" is a misnomer.
*/
VALUE
Enum_case_eq(VALUE self, VALUE other)
{
    MagickEnum *this, *that;

    if (CLASS_OF(self) == CLASS_OF(other))
    {
        Data_Get_Struct(self, MagickEnum, this);
        Data_Get_Struct(other, MagickEnum, that);
        return this->val == that->val ? Qtrue : Qfalse;
    }

    return Qfalse;
}


/*
 *  Method:     xxx#initialize
 *  Purpose:    initialize method for all Enum subclasses
*/
VALUE
Enum_type_initialize(VALUE self, VALUE sym, VALUE val)
{
    VALUE super_argv[2];
    volatile VALUE enumerators;

    super_argv[0] = sym;
    super_argv[1] = val;
    (void) rb_call_super(2, (const VALUE *)super_argv);

    if (rb_cvar_defined(CLASS_OF(self), rb_intern(ENUMERATORS_CLASS_VAR)) != Qtrue)
    {
        rb_cv_set(CLASS_OF(self), ENUMERATORS_CLASS_VAR, rb_ary_new());
    }

    enumerators = rb_cv_get(CLASS_OF(self), ENUMERATORS_CLASS_VAR);
    (void) rb_ary_push(enumerators, self);

    return self;
}


/*
 *  Method:     xxx#inspect
 *  Purpose:    Enum subclass #inspect
*/
static VALUE
Enum_type_inspect(VALUE self)
{
    char str[100];
    MagickEnum *magick_enum;

    Data_Get_Struct(self, MagickEnum, magick_enum);
    sprintf(str, "%.32s=%d", rb_id2name(magick_enum->id), magick_enum->val);

    return rb_str_new2(str);
}


/*
 *  Method:     xxx.values
 *  Purpose:    Behaves like #each if a block is present, otherwise like #to_a.
 *  Notes:      defined for each Enum subclass
*/
static VALUE
Enum_type_values(VALUE class)
{
    volatile VALUE enumerators, copy;
    volatile VALUE rv;
    int x;

    enumerators = rb_cv_get(class, ENUMERATORS_CLASS_VAR);

    if (rb_block_given_p())
    {
        for (x = 0; x < RARRAY_LEN(enumerators); x++)
        {
            (void) rb_yield(rb_ary_entry(enumerators, x));
        }
        rv = class;
    }
    else
    {
        copy = rb_ary_new2(RARRAY_LEN(enumerators));
        for (x = 0; x < RARRAY_LEN(enumerators); x++)
        {
            (void) rb_ary_push(copy, rb_ary_entry(enumerators, x));
        }
        rb_obj_freeze(copy);
        rv = copy;
    }

    return rv;
}


/*
    Static:     ComplianceType_name
    Purpose:    Return the string representation of a ComplianceType value
    Notes:      xMagick will OR multiple compliance types so we have to
                arbitrarily pick one name. Set the compliance argument
                to the selected value.
*/
static const char *
ComplianceType_name(ComplianceType *c)
{
    if ((*c & (SVGCompliance|X11Compliance|XPMCompliance))
        == (SVGCompliance|X11Compliance|XPMCompliance))
    {
        return "AllCompliance";
    }
    else if (*c & SVGCompliance)
    {
        *c = SVGCompliance;
        return "SVGCompliance";
    }
    else if (*c & X11Compliance)
    {
        *c = X11Compliance;
        return "X11Compliance";
    }
    else if (*c & XPMCompliance)
    {
        *c = XPMCompliance;
        return "XPMCompliance";
    }
    else if (*c == NoCompliance)
    {
        *c = NoCompliance;
        return "NoCompliance";
    }
    else
    {
        *c = UndefinedCompliance;
        return "UndefinedCompliance";
    }
}


/*
    Extern:     StorageType_name
    Purpose:    Return the string representation of a StorageType value
*/
const char *
StorageType_name(StorageType type)
{
    switch (type)
    {
        ENUM_TO_NAME(UndefinedPixel)
        ENUM_TO_NAME(CharPixel)
        ENUM_TO_NAME(DoublePixel)
        ENUM_TO_NAME(FloatPixel)
        ENUM_TO_NAME(IntegerPixel)
        ENUM_TO_NAME(LongPixel)
        ENUM_TO_NAME(QuantumPixel)
        ENUM_TO_NAME(ShortPixel)
    }

    return "UndefinedPixel";
}


/*
    Static:     StretchType_name
    Purpose:    Return the string representation of a StretchType value
*/
static const char *
StretchType_name(StretchType stretch)
{
    switch (stretch)
    {
        ENUM_TO_NAME(UndefinedStretch)
        ENUM_TO_NAME(NormalStretch)
        ENUM_TO_NAME(UltraCondensedStretch)
        ENUM_TO_NAME(ExtraCondensedStretch)
        ENUM_TO_NAME(CondensedStretch)
        ENUM_TO_NAME(SemiCondensedStretch)
        ENUM_TO_NAME(SemiExpandedStretch)
        ENUM_TO_NAME(ExpandedStretch)
        ENUM_TO_NAME(ExtraExpandedStretch)
        ENUM_TO_NAME(UltraExpandedStretch)
        ENUM_TO_NAME(AnyStretch)
    }

    return "UndefinedStretch";
}


/*
    Static:     StyleType_name
    Purpose:    Return the string representation of a StyleType value
*/
static const char *
StyleType_name(StyleType style)
{
    switch (style)
    {
        ENUM_TO_NAME(UndefinedStyle)
        ENUM_TO_NAME(NormalStyle)
        ENUM_TO_NAME(ItalicStyle)
        ENUM_TO_NAME(ObliqueStyle)
        ENUM_TO_NAME(AnyStyle)
    }

    return "UndefinedStyle";
}


/*
    External:   write_temp_image
    Purpose:    Write a temporary copy of the image to the IM registry
    Returns:    the "filename" of the registered image
    Notes:      The `temp_name' argument must point to an char array
                of size MaxTextExtent.
*/
void
rm_write_temp_image(Image *image, char *temp_name)
{

#if defined(HAVE_SETIMAGEREGISTRY)
#define TMPNAM_CLASS_VAR "@@_tmpnam_"

    MagickBooleanType okay;
    ExceptionInfo exception;
    volatile VALUE id_value;
    int id;

    GetExceptionInfo(&exception);


    // 'id' is always the value of its previous use
    if (rb_cvar_defined(Module_Magick, rb_intern(TMPNAM_CLASS_VAR)) == Qtrue)
    {
        id_value = rb_cv_get(Module_Magick, TMPNAM_CLASS_VAR);
        id = FIX2INT(id_value);
    }
    else
    {
        id = 0;
        rb_cv_set(Module_Magick, TMPNAM_CLASS_VAR, INT2FIX(id));
    }

    id += 1;
    rb_cv_set(Module_Magick, TMPNAM_CLASS_VAR, INT2FIX(id));
    sprintf(temp_name, "mpri:%d", id);

    // Omit "mpri:" from filename to form the key
    okay = SetImageRegistry(ImageRegistryType, temp_name+5, image, &exception);
    CHECK_EXCEPTION()
    DestroyExceptionInfo(&exception);
    if (!okay)
    {
        rb_raise(rb_eRuntimeError, "SetImageRegistry failed.");
    }

#else

    long registry_id;

    rb_warn("`%s' can cause memory leaks with ImageMagick "  MagickLibVersionText
            ".\nUpgrade to ImageMagick 6.3.4-10 or later to prevent this behavior."
          , rb_id2name(THIS_FUNC()));

    registry_id = SetMagickRegistry(ImageRegistryType, image, sizeof(Image), &image->exception);
    rm_check_image_exception(image, RetainOnError);
    if (registry_id < 0)
    {
        rb_raise(rb_eRuntimeError, "SetMagickRegistry failed.");
    }

    sprintf(temp_name, "mpri:%ld", registry_id);
#endif

}


/*
    External:   delete_temp_image
    Purpose:    Delete the temporary image from the registry
    Returns:    void
*/

void
rm_delete_temp_image(char *temp_name)
{
#if defined(HAVE_SETIMAGEREGISTRY)
    MagickBooleanType okay = DeleteImageRegistry(temp_name+5);

    if (!okay)
    {
        rb_warn("DeleteImageRegistry failed for `%s'", temp_name);
    }
#else
    long registry_id = -1;

    sscanf(temp_name, "mpri:%ld", &registry_id);
    if (registry_id >= 0)
    {
        (void) DeleteMagickRegistry(registry_id);
    }
#endif
}


/*
    External:   rm_not_implemented
    Purpose:    raise NotImplementedError
    Notes:      Called when a xMagick API is not available.
                Replaces Ruby's rb_notimplement function.
*/
void
rm_not_implemented(void)
{

    rb_raise(rb_eNotImpError, "the `%s' method is not supported by ImageMagick "
            MagickLibVersionText, rb_id2name(THIS_FUNC()));
}


/*
    Static:     rm_magick_error(msg, loc)
    Purpose:    create a new ImageMagickError object and raise an exception
    Notes:      does not return
                This funky technique allows me to safely add additional
                information to the ImageMagickError object in both 1.6.8 and
                1.8.0. See www.ruby_talk.org/36408.
*/
void
rm_magick_error(const char *msg, const char *loc)
{
    volatile VALUE exc, mesg, extra;

    mesg = rb_str_new2(msg);
    extra = loc ? rb_str_new2(loc) : Qnil;

    exc = rb_funcall(Class_ImageMagickError, rm_ID_new, 2, mesg, extra);
    (void) rb_funcall(rb_cObject, rb_intern("raise"), 1, exc);
}


/*
    Method:     ImageMagickError#initialize(msg, loc)
    Purpose:    initialize a new ImageMagickError object - store
                the "loc" string in the @magick_location instance variable
*/
VALUE
ImageMagickError_initialize(int argc, VALUE *argv, VALUE self)
{
    VALUE super_argv[1] = {(VALUE)0};
    int super_argc = 0;
    volatile VALUE extra = Qnil;

    switch(argc)
    {
        case 2:
            extra = argv[1];
        case 1:
            super_argv[0] = argv[0];
            super_argc = 1;
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
    }

    (void) rb_call_super(super_argc, (const VALUE *)super_argv);
    (void) rb_iv_set(self, "@"MAGICK_LOC, extra);


    return self;
}


/*
    Function:   rm_get_property
    Purpose:    Backport GetImageProperty for pre-6.3.1 versions of ImageMagick
*/
const char *
rm_get_property(const Image *img, const char *property)
{
#if defined(HAVE_GETIMAGEPROPERTY)
    return GetImageProperty(img, property);
#else
    const ImageAttribute *attr;

    attr = GetImageAttribute(img, property);
    return attr ? (const char *)attr->value : NULL;
#endif
}


/*
    Function:   rm_set_property
    Purpose:    Backport SetImageProperty for pre-6.3.1 versions of ImageMagick
*/
MagickBooleanType
rm_set_property(Image *image, const char *property, const char *value)
{
#if defined(HAVE_SETIMAGEPROPERTY)
    return SetImageProperty(image, property, value);
#else
    return SetImageAttribute(image, property, value);
#endif
}


/*
    Function:   rm_get_optional_arguments
    Purpose:    Collect optional method arguments via Magick::OptionalMethodArguments
    Notes:      Creates an instance of Magick::OptionalMethodArguments, then yields
                to a block in the context of the instance.
*/
void
rm_get_optional_arguments(VALUE img)
{
  volatile VALUE OptionalMethodArguments;
  volatile VALUE opt_args;
  VALUE argv[1];

  // opt_args = Magick::OptionalMethodArguments.new(img)
  // opt_args.instance_eval { block }
  if (rb_block_given_p())
  {
      OptionalMethodArguments = rb_const_get_from(Module_Magick, rb_intern("OptionalMethodArguments"));
      argv[0] = img;
      opt_args = rb_class_new_instance(1, argv, OptionalMethodArguments);
      (void) rb_obj_instance_eval(0, NULL, opt_args);
  }

  return;
}


/*
    Function:   rm_exif_by_entry
    Purpose:    replicate old (< 6.3.2) EXIF:* functionality using GetImageProperty
                by returning the exif entries as a single string, separated by \n's.
                Do this so that RMagick.rb works no matter which version of
                ImageMagick is in use.
    Notes:      see magick/identify.c
*/
VALUE
rm_exif_by_entry(Image *image)
{
#if defined(HAVE_GETIMAGEPROPERTY)
    const char *property, *value;
    char *str;
    size_t len = 0, property_l, value_l;
    volatile VALUE v;

    (void) GetImageProperty(image, "exif:*");
    ResetImagePropertyIterator(image);
    property = GetNextImageProperty(image);

    // Measure the exif properties and values
    while (property)
    {
        // ignore properties that don't start with "exif:"
        property_l = strlen(property);
        if (property_l > 5 && rm_strncasecmp(property, "exif:", 5) == 0)
        {
            if (len > 0)
            {
                len += 1;   // there will be a \n between property=value entries
            }
            len += property_l - 5;
            value = GetImageProperty(image,property);
            if (value)
            {
                // add 1 for the = between property and value
                len += 1 + strlen(value);
            }
        }
        property = GetNextImageProperty(image);
    }

    if (len == 0)
    {
        return Qnil;
    }
    str = xmalloc(len);
    len = 0;

    // Copy the exif properties and values into the string.
    ResetImagePropertyIterator(image);
    property = GetNextImageProperty(image);

    while (property)
    {
        property_l = strlen(property);
        if (property_l > 5 && rm_strncasecmp(property, "exif:", 5) == 0)
        {
            if (len > 0)
            {
                str[len++] = '\n';
            }
            memcpy(str+len, property+5, property_l-5);
            len += property_l - 5;
            value = GetImageProperty(image,property);
            if (value)
            {
                value_l = strlen(value);
                str[len++] = '=';
                memcpy(str+len, value, value_l);
                len += value_l;
            }
        }
        property = GetNextImageProperty(image);
    }

    v = rb_str_new(str, len);
    xfree(str);
    return v;

#else

    const char *attr = rm_get_property(image, "EXIF:*");
    return attr ? rb_str_new2(attr) : Qnil;

#endif
}


/*
    Function:   rm_exif_by_number
    Purpose:    replicate old (< 6.3.2) EXIF:! functionality using GetImageProperty
                by returning the exif entries as a single string, separated by \n's.
                Do this so that RMagick.rb works no matter which version of
                ImageMagick is in use.
    Notes:      see magick/identify.c
*/
VALUE
rm_exif_by_number(Image *image)
{
#if defined(HAVE_GETIMAGEPROPERTY)
    const char *property, *value;
    char *str;
    size_t len = 0, property_l, value_l;
    volatile VALUE v;

    (void) GetImageProperty(image, "exif:!");
    ResetImagePropertyIterator(image);
    property = GetNextImageProperty(image);

    // Measure the exif properties and values
    while (property)
    {
        // ignore properties that don't start with "#"
        property_l = strlen(property);
        if (property_l > 1 && property[0] == '#')
        {
            if (len > 0)
            {
                len += 1;   // there will be a \n between property=value entries
            }
            len += property_l;
            value = GetImageProperty(image,property);
            if (value)
            {
                // add 1 for the = between property and value
                len += 1 + strlen(value);
            }
        }
        property = GetNextImageProperty(image);
    }

    if (len == 0)
    {
        return Qnil;
    }
    str = xmalloc(len);
    len = 0;

    // Copy the exif properties and values into the string.
    ResetImagePropertyIterator(image);
    property = GetNextImageProperty(image);

    while (property)
    {
        property_l = strlen(property);
        if (property_l > 1 && property[0] == '#')
        {
            if (len > 0)
            {
                str[len++] = '\n';
            }
            memcpy(str+len, property, property_l);
            len += property_l;
            value = GetImageProperty(image,property);
            if (value)
            {
                value_l = strlen(value);
                str[len++] = '=';
                memcpy(str+len, value, value_l);
                len += value_l;
            }
        }
        property = GetNextImageProperty(image);
    }

    v = rb_str_new(str, len);
    xfree(str);
    return v;

#else

    const char *attr = rm_get_property(image, "EXIF:!");
    return attr ? rb_str_new2(attr) : Qnil;

#endif
}


/*
 *  Extern:     rm_get_geometry
 *  Purpose:    Get the values from a Geometry object and return
 *              them in C variables.
*/
void
rm_get_geometry(
    VALUE geom,
    long *x,
    long *y,
    unsigned long *width,
    unsigned long *height,
    int *flag)
{
    VALUE v;

    v = rb_funcall(geom, rm_ID_x, 0);
    *x = NUM2LONG(v);
    v = rb_funcall(geom, rm_ID_y, 0);
    *y = NUM2LONG(v);
    v = rb_funcall(geom, rm_ID_width, 0);
    *width = NUM2ULONG(v);
    v = rb_funcall(geom, rm_ID_height, 0);
    *height = NUM2ULONG(v);

    // Getting the flag field is a bit more difficult since it's
    // supposed to be an instance of the GeometryValue Enum class. We
    // may not know the VALUE for the GeometryValue class, and we
    // need to check that the flag field is an instance of that class.
    if (flag)
    {
        MagickEnum *magick_enum;

        v = rb_funcall(geom, rm_ID_flag, 0);
        if (!Class_GeometryValue)
        {
            Class_GeometryValue = rb_const_get(Module_Magick, rm_ID_GeometryValue);
        }
        if (CLASS_OF(v) != Class_GeometryValue)
        {
            rb_raise(rb_eTypeError, "wrong enumeration type - expected %s, got %s"
                        , rb_class2name(Class_GeometryValue),rb_class2name(CLASS_OF(v)));
        }
        Data_Get_Struct(v, MagickEnum, magick_enum);
        *flag = magick_enum->val;
    }

}


/*
 *  Extern:     rm_clone_image
 *  Purpose:    clone an image, handle errors
 *  Notes:      don't trace creation - the clone may not be used as an Image
 *              object. Let the caller do the trace if desired.
 */
Image *
rm_clone_image(Image *image)
{
    Image *clone;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    clone = CloneImage(image, 0, 0, MagickTrue, &exception);
    if (!clone)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    rm_check_exception(&exception, clone, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    return clone;
}


/*
    Extern:     rm_progress_monitor
    Purpose:    SetImage(Info)ProgressMonitor exit
    Notes:      ImageMagick's "tag" argument is unused. We pass along the method name instead.
*/
MagickBooleanType
rm_progress_monitor(
    const char *tag,
    const MagickOffsetType of,
    const MagickSizeType sp,
    void *client_data)
{
    volatile VALUE rval;
    volatile VALUE method, offset, span;

    tag = tag;      // defeat gcc message

#if defined(HAVE_LONG_LONG)     // defined in Ruby's defines.h
    offset = rb_ll2inum(of);
    span = rb_ull2inum(sp);
#else
    offset = rb_int2big((long)of);
    span = rb_uint2big((unsigned long)sp);
#endif

    method = rb_str_new2(rb_id2name(THIS_FUNC()));

    rval = rb_funcall((VALUE)client_data, rm_ID_call, 3, method, offset, span);

    return RTEST(rval) ? MagickTrue : MagickFalse;
}


/*
    Extern:     rm_split
    Purpose:    Remove the ImageMagick links between images in an scene
                sequence.
    Notes:      The images remain grouped via the ImageList
*/
void
rm_split(Image *image)
{

    if (!image)
    {
        rb_bug("RMagick FATAL: split called with NULL argument.");
    }
    while (image)
    {
        (void) RemoveFirstImageFromList(&image);
    }
}


/*
    Extern:     rm_check_image_exception
    Purpose:    If an ExceptionInfo struct in a list of images indicates a warning,
                issue a warning message. If an ExceptionInfo struct indicates an
                error, raise an exception and optionally destroy the images.
 */
void
rm_check_image_exception(Image *imglist, ErrorRetention retention)
{
    ExceptionInfo exception;
    Image *badboy = NULL;
    Image *image;

    if (imglist == NULL)
    {
        return;
    }

    GetExceptionInfo(&exception);

    // Find the image with the highest severity
    image = GetFirstImageInList(imglist);
    while (image)
    {
        if (image->exception.severity != UndefinedException)
        {
            if (!badboy || image->exception.severity > badboy->exception.severity)
            {
                badboy = image;
                InheritException(&exception, &badboy->exception);
            }

            ClearMagickException(&image->exception);
        }
        image = GetNextImageInList(image);
    }

    if (badboy)
    {
        rm_check_exception(&exception, imglist, retention);
    }

    (void) DestroyExceptionInfo(&exception);
}


/*
 *  Extern:     rm_check_exception
 *  Purpose:    Call handle_exception if there is an exception to handle.
 */
void
rm_check_exception(ExceptionInfo *exception, Image *imglist, ErrorRetention retention)
{
    if (exception->severity == UndefinedException)
    {
        return;
    }

    handle_exception(exception, imglist, retention);
}


/*
 *  Static:     handle_exception
 *  Purpose:    called when rm_check_exception determines that we need
 *              to either issue a warning message or raise an exception.
 *              This function allocates a bunch of stack so we don't call
 *              it unless we have to.
*/
static void
handle_exception(ExceptionInfo *exception, Image *imglist, ErrorRetention retention)
{

    char reason[500];
    char desc[500];
    char msg[sizeof(reason)+sizeof(desc)+20];

    memset(msg, 0, sizeof(msg));


    // Handle simple warning
    if (exception->severity < ErrorException)
    {
#if defined(HAVE_SNPRINTF)
        snprintf(msg, sizeof(msg)-1, "RMagick: %s%s%s",
#else
        sprintf(msg, "RMagick: %.500s%s%.500s",
#endif
            GetLocaleExceptionMessage(exception->severity, exception->reason),
            exception->description ? ": " : "",
            exception->description ? GetLocaleExceptionMessage(exception->severity, exception->description) : "");
        msg[sizeof(msg)-1] = '\0';
        rb_warning(msg);

        // Caller deletes ExceptionInfo...

        return;
    }

    // Raise an exception. We're not coming back...


    // Newly-created images should be destroyed, images that are part
    // of image objects should be retained but split.
    if (imglist)
    {
        if (retention == DestroyOnError)
        {
            (void) DestroyImageList(imglist);
            imglist = NULL;
        }
        else
        {
            rm_split(imglist);
        }
    }


    // Clone the ExceptionInfo with all arguments on the stack.
    memset(reason, 0, sizeof(reason));
    memset(desc, 0, sizeof(desc));

    if (exception->reason)
    {
        strncpy(reason, exception->reason, sizeof(reason)-1);
        reason[sizeof(reason)-1] = '\0';
    }
    if (exception->description)
    {
        strncpy(desc, exception->description, sizeof(desc)-1);
        desc[sizeof(desc)-1] = '\0';
    }


#if defined(HAVE_SNPRINTF)
    snprintf(msg, sizeof(msg)-1, "%s%s%s",
        GetLocaleExceptionMessage(exception->severity, reason),
        desc[0] ? ": " : "",
        desc[0] ? GetLocaleExceptionMessage(exception->severity, desc) : "");
#else
    sprintf(msg, "%.*s%s%.*s",
        sizeof(reason)-1, GetLocaleExceptionMessage(exception->severity, reason),
        desc[0] ? ": " : "",
        sizeof(desc)-1, desc[0] ? GetLocaleExceptionMessage(exception->severity, desc) : "");
#endif

    msg[sizeof(msg)-1] = '\0';

    (void) DestroyExceptionInfo(exception);
    rm_magick_error(msg, NULL);

}


/*
 *  Extern:     rm_ensure_result
 *  Purpose:    RMagick expected a result. If it got NULL instead raise an exception.
 */
void
rm_ensure_result(Image *image)
{
    if (!image)
    {
        rb_raise(rb_eRuntimeError, MagickPackageName " library function failed to return a result.");
    }
}

