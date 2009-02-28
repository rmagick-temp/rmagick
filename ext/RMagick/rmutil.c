/* $Id: rmutil.c,v 1.178 2009/02/28 23:50:36 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2009 by Timothy P. Hunter
| Name:     rmutil.c
| Author:   Tim Hunter
| Purpose:  Utility functions for RMagick
\============================================================================*/

#include "rmagick.h"
#include <errno.h>

static void handle_exception(ExceptionInfo *, Image *, ErrorRetention);


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
    Extern:     rm_pixelpacket_to_color_name
    Purpose:    Map the color intensity to a named color
    Returns:    the named color as a String
    Notes:      See below for the equivalent function that accepts an Info
                structure instead of an Image.
*/
VALUE
rm_pixelpacket_to_color_name(Image *image, PixelPacket *color)
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
    Extern:     rm_pixelpacket_to_color_name_info
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
rm_pixelpacket_to_color_name_info(Info *info, PixelPacket *color)
{
    Image *image;
    Info *my_info;
    volatile VALUE color_name;

    my_info = info ? info : CloneImageInfo(NULL);

    image = AcquireImage(info);
    image->matte = MagickFalse;
    color_name = rm_pixelpacket_to_color_name(image, color);
    (void) DestroyImage(image);
    if (!info)
    {
        (void) DestroyImageInfo(my_info);
    }

    return color_name;
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
    Function:   rm_set_user_artifact
    Purpose:    If a "user" option is present in the Info, assign its value to
                a "user" artifact in each image.
*/
void rm_set_user_artifact(Image *images, Info *info)
{
#if defined(HAVE_SETIMAGEARTIFACT)
    Image *image;
    const char *value;

    value = GetImageOption(info, "user");
    if (value)
    {
        image = GetFirstImageInList(images);
        while (image)
        {
            (void) SetImageArtifact(image, "user", value);
            image = GetNextImageInList(image);
        }
    }
#else
    images = images;
    info = info;
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
    Static:     copy_options
    Purpose:    copy image options from the Info structure to the Image structure
*/
#if defined(HAVE_SETIMAGEARTIFACT)
static void copy_options(Image *image, Info *info)
{
    char property[MaxTextExtent];
    const char *value, *option;

    ResetImageOptionIterator(info);
    for (option = GetNextImageOption(info); option; option = GetNextImageOption(info))
    {
        value = GetImageOption(info,option);
        if (value)
        {
            strncpy(property, value, MaxTextExtent);
            property[MaxTextExtent-1] = '\0';
            (void) SetImageArtifact(image, property, value);
        }
    }
}
#endif


/*
    Extern:     rm_sync_image_options
    Purpose:    Propagate ImageInfo values to the Image
    Ref:        SyncImageSettings (in mogrify.c)
*/
void rm_sync_image_options(Image *image, Info *info)
{
    MagickStatusType flags;
    GeometryInfo geometry_info;
    const char *option;

    // The option strings will be set only when their attribute values were
    // set in the optional argument block.
    option = GetImageOption(info,"background");
    if (option)
    {
        image->background_color = info->background_color;
    }

    option = GetImageOption(info,"bordercolor");
    if (option)
    {
        image->border_color = info->border_color;
    }

    if (info->colorspace != UndefinedColorspace)
    {
        image->colorspace = info->colorspace;
    }

    if (info->compression != UndefinedCompression)
    {
        image->compression = info->compression;
    }

    option = GetImageOption(info, "delay");
    if (option)
    {
        image->delay = strtoul(option, NULL, 0);
    }

    if (info->density)
    {
        flags = ParseGeometry(info->density, &geometry_info);
        image->x_resolution = geometry_info.rho;
        image->y_resolution = geometry_info.sigma;
        if ((flags & SigmaValue) == 0)
        {
            image->y_resolution = image->x_resolution;
        }
    }

    if (info->depth != 0)
    {
        image->depth = info->depth;
    }

    option = GetImageOption(info, "dispose");
    if (option)
    {
        image->dispose = rm_dispose_to_enum(option);
    }

    if (info->extract)
    {
        ParseAbsoluteGeometry(info->extract, &image->extract_info);
    }

    if (info->fuzz != 0.0)
    {
        image->fuzz = info->fuzz;
    }

    option = GetImageOption(info, "gravity");
    if (option)
    {
        image->gravity = rm_gravity_to_enum(option);
    }

    if (info->interlace != NoInterlace)
    {
        image->interlace = info->interlace;
    }

    option = GetImageOption(info,"mattecolor");
    if (option)
    {
        image->matte_color = info->matte_color;
    }

    if (info->orientation != UndefinedOrientation)
    {
        image->orientation = info->orientation;
    }

    if (info->page)
    {
        (void)ParseAbsoluteGeometry(info->page, &image->page);
    }

    if (info->quality != 0UL)
    {
        image->quality = info->quality;
    }

    option = GetImageOption(info, "scene");
    if (option)
    {
        image->scene = info->scene;
    }

#if defined(HAVE_ST_TILE_OFFSET)
    option = GetImageOption(info, "tile-offset");
    if (option)
    {
        (void)ParseAbsoluteGeometry(option, &image->tile_offset);
    }
#endif

    option = GetImageOption(info, "transparent");
    if (option)
    {
        image->transparent_color = info->transparent_color;
    }

#if defined(HAVE_ST_TYPE)
    if (info->type != UndefinedType)
    {
        image->type = info->type;
    }
#endif

    if (info->units != UndefinedResolution)
    {
        if (image->units != info->units)
        {
            switch (image->units)
            {
              case PixelsPerInchResolution:
              {
                if (info->units == PixelsPerCentimeterResolution)
                {
                    image->x_resolution /= 2.54;
                    image->y_resolution /= 2.54;
                }
                break;
              }
              case PixelsPerCentimeterResolution:
              {
                if (info->units == PixelsPerInchResolution)
                {
                    image->x_resolution *= 2.54;
                    image->y_resolution *= 2.54;
                }
                break;
              }
              default:
                break;
            }
        }

        image->units = info->units;
    }

#if defined(HAVE_SETIMAGEARTIFACT)
    copy_options(image, info);
#endif
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
 *  Extern:     rm_warning_handler
 *  Purpose:    called from ImageMagick for a warning
*/
void
rm_warning_handler(const ExceptionType severity, const char *reason, const char *description)
{
    ExceptionType dummy;

    rb_warning("RMagick: %s: `%s'", reason, description);
    dummy = severity;
    dummy = dummy;
}


/*
 *  Extern:     rm_error_handler
 *  Purpose:    called from ImageMagick for a error
*/
void
rm_error_handler(const ExceptionType severity, const char *reason, const char *description)
{
    char msg[500];
    int len;
    ExceptionType dummy;

    memset(msg, 0, sizeof(msg));
#if defined(HAVE_SNPRINTF)
    len = snprintf(msg, sizeof(msg), "%s: `%s'", reason, description);
#else
    len = sprintf(msg, "%.250s: `%.240s'", reason, description);
#endif
    msg[len] = '\0';

    rm_magick_error(msg, NULL);
    dummy = severity;
    dummy = dummy;
}


/*
 *  Extern:     rm_fatal_error_handler
 *  Purpose:    called from ImageMagick for a fatal error
*/
void
rm_fatal_error_handler(const ExceptionType severity, const char *reason, const char *description)
{
    rb_raise(Class_FatalImageMagickError, GetLocaleExceptionMessage(severity, reason));
    description = description;
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

