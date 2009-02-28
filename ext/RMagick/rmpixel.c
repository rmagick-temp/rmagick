/* $Id: rmpixel.c,v 1.4 2009/02/28 23:50:36 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2009 by Timothy P. Hunter
| Name:     rmpixel.c
| Author:   Tim Hunter
| Purpose:  Contains Pixel class methods.
\============================================================================*/

#include "rmagick.h"




static void Color_Name_to_PixelPacket(PixelPacket *, VALUE);




/*
    Extern:     destroy_Pixel
    Purpose:    Free the storage associated with a Pixel object
*/
void
destroy_Pixel(Pixel *pixel)
{
    xfree(pixel);
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

    image = AcquireImage(info);

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
    Static:     Pixel_from_MagickPixelPacket
    Purpose:    Create a Magick::Pixel object from a MagickPixelPacket structure.
    Notes:      bypasses normal Pixel.new, Pixel#initialize methods
*/
VALUE
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
    Method:     Pixel#marshal_dump
    Purpose:    Support Marshal.dump
*/
VALUE
Pixel_marshal_dump(VALUE self)
{
    Pixel *pixel;
    volatile VALUE dpixel;

    Data_Get_Struct(self, Pixel, pixel);
    dpixel = rb_hash_new();
    rb_hash_aset(dpixel, CSTR2SYM("red"), QUANTUM2NUM(pixel->red));
    rb_hash_aset(dpixel, CSTR2SYM("green"), QUANTUM2NUM(pixel->green));
    rb_hash_aset(dpixel, CSTR2SYM("blue"), QUANTUM2NUM(pixel->blue));
    rb_hash_aset(dpixel, CSTR2SYM("opacity"), QUANTUM2NUM(pixel->opacity));
    return dpixel;
}


/*
    Method:     Pixel#marshal_load
    Purpose:    Support Marshal.load
*/
VALUE
Pixel_marshal_load(VALUE self, VALUE dpixel)
{
    Pixel *pixel;

    Data_Get_Struct(self, Pixel, pixel);
    pixel->red = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("red")));
    pixel->green = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("green")));
    pixel->blue = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("blue")));
    pixel->opacity = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("opacity")));
    return self;
}


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
    image = AcquireImage(info);
    image->depth = depth;
    image->matte = matte;
    (void) DestroyImageInfo(info);

    GetMagickPixelPacket(image, &mpp);
    rm_set_magick_pixel_packet(pixel, &mpp);

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
    Static:     rm_set_magick_pixel_packet
    Purpose:    Convert a PixelPacket to a MagickPixelPacket
    Notes:      Same code as the private function SetMagickPixelPacket
                in ImageMagick.
*/
void
rm_set_magick_pixel_packet(Pixel *pixel, MagickPixelPacket *pp)
{
    pp->red     = (MagickRealType) pixel->red;
    pp->green   = (MagickRealType) pixel->green;
    pp->blue    = (MagickRealType) pixel->blue;
    pp->opacity = (MagickRealType) pixel->opacity;
    pp->index   = (MagickRealType) 0.0;
}

