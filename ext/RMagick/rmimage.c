/**************************************************************************//**
 * Image class method definitions for RMagick.
 *
 * Copyright &copy; 2002 - 2009 by Timothy P. Hunter
 *
 * Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or
 *
 * @file     rmimage.c
 * @version  $Id: rmimage.c,v 1.360 2009/12/21 10:34:57 baror Exp $
 * @author   Tim Hunter
 ******************************************************************************/

#include "rmagick.h"
#include "magick/xwindow.h"     // XImageInfo

/** Method that effects an image */
typedef Image *(effector_t)(const Image *, const double, const double, ExceptionInfo *);
/** Method that flips an image */
typedef Image *(flipper_t)(const Image *, ExceptionInfo *);
/** Method that magnifies an image */
typedef Image *(magnifier_t)(const Image *, ExceptionInfo *);
/** Method that reads an image */
typedef Image *(reader_t)(const Info *, ExceptionInfo *);
/** Method that scales an image */
typedef Image *(scaler_t)(const Image *, const unsigned long, const unsigned long, ExceptionInfo *);
/** Method that computes threshold on an image */
typedef MagickBooleanType (thresholder_t)(Image *, const char *);
/** Method that transforms an image */
typedef Image *(xformer_t)(const Image *, const RectangleInfo *, ExceptionInfo *);

static VALUE cropper(int, int, VALUE *, VALUE);
static VALUE effect_image(VALUE, int, VALUE *, effector_t);
static VALUE flipflop(int, VALUE, flipper_t);
static VALUE rd_image(VALUE, VALUE, reader_t);
static VALUE rotate(int, int, VALUE *, VALUE);
static VALUE scale(int, int, VALUE *, VALUE, scaler_t);
static VALUE threshold_image(int, VALUE *, VALUE, thresholder_t);
static VALUE xform_image(int, VALUE, VALUE, VALUE, VALUE, VALUE, xformer_t);
static VALUE array_from_images(Image *);
static void call_trace_proc(Image *, const char *);

static const char *BlackPointCompensationKey = "PROFILE:black-point-compensation";




/**
 * Call Adaptive(Blur|Sharpen)Image.
 *
 * No Ruby usage (internal function)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param fp pointer to the function to call
 * @return a new image
 */
static VALUE
adaptive_method(int argc, VALUE *argv, VALUE self
                , Image *fp(const Image *, const double, const double, ExceptionInfo *))
{
    Image *image, *new_image;
    double radius = 0.0;
    double sigma = 1.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 2:
            sigma = NUM2DBL(argv[1]);
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = (fp)(image, radius, sigma, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}



/**
 * Call Adaptive(Blur|Sharpen)ImageChannel.
 *
 * No Ruby usage (internal function)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param fp pointer to the function to call
 * @return a new image
 */
static VALUE
adaptive_channel_method(int argc, VALUE *argv, VALUE self
                        , Image *fp(const Image *, const ChannelType, const double, const double, ExceptionInfo *))
{
    Image *image, *new_image;
    double radius = 0.0;
    double sigma = 1.0;
    ExceptionInfo exception;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    switch (argc)
    {
        case 2:
            sigma = NUM2DBL(argv[1]);
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            raise_ChannelType_error(argv[argc-1]);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = (fp)(image, channels, radius, sigma, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call AdaptiveBlurImage.
 *
 * Ruby usage:
 *   - @verbatim Image#adaptive_blur @endverbatim
 *   - @verbatim Image#adaptive_blur(radius) @endverbatim
 *   - @verbatim Image#adaptive_blur(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_adaptive_blur(int argc, VALUE *argv, VALUE self)
{
    return adaptive_method(argc, argv, self, AdaptiveBlurImage);
}



/**
 * Call AdaptiveBlurImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#adaptive_blur_channel @endverbatim
 *   - @verbatim Image#adaptive_blur_channel(radius) @endverbatim
 *   - @verbatim Image#adaptive_blur_channel(radius, sigma) @endverbatim
 *   - @verbatim Image#adaptive_blur_channel(radius, sigma, channel) @endverbatim
 *   - @verbatim Image#adaptive_blur_channel(radius, sigma, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_adaptive_blur_channel(int argc, VALUE *argv, VALUE self)
{
    return adaptive_channel_method(argc, argv, self, AdaptiveBlurImageChannel);
}


/**
 * Call AdaptiveResizeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#adaptive_resize(scale_val) @endverbatim
 *   - @verbatim Image#adaptive_resize(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_adaptive_resize(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    unsigned long rows, columns;
    double scale_val, drows, dcols;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 2:
            rows = NUM2ULONG(argv[1]);
            columns = NUM2ULONG(argv[0]);
            break;
        case 1:
            scale_val = NUM2DBL(argv[0]);
            if (scale_val < 0.0)
            {
                rb_raise(rb_eArgError, "invalid scale_val value (%g given)", scale_val);
            }
            drows = scale_val * image->rows + 0.5;
            dcols = scale_val * image->columns + 0.5;
            if (drows > (double)ULONG_MAX || dcols > (double)ULONG_MAX)
            {
                rb_raise(rb_eRangeError, "resized image too big");
            }
            rows = (unsigned long) drows;
            columns = (unsigned long) dcols;
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = AdaptiveResizeImage(image, columns, rows, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}



/**
 * Call AdaptiveSharpenImage.
 *
 * Ruby usage:
 *   - @verbatim Image#adaptive_sharpen @endverbatim
 *   - @verbatim Image#adaptive_sharpen(radius) @endverbatim
 *   - @verbatim Image#adaptive_sharpen(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_adaptive_sharpen(int argc, VALUE *argv, VALUE self)
{
    return adaptive_method(argc, argv, self, AdaptiveSharpenImage);
}



/**
 * Call AdaptiveSharpenImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#adaptive_sharpen_channel(radius=0.0, sigma=1.0[, channel...]) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_adaptive_sharpen_channel(int argc, VALUE *argv, VALUE self)
{
    return adaptive_channel_method(argc, argv, self, AdaptiveSharpenImageChannel);
}



/**
 * Selects an individual threshold for each pixel based on the range of
 * intensity values in its local neighborhood. This allows for thresholding of
 * an image whose global intensity histogram doesn't contain distinctive peaks.
 *
 * Ruby usage:
 *   - @verbatim Image#adaptive_threshold @endverbatim
 *   - @verbatim Image#adaptive_threshold(width) @endverbatim
 *   - @verbatim Image#adaptive_threshold(width, height) @endverbatim
 *   - @verbatim Image#adaptive_threshold(width, height, offset) @endverbatim
 *
 * Notes:
 *   - Default width is 3
 *   - Default height is 3
 *   - Default offset is 0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_adaptive_threshold(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    unsigned long width = 3, height = 3;
    long offset = 0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 3:
            offset = NUM2LONG(argv[2]);
        case 2:
            height = NUM2ULONG(argv[1]);
        case 1:
            width  = NUM2ULONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 3)", argc);
    }

    GetExceptionInfo(&exception);
    new_image = AdaptiveThresholdImage(image, width, height, offset, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Set the image composite mask. 
 *
 * Ruby usage:
 *   - @verbatim Image#add_compose_mask(mask) @endverbatim
 *
 * @param self this object
 * @param mask the composite mask
 * @return self
 * @see Image_mask
 * @see Image_delete_compose_mask
 * @see SetImageMask in ImageMagick
 */
VALUE
Image_add_compose_mask(VALUE self, VALUE mask)
{
    Image *image;
    Image *mask_image = NULL;

    image = rm_check_frozen(self);
    mask_image = rm_check_destroyed(mask);
    if (image->columns != mask_image->columns || image->rows != mask_image->rows)
    {
        rb_raise(rb_eArgError, "mask must be the same size as image");
    }

    // Delete any previously-existing mask image.
    // Store a clone of the new mask image.
    (void) SetImageMask(image, mask_image);
    (void) NegateImage(image->mask, MagickFalse);

    // Since both Set and GetImageMask clone the mask image I don't see any
    // way to negate the mask without referencing it directly. Sigh.

    return self;
}


/**
 * Add random noise to a copy of the image.
 *
 * Ruby usage:
 *   - @verbatim Image#add_noise(noise_type) @endverbatim
 *
 * @param self this object
 * @param noise the noise
 * @return a new image
 */
VALUE
Image_add_noise(VALUE self, VALUE noise)
{
    Image *image, *new_image;
    NoiseType noise_type;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    VALUE_TO_ENUM(noise, noise_type, NoiseType);

    GetExceptionInfo(&exception);
    new_image = AddNoiseImage(image, noise_type, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}

/**
 * Add random noise to a copy of the image.
 *
 * Ruby usage:
 *   - @verbatim Image#add_noise_channel(noise_type) @endverbatim
 *   - @verbatim Image#add_noise_channel(noise_type,channel) @endverbatim
 *   - @verbatim Image#add_noise_channel(noise_type,channel,channel,...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_add_noise_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    NoiseType noise_type;
    ExceptionInfo exception;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There must be 1 remaining argument.
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "missing noise type argument");
    }
    else if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    VALUE_TO_ENUM(argv[0], noise_type, NoiseType);
    channels &= ~OpacityChannel;

    GetExceptionInfo(&exception);
    new_image = AddNoiseImageChannel(image, channels, noise_type, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Add all the profiles in the specified file.
 *
 * Ruby usage:
 *   - @verbatim Image#add_profile(name) @endverbatim
 *
 * @param self this object
 * @param name the profile filename
 * @return self
 */
VALUE
Image_add_profile(VALUE self, VALUE name)
{
    // ImageMagick code based on the code for the "-profile" option in mogrify.c
    Image *image, *profile_image;
    ImageInfo *info;
    ExceptionInfo exception;
    char *profile_name;
    char *profile_filename = NULL;
    long profile_filename_l = 0;
    const StringInfo *profile;

    image = rm_check_frozen(self);

    // ProfileImage issues a warning if something goes wrong.
    profile_filename = rm_str2cstr(name, &profile_filename_l);

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    profile = GetImageProfile(image, "iptc");
    if (profile)
    {
        info->profile = (void *)CloneStringInfo(profile);
    }
    strncpy(info->filename, profile_filename, min((size_t)profile_filename_l, sizeof(info->filename)));
    info->filename[MaxTextExtent-1] = '\0';

    GetExceptionInfo(&exception);
    profile_image = ReadImage(info, &exception);
    (void) DestroyImageInfo(info);
    rm_check_exception(&exception, profile_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(profile_image);

    ResetImageProfileIterator(profile_image);
    profile_name = GetNextImageProfile(profile_image);
    while (profile_name)
    {
        profile = GetImageProfile(profile_image, profile_name);
        if (profile)
        {
            (void)ProfileImage(image, profile_name, GetStringInfoDatum(profile)
                               , GetStringInfoLength(profile), MagickFalse);
            if (image->exception.severity >= ErrorException)
            {
                break;
            }
        }
        profile_name = GetNextImageProfile(profile_image);
    }

    (void) DestroyImage(profile_image);
    rm_check_image_exception(image, RetainOnError);


    return self;
}



/**
 * Calls SetImageAlphaChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#alpha(type) @endverbatim
 *
 * Notes:
 *   - Replaces matte=, alpha=
 *   - Originally there was an alpha attribute getter and setter. These are
 *     replaced with alpha? and alpha(type). We still define (but don't
 *     document) alpha=. For backward compatibility, if this method is called
 *     without an argument, make it act like the old alpha getter and return
 *     true if the matte channel is active, false otherwise.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the type (or true/false if called without an argument, see above)
 */
VALUE
Image_alpha(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    AlphaChannelType alpha;


    // For backward compatibility, make alpha() act like alpha?
    if (argc == 0)
    {
        return Image_alpha_q(self);
    }
    else if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1)", argc);
    }


    image = rm_check_frozen(self);
    VALUE_TO_ENUM(argv[0], alpha, AlphaChannelType);

#if defined(HAVE_SETIMAGEALPHACHANNEL)
    // Added in 6.3.6-9
    (void) SetImageAlphaChannel(image, alpha);
    rm_check_image_exception(image, RetainOnError);
#else
    switch (alpha)
    {
        case ActivateAlphaChannel:
            image->matte = MagickTrue;
            break;

        case DeactivateAlphaChannel:
            image->matte = MagickFalse;
            break;

        case ResetAlphaChannel:
            if (image->matte == MagickFalse)
            {
                (void) SetImageOpacity(image, OpaqueOpacity);
                rm_check_image_exception(image, RetainOnError);
            }
            break;

        case SetAlphaChannel:
            (void) CompositeImage(image, CopyOpacityCompositeOp, image, 0, 0);
            rm_check_image_exception(image, RetainOnError);
            break;

        default:
            rb_raise(rb_eArgError, "unknown AlphaChannelType value");
            break;
    }
#endif

    return argv[0];
}



/**
 * Determine whether the image's alpha channel is activated.
 *
 * Ruby usage:
 *   - @verbatim Image#alpha? @endverbatim
 *
 * Notes:
 *   - Replaces Image#matte
 *
 * @param self this object
 * @return true if the image's alpha channel is activated
 */
VALUE
Image_alpha_q(VALUE self)
{
    Image *image = rm_check_destroyed(self);
#if defined(HAVE_GETIMAGEALPHACHANNEL)
    return GetImageAlphaChannel(image) ? Qtrue : Qfalse;
#else
    return image->matte ? Qtrue : Qfalse;
#endif
}


/**
 * Equivalent to -alpha option.
 *
 * Ruby usage:
 *   - @verbatim Image#alpha=(alpha) @endverbatim
 *
 * @param self this object
 * @param type the alpha type
 * @return alpha
 * @deprecated This method has been deprecated. Please use Image_alpha.
 * @see Image_alpha
 * @see mogrify.c (in ImageMagick)
 */
VALUE
Image_alpha_eq(VALUE self, VALUE type)
{
    VALUE argv[1];
    argv[0] = type;
    Image_alpha(1, argv, self);
    return type;
}


/**
 * Transform an image as dictated by the affine matrix argument.
 *
 * Ruby usage:
 *   - @verbatim Image#affine_transform(affine_matrix) @endverbatim
 *
 * @param self this object
 * @param affine the affine matrix
 * @return a new image
 */
VALUE
Image_affine_transform(VALUE self, VALUE affine)
{
    Image *image, *new_image;
    ExceptionInfo exception;
    AffineMatrix matrix;

    image = rm_check_destroyed(self);

    // Convert Magick::AffineMatrix to AffineMatrix structure.
    Export_AffineMatrix(&matrix, affine);

    GetExceptionInfo(&exception);
    new_image = AffineTransformImage(image, &matrix, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}

/**
 * Return the image property associated with "key".
 *
 * Ruby usage:
 *   - @verbatim Image#["key"] @endverbatim
 *   - @verbatim Image#[:key] @endverbatim
 *
 * Notes:
 *   - Use Image#[]= (aset) to establish more properties or change the value of
 *     an existing property.
 *
 * @param self this object
 * @param key_arg the key to get
 * @return property value or nil if key doesn't exist
 */
VALUE
Image_aref(VALUE self, VALUE key_arg)
{
    Image *image;
    const char *key;
    const char *attr;

    image = rm_check_destroyed(self);

    switch (TYPE(key_arg))
    {
        case T_NIL:
            return Qnil;

        case T_SYMBOL:
            key = rb_id2name((ID)SYM2ID(key_arg));
            break;

        default:
            key = StringValuePtr(key_arg);
            if (*key == '\0')
            {
                return Qnil;
            }
            break;
    }


    if (rm_strcasecmp(key, "EXIF:*") == 0)
    {
        return rm_exif_by_entry(image);
    }
    else if (rm_strcasecmp(key, "EXIF:!") == 0)
    {
        return rm_exif_by_number(image);
    }

    attr = rm_get_property(image, key);
    return attr ? rb_str_new2(attr) : Qnil;
}

/**
 * Update or add image attribute "key".
 *
 * Ruby usage:
 *   - @verbatim Image#["key"] = attr @endverbatim
 *   - @verbatim Image#[:key] = attr @endverbatim
 *
 * Notes:
 *   - Specify attr=nil to remove the key from the list.  
 *   - SetImageProperty normally APPENDS the new value to any existing value.
 *     Since this usage is tremendously counter-intuitive, this function always
 *     deletes the existing value before setting the new value.
 *   - There's no use checking the return value since SetImageProperty returns
 *     "False" for many reasons, some legitimate.
 *
 * @param self this object
 * @param key_arg the key to set
 * @param attr_arg the value to which to set it
 * @return self
 */
VALUE
Image_aset(VALUE self, VALUE key_arg, VALUE attr_arg)
{
    Image *image;
    const char *key;
    char *attr;
    unsigned int okay;

    image = rm_check_frozen(self);

    attr = attr_arg == Qnil ? NULL : StringValuePtr(attr_arg);

    switch (TYPE(key_arg))
    {
        case T_NIL:
            return self;

        case T_SYMBOL:
            key = rb_id2name((ID)SYM2ID(key_arg));
            break;

        default:
            key = StringValuePtr(key_arg);
            if (*key == '\0')
            {
                return self;
            }
            break;
    }


    // Delete existing value. SetImageProperty returns False if
    // the attribute doesn't exist - we don't care.
    (void) rm_set_property(image, key, NULL);
    // Set new value
    if (attr)
    {
        okay = rm_set_property(image, key, attr);
        if (!okay)
        {
            rb_warning("SetImageProperty failed (probably out of memory)");
        }
    }
    return self;
}


/**
 * Handle #transverse, #transform methods.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @param fp the transverse/transform method to call
 * @return self if bang, otherwise a new image
 */
static VALUE
crisscross(int bang, VALUE self, Image *fp(const Image *, ExceptionInfo *))
{
    Image *image, *new_image;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);
    GetExceptionInfo(&exception);

    new_image = (fp)(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);

}



/**
 * Handle #auto_gamma_channel, #auto_level_channel methods.
 *
 * No Ruby usage (internal function)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param fp the channel method to call
 * @return a new image
 */
static VALUE
auto_channel(int argc, VALUE *argv, VALUE self, MagickBooleanType (*fp)(Image *, const ChannelType))
{
    Image *image, *new_image;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    new_image = rm_clone_image(image);
    (void) (fp)(new_image, channels);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Get/set the auto Gamma channel
 *
 * Ruby usage:
 *   - @verbatim Image#auto_gamma_channel @endverbatim
 *   - @verbatim Image#auto_gamma_channel channel @endverbatim
 *   - @verbatim Image#auto_gamma_channel channel, ... @endverbatim
 *
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_auto_gamma_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_AUTOGAMMAIMAGECHANNEL)
    return auto_channel(argc, argv, self, AutoGammaImageChannel);
#else
    rm_not_implemented();
    return (VALUE) 0;
    argc = argc;
    argv = argv;
    self = self;
#endif
}


/**
 * Get/set the auto level channel
 *
 * Ruby usage:
 *   - @verbatim Image#auto_level_channel @endverbatim
 *   - @verbatim Image#auto_level_channel channel @endverbatim
 *   - @verbatim Image#auto_level_channel channel, ... @endverbatim
 *
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_auto_level_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_AUTOLEVELIMAGECHANNEL)
    return auto_channel(argc, argv, self, AutoLevelImageChannel);
#else
    rm_not_implemented();
    return (VALUE)0;
    argc = argc;
    argv = argv;
    self = self;
#endif
}


/**
 * Implement mogrify's -auto_orient option automatically orient image based on
 * EXIF orientation value.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see mogrify.c (in ImageMagick 6.2.8)
 */
static VALUE
auto_orient(int bang, VALUE self)
{
    Image *image;
    volatile VALUE new_image;
    VALUE degrees[1];

    Data_Get_Struct(self, Image, image);

    switch (image->orientation)
    {
        case TopRightOrientation:
            new_image = flipflop(bang, self, FlopImage);
            break;

        case BottomRightOrientation:
            degrees[0] = rb_float_new(180.0);
            new_image = rotate(bang, 1, degrees, self);
            break;

        case BottomLeftOrientation:
            new_image = flipflop(bang, self, FlipImage);
            break;

        case LeftTopOrientation:
            new_image = crisscross(bang, self, TransposeImage);
            break;

        case RightTopOrientation:
            degrees[0] = rb_float_new(90.0);
            new_image = rotate(bang, 1, degrees, self);
            break;

        case RightBottomOrientation:
            new_image = crisscross(bang, self, TransverseImage);
            break;

        case LeftBottomOrientation:
            degrees[0] = rb_float_new(270.0);
            new_image = rotate(bang, 1, degrees, self);
            break;

        default:                // Return IMMEDIATELY
            return bang ? Qnil : Image_copy(self);
            break;
    }


    Data_Get_Struct(new_image, Image, image);
    image->orientation = TopLeftOrientation;

    return new_image;
}


/**
 * Implement mogrify's -auto_orient option automatically orient image based on
 * EXIF orientation value.
 *
 * Ruby usage:
 *   - @verbatim Image#auto_orient @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see mogrify.c (in ImageMagick 6.2.8)
 */
VALUE
Image_auto_orient(VALUE self)
{
    (void) rm_check_destroyed(self);
    return auto_orient(False, self);
}


/**
 * Implement mogrify's -auto_orient option automatically orient image based on
 * EXIF orientation value.
 *
 * Ruby usage:
 *   - @verbatim Image#auto_orient! @endverbatim
 *
 * @param self this object
 * @return nil if the image is already properly oriented, otherwise self
*/
VALUE
Image_auto_orient_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return auto_orient(True, self);
}


/**
 * Return the name of the background color as a String.
 *
 * Ruby usage:
 *   - @verbatim Image#background_color @endverbatim
 *
 * @param self this object
 * @return the background color
 */
VALUE
Image_background_color(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return rm_pixelpacket_to_color_name(image, &image->background_color);
}


/**
 * Set the the background color to the specified color spec.
 *
 * Ruby usage:
 *   - @verbatim Image#background_color= @endverbatim
 *
 * @param self this object
 * @param color the color
 * @return self
 */
VALUE
Image_background_color_eq(VALUE self, VALUE color)
{
    Image *image = rm_check_frozen(self);
    Color_to_PixelPacket(&image->background_color, color);
    return self;
}


/**
 * Return the number of rows (before transformations).
 *
 * Ruby usage:
 *   - @verbatim Image#base_columns @endverbatim
 *
 * @param self this object
 * @return the number of rows
 */
VALUE
Image_base_columns(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return INT2FIX(image->magick_columns);
}

/**
 * Return the image filename (before transformations).
 *
 * Ruby usage:
 *   - @verbatim Image#base_filename @endverbatim
 *
 * @param self this object
 * @return the base image filename (or the current filename if there is no base)
 */
VALUE
Image_base_filename(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    if (*image->magick_filename)
    {
        return rb_str_new2(image->magick_filename);
    }
    else
    {
        return rb_str_new2(image->filename);
    }
}

/**
 * Return the number of rows (before transformations).
 *
 * Ruby usage:
 *   - @verbatim Image#base_rows @endverbatim
 *
 * @param self this object
 * @return the number of rows
 */
VALUE
Image_base_rows(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return INT2FIX(image->magick_rows);
}


/**
 * Get image bias (used when convolving an image).
 *
 * Ruby usage:
 *   - @verbatim Image#bias @endverbatim
 *
 * @param self this object
 * @return the image bias
 */
VALUE
Image_bias(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return rb_float_new(image->bias);
}


/**
 * Set image bias (used when convolving an image).
 *
 * Ruby usage:
 *   - @verbatim Image#bias = a number between 0.0 and 1.0 or "NN%" @endverbatim
 *
 * @param self this object
 * @param pct the bias
 * @return self
 */
VALUE
Image_bias_eq(VALUE self, VALUE pct)
{
    Image *image;
    double bias;

    image = rm_check_frozen(self);
    bias = rm_percentage(pct,1.0);
    image->bias = bias * QuantumRange;

    return self;
}

/**
 * Create a bilevel image.
 *
 * Ruby usage:
 *   - @verbatim Image#bilevel_channel(threshold) @endverbatim
 *   - @verbatim Image#bilevel_channel(threshold, channel) @endverbatim
 *
 * Notes:
 *   - If no channel is specified AllChannels is used
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_bilevel_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "no threshold specified");
    }

    new_image = rm_clone_image(image);

    (void)BilevelImageChannel(new_image, channels, NUM2DBL(argv[0]));
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Return current black point compensation attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#black_point_compensation @endverbatim
 *
 * @param self this object
 * @return the black point compensation
 */
VALUE
Image_black_point_compensation(VALUE self)
{
    Image *image;
    const char *attr;
    volatile VALUE value;

    image = rm_check_destroyed(self);

    attr = rm_get_property(image, BlackPointCompensationKey);
    if (attr && rm_strcasecmp(attr, "true") == 0)
    {
        value = Qtrue;
    }
    else
    {
        value = Qfalse;
    }
    return value;
}


/**
 * Set black point compensation attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#black_point_compensation=true or false @endverbatim
 *
 * @param self this object
 * @param arg the compensation
 * @return self
 */
VALUE
Image_black_point_compensation_eq(VALUE self, VALUE arg)
{
    Image *image;
    const char *value;

    image = rm_check_frozen(self);
    (void) rm_set_property(image, BlackPointCompensationKey, NULL);
    value = RTEST(arg) ? "true" : "false";
    (void) rm_set_property(image, BlackPointCompensationKey, value);

    return self;
}


/**
 * Call BlackThresholdImage.
 *
 * Ruby usage:
 *   - @verbatim Image#black_threshold(red_channel) @endverbatim
 *   - @verbatim Image#black_threshold(red_channel, green_channel) @endverbatim
 *   - @verbatim Image#black_threshold(red_channel, green_channel, blue_channel) @endverbatim
 *   - @verbatim Image#black_threshold(red_channel, green_channel, blue_channel, opacity_channel) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see threshold_image
 * @see Image_white_threshold
 */
VALUE
Image_black_threshold(int argc, VALUE *argv, VALUE self)
{
    return threshold_image(argc, argv, self, BlackThresholdImage);
}


/**
 * Compute offsets using the gravity to determine what the offsets are relative
 * to.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - No return value: modifies x_offset and y_offset directly.
 *
 * @param grav the gravity
 * @param image the destination image
 * @param mark the source image
 * @param x_offset pointer to x offset
 * @param y_offset pointer to y offset
 */
static void
get_relative_offsets(VALUE grav, Image *image, Image *mark, long *x_offset, long *y_offset)
{
    GravityType gravity;

    VALUE_TO_ENUM(grav, gravity, GravityType);

    switch (gravity)
    {
        case NorthEastGravity:
        case EastGravity:
        case SouthEastGravity:
            *x_offset = (long)(image->columns) - (long)(mark->columns) - *x_offset;
            break;
        case NorthGravity:
        case SouthGravity:
        case CenterGravity:
        case StaticGravity:
            *x_offset += (long)(image->columns/2) - (long)(mark->columns/2);
            break;
        default:
            break;
    }
    switch (gravity)
    {
        case SouthWestGravity:
        case SouthGravity:
        case SouthEastGravity:
            *y_offset = (long)(image->rows) - (long)(mark->rows) - *y_offset;
            break;
        case EastGravity:
        case WestGravity:
        case CenterGravity:
        case StaticGravity:
            *y_offset += (long)(image->rows/2) - (long)(mark->rows/2);
            break;
        case NorthEastGravity:
        case NorthGravity:
        default:
            break;
    }

}


/**
 * Compute watermark offsets from gravity type.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - No return value: modifies x_offset and y_offset directly.
 *
 * @param grav the gravity
 * @param image the destination image
 * @param mark the source image
 * @param x_offset pointer to x offset
 * @param y_offset pointer to y offset
 */
static void
get_offsets_from_gravity(GravityType gravity, Image *image, Image *mark
                         , long *x_offset, long *y_offset)
{

    switch (gravity)
    {
        case ForgetGravity:
        case NorthWestGravity:
            *x_offset = 0;
            *y_offset = 0;
            break;
        case NorthGravity:
            *x_offset = ((long)(image->columns) - (long)(mark->columns)) / 2;
            *y_offset = 0;
            break;
        case NorthEastGravity:
            *x_offset = (long)(image->columns) - (long)(mark->columns);
            *y_offset = 0;
            break;
        case WestGravity:
            *x_offset = 0;
            *y_offset = ((long)(image->rows) - (long)(mark->rows)) / 2;
            break;
        case StaticGravity:
        case CenterGravity:
        default:
            *x_offset = ((long)(image->columns) - (long)(mark->columns)) / 2;
            *y_offset = ((long)(image->rows) - (long)(mark->rows)) / 2;
            break;
        case EastGravity:
            *x_offset = (long)(image->columns) - (long)(mark->columns);
            *y_offset = ((long)(image->rows) - (long)(mark->rows)) / 2;
            break;
        case SouthWestGravity:
            *x_offset = 0;
            *y_offset = (long)(image->rows) - (long)(mark->rows);
            break;
        case SouthGravity:
            *x_offset = ((long)(image->columns) - (long)(mark->columns)) / 2;
            *y_offset = (long)(image->rows) - (long)(mark->rows);
            break;
        case SouthEastGravity:
            *x_offset = (long)(image->columns) - (long)(mark->columns);
            *y_offset = (long)(image->rows) - (long)(mark->rows);
            break;
    }
}


/**
 * Called from rb_protect, returns the number if obj is really a numeric value.
 *
 * No Ruby usage (internal function)
 *
 * @param obj the value
 * @return numeric value of obj
 * @todo Make sure that we are really returning the obj here
 */
static VALUE
check_for_long_value(VALUE obj)
{
    long t;
    t = NUM2LONG(obj);
    t = t;      // placate gcc
    return(VALUE)0;
}


/**
 * Compute x- and y-offset of source image for a compositing method.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - No return value: modifies x_offset and y_offset directly.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param dest the destination image
 * @param src the source image
 * @param x_offset pointer to x offset
 * @param y_offset pointer to y offset
 */
static void
get_composite_offsets(int argc, VALUE *argv, Image *dest, Image *src
                      , long *x_offset, long *y_offset)
{
    GravityType gravity;
    int exc = 0;

    if (CLASS_OF(argv[0]) == Class_GravityType)
    {
        VALUE_TO_ENUM(argv[0], gravity, GravityType);

        switch (argc)
        {
            // Gravity + offset(s). Offsets are relative to the image edges
            // as specified by the gravity.
            case 3:
                *y_offset = NUM2LONG(argv[2]);
            case 2:
                *x_offset = NUM2LONG(argv[1]);
                get_relative_offsets(argv[0], dest, src, x_offset, y_offset);
                break;
            case 1:
                // No offsets specified. Compute offset based on the gravity alone.
                get_offsets_from_gravity(gravity, dest, src, x_offset, y_offset);
                break;
        }
    }
    // Gravity not specified at all. Offsets are measured from the
    // NorthWest corner. The arguments must be numbers.
    else
    {
        (void)rb_protect(check_for_long_value, argv[0], &exc);
        if (exc)
        {
            rb_raise(rb_eTypeError, "expected GravityType, got %s"
                     , rb_class2name(CLASS_OF(argv[0])));
        }
        *x_offset = NUM2LONG(argv[0]);
        if (argc > 1)
        {
            *y_offset = NUM2LONG(argv[1]);
        }
    }

}


/**
 * Convert 2 doubles to a blend or dissolve geometry string.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - the geometry buffer needs to be at least 16 characters long.
 *   - For safety's sake this function asserts that it is at least 20 characters
 *     long.
 *   - The percentages must be in the range -1000 < n < 1000. This is far in
 *     excess of what xMagick will allow.
 *
 * @param geometry the geometry
 * @param geometry_l length of geometry
 * @param src_percent source percentage
 * @param dst_percent destination percentage
 */
static void
blend_geometry(char *geometry, size_t geometry_l, double src_percent, double dst_percent)
{
    size_t sz = 0;
    int fw, prec;

    if (fabs(src_percent) >= 1000.0 || fabs(dst_percent) >= 1000.0)
    {
        if (fabs(src_percent) < 1000.0)
        {
            src_percent = dst_percent;
        }
        rb_raise(rb_eArgError, "%g is out of range +/-999.99", src_percent);
    }

    assert(geometry_l >= 20);
    memset(geometry, 0xdf, geometry_l);

    fw = 4;
    prec = 0;
    if (src_percent != floor(src_percent))
    {
        prec = 2;
        fw += 3;
    }

    sz = (size_t)sprintf(geometry, "%*.*f", -fw, prec, src_percent);
    assert(sz < geometry_l);

    sz = strcspn(geometry, " ");

    // if dst_percent was nil don't add to the geometry
    if (dst_percent != -1.0)
    {
        fw = 4;
        prec = 0;
        if (dst_percent != floor(dst_percent))
        {
            prec = 2;
            fw += 3;
        }


        sz += (size_t)sprintf(geometry+sz, "x%*.*f", -fw, prec, dst_percent);
        assert(sz < geometry_l);
        sz = strcspn(geometry, " ");
    }

    if (sz < geometry_l)
    {
        memset(geometry+sz, 0x00, geometry_l-sz);
    }

}


/**
 * Create a composite of an image and an overlay (for blending, dissolving, etc.).
 *
 * No Ruby usage (internal function)
 *
 * @param image the original image
 * @param overlay the overlay
 * @param image_pct image percentage
 * @param overlay_pct overlay percentage
 * @param x_off the x offset
 * @param y_off the y offset
 * @param op the composite operator to use
 * @return a new image
 */
static VALUE
special_composite(Image *image, Image *overlay, double image_pct, double overlay_pct
                  , long x_off, long y_off, CompositeOperator op)
{
    Image *new_image;
    char geometry[20];

    blend_geometry(geometry, sizeof(geometry), image_pct, overlay_pct);
    (void) CloneString(&overlay->geometry, geometry);
#if defined(HAVE_SETIMAGEARTIFACT)
    (void) SetImageArtifact(overlay,"compose:args", geometry);
#endif

    new_image = rm_clone_image(image);
    (void) CompositeImage(new_image, op, overlay, x_off, y_off);

    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Corresponds to the composite -blend operation.
 *
 * Ruby usage:
 *   - @verbatim Image#blend(overlay, src_percent, dst_percent) @endverbatim
 *   - @verbatim Image#blend(overlay, src_percent, dst_percent, x_offset) @endverbatim
 *   - @verbatim Image#blend(overlay, src_percent, dst_percent, x_offset, y_offset) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, gravity) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, gravity, x_offset) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, gravity, x_offset, y_offset) @endverbatim
 *
 * Notes:
 *   - Default x_offset is 0
 *   - Default y_offset is 0
 *   - Percent can be a number or a string in the form "NN%"
 *   - The default value for dst_percent is 100%-src_percent
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_blend(int argc, VALUE *argv, VALUE self)
{
    volatile VALUE ovly;
    Image *image, *overlay;
    double src_percent, dst_percent;
    long x_offset = 0L, y_offset = 0L;

    image = rm_check_destroyed(self);

    if (argc < 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
    }

    ovly = rm_cur_image(argv[0]);
    overlay = rm_check_destroyed(ovly);

    if (argc > 3)
    {
        get_composite_offsets(argc-3, &argv[3], image, overlay, &x_offset, &y_offset);
        // There must be 3 arguments left
        argc = 3;
    }

    switch (argc)
    {
        case 3:
            dst_percent = rm_percentage(argv[2],1.0) * 100.0;
            src_percent = rm_percentage(argv[1],1.0) * 100.0;
            break;
        case 2:
            src_percent = rm_percentage(argv[1],1.0) * 100.0;
            dst_percent = FMAX(100.0 - src_percent, 0);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
            break;
    }

    return special_composite(image, overlay, src_percent, dst_percent
                             , x_offset, y_offset, BlendCompositeOp);

}



/**
 * Call BlueShiftImage.
 *
 * Ruby usage:
 *   - @verbatim Image#blue_shift @endverbatim
 *   - @verbatim Image#blue_shift(factor) @endverbatim
 *
 * Notes:
 *   - Default factor is 1.5
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_blue_shift(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_BLUESHIFTIMAGE)
    Image *image, *new_image;
    double factor = 1.5;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 1:
            factor = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }


    GetExceptionInfo(&exception);
    new_image = BlueShiftImage(image, factor, &exception);
    CHECK_EXCEPTION();
    DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
#else
    rm_not_implemented();
    return (VALUE)0;
    argc = argc;
    argv = argv;
    self = self;
#endif
}



DEF_ATTR_ACCESSOR(Image, blur, dbl)


/**
 * Call BlurImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#blur_channel @endverbatim
 *   - @verbatim Image#blur_channel(radius) @endverbatim
 *   - @verbatim Image#blur_channel(radius, sigma) @endverbatim
 *   - @verbatim Image#blur_channel(radius, sigma, channel) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_blur_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;
    ChannelType channels;
    double radius = 0.0, sigma = 1.0;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    // There can be 0, 1, or 2 remaining arguments.
    switch (argc)
    {
        case 2:
            sigma = NUM2DBL(argv[1]);
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);
    new_image = BlurImageChannel(image, channels, radius, sigma, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Blur the image.
 *
 * Ruby usage:
 *   - @verbatim Image#blur_image @endverbatim
 *   - @verbatim Image#blur_image(radius) @endverbatim
 *   - @verbatim Image#blur_image(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - The "blur" name is used for the attribute
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_blur_image(int argc, VALUE *argv, VALUE self)
{
    return effect_image(self, argc, argv, BlurImage);
}


/**
 * Surrounds the image with a border of the specified width, height, and named
 * color.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @param width the width of the border
 * @param height the height of the border
 * @param color the color of the border
 * @return self if bang, otherwise a new image
 * @see Image_border
 * @see Image_border_bang
 */
static VALUE
border(int bang, VALUE self, VALUE width, VALUE height, VALUE color)
{
    Image *image, *new_image;
    PixelPacket old_border;
    ExceptionInfo exception;
    RectangleInfo rect;

    Data_Get_Struct(self, Image, image);

    memset(&rect, 0, sizeof(rect));
    rect.width = NUM2UINT(width);
    rect.height = NUM2UINT(height);

    // Save current border color - we'll want to restore it afterwards.
    old_border = image->border_color;
    Color_to_PixelPacket(&image->border_color, color);

    GetExceptionInfo(&exception);
    new_image = BorderImage(image, &rect, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        new_image->border_color = old_border;
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    image->border_color = old_border;
    return rm_image_new(new_image);
}

/**
 * Surrounds the image with a border of the specified width, height, and named
 * color.
 *
 * Ruby usage:
 *   - @verbatim Image#border!(width, height, color) @endverbatim
 *
 * @param self this object
 * @param width the width of the border
 * @param height the height of the border
 * @param color the color of the border
 * @return self
 * @see border
 * @see Image_border
 */
VALUE
Image_border_bang(VALUE self, VALUE width, VALUE height, VALUE color)
{
    (void) rm_check_frozen(self);
    return border(True, self, width, height, color);
}


/**
 * Surrounds the image with a border of the specified width, height, and named
 * color.
 *
 * Ruby usage:
 *   - @verbatim Image#border(width, height, color) @endverbatim
 *
 * @param self this object
 * @param width the width of the border
 * @param height the height of the border
 * @param color the color of the border
 * @return a new image
 * @see border
 * @see Image_border_bang
 */
VALUE
Image_border(VALUE self, VALUE width, VALUE height, VALUE color)
{
    (void) rm_check_destroyed(self);
    return border(False, self, width, height, color);
}


/**
 * Return the name of the border color as a String.
 *
 * Ruby usage:
 *   - @verbatim Image#border_color @endverbatim
 *
 * @param self this object
 * @return the name of the border color
 */
VALUE
Image_border_color(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return rm_pixelpacket_to_color_name(image, &image->border_color);
}


/**
 * Set the the border color.
 *
 * Ruby usage:
 *   - @verbatim Image#border_color= @endverbatim
 *
 * @param self this object
 * @param color the color
 * @return self
 */
VALUE
Image_border_color_eq(VALUE self, VALUE color)
{
    Image *image = rm_check_frozen(self);
    Color_to_PixelPacket(&image->border_color, color);
    return self;
}


/**
 * returns the bounding box of an image canvas.
 *
 * Ruby usage:
 *   - @verbatim Image#bounding_box @endverbatim
 *
 * @param self this object
 * @return the bounding box
 */
VALUE
Image_bounding_box(VALUE self)
{
    Image *image;
    RectangleInfo box;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);
    box = GetImageBoundingBox(image, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return Import_RectangleInfo(&box);
}


/**
 * do a screen capture.
 *
 * Ruby usage:
 *   - @verbatim Image.capture @endverbatim
 *   - @verbatim Image.capture(silent) { optional parms } @endverbatim
 *   - @verbatim Image.capture(silent,frame) { optional parms } @endverbatim
 *   - @verbatim Image.capture(silent,frame,descend) { optional parms } @endverbatim
 *   - @verbatim Image.capture(silent,frame,descend,screen) { optional parms } @endverbatim
 *   - @verbatim Image.capture(silent,frame,descend,screen,borders) { optional parms } @endverbatim
 *
 * Notes:
 *   - Default silent is false
 *   - Default frame is false
 *   - Default descent is false
 *   - Default screen is false
 *   - Default borders if false
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_capture(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    ImageInfo *image_info;
    volatile VALUE info_obj;
    XImportInfo ximage_info;

    self = self;  // Suppress "never referenced" message from icc

    XGetImportInfo(&ximage_info);
    switch (argc)
    {
        case 5:
            ximage_info.borders = (MagickBooleanType)RTEST(argv[4]);
        case 4:
            ximage_info.screen  = (MagickBooleanType)RTEST(argv[3]);
        case 3:
            ximage_info.descend = (MagickBooleanType)RTEST(argv[2]);
        case 2:
            ximage_info.frame   = (MagickBooleanType)RTEST(argv[1]);
        case 1:
            ximage_info.silent  = (MagickBooleanType)RTEST(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 5)", argc);
            break;
    }

    // Get optional parms.
    // Set info->filename = "root", window ID number or window name,
    //  or nothing to do an interactive capture
    // Set info->server_name to the server name
    // Also info->colorspace, depth, dither, interlace, type
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, image_info);

    // If an error occurs, IM will call our error handler and we raise an exception.
    image = XImportImage(image_info, &ximage_info);
    rm_check_image_exception(image, DestroyOnError);
    rm_ensure_result(image);

    rm_set_user_artifact(image, image_info);

    return rm_image_new(image);
}


/**
 * parse geometry string, compute new image geometry.
 *
 * Ruby usage:
 *   - @verbatim Image#change_geometry(geometry_string) { |cols, rows, image| } @endverbatim
 *
 * @param self this object
 * @param geom_arg the geometry string
 * @return new image geometry
 */
VALUE
Image_change_geometry(VALUE self, VALUE geom_arg)
{
    Image *image;
    RectangleInfo rect;
    volatile VALUE geom_str;
    char *geometry;
    unsigned int flags;
    volatile VALUE ary;

    image = rm_check_destroyed(self);
    geom_str = rm_to_s(geom_arg);
    geometry = StringValuePtr(geom_str);

    memset(&rect, 0, sizeof(rect));

    SetGeometry(image, &rect);
    rm_check_image_exception(image, RetainOnError);
    flags = ParseMetaGeometry(geometry, &rect.x,&rect.y, &rect.width,&rect.height);
    if (flags == NoValue)
    {
        rb_raise(rb_eArgError, "invalid geometry string `%s'", geometry);
    }

    ary = rb_ary_new2(3);
    rb_ary_store(ary, 0, ULONG2NUM(rect.width));
    rb_ary_store(ary, 1, ULONG2NUM(rect.height));
    rb_ary_store(ary, 2, self);

    return rb_yield(ary);
}


/**
 * Return true if any pixel in the image has been altered since the image was
 * constituted.
 *
 * Ruby usage:
 *   - @verbatim Image#changed? @endverbatim
 *
 * @param self this object
 * @return true if altered, false otherwise
 */
VALUE
Image_changed_q(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    VALUE okay = IsTaintImage(image) ? Qtrue : Qfalse;
    rm_check_image_exception(image, RetainOnError);
    return okay;
}


/**
 * Extract a channel from the image.  A channel is a particular color component
 * of each pixel in the image.
 *
 * Ruby usage:
 *   - @verbatim Image#channel @endverbatim
 *
 * @param self this object
 * @param channel_arg the type of the channel to extract
 * @return the channel of the specified type
 */
VALUE
Image_channel(VALUE self, VALUE channel_arg)
{
    Image *image, *new_image;
    ChannelType channel;

    image = rm_check_destroyed(self);

    VALUE_TO_ENUM(channel_arg, channel, ChannelType);

    new_image = rm_clone_image(image);

    (void) SeparateImageChannel(new_image, channel);

    rm_check_image_exception(new_image, DestroyOnError);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * GetImageChannelDepth.
 *
 * Ruby usage:
 *   - @verbatim Image#channel_depth @endverbatim
 *   - @verbatim Image#channel_depth(channel_depth) @endverbatim
 *
 * Notes:
 *   - Default channel_depth is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the channel depth
 */
VALUE
Image_channel_depth(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    ChannelType channels;
    unsigned long channel_depth;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // Ensure all arguments consumed.
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);

    channel_depth = GetImageChannelDepth(image, channels, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return ULONG2NUM(channel_depth);
}


/**
 * Return an array [min, max] where 'min' and 'max' are the minimum and maximum
 * values of all channels.
 *
 * Ruby usage:
 *   - @verbatim Image#channel_extrema @endverbatim
 *   - @verbatim Image#channel_extrema(channel) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *   - GM's implementation is very different from ImageMagick. This method
 *     follows the IM API very closely and then shoehorn's the GM API to
 *     more-or-less fit. Note that IM allows you to specify more than one
 *     channel argument. GM does not.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return [min,max] of the channel
 */
VALUE
Image_channel_extrema(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    ChannelType channels;
    ExceptionInfo exception;
    unsigned long min, max;
    volatile VALUE ary;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    // Ensure all arguments consumed.
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);
    (void) GetImageChannelExtrema(image, channels, &min, &max, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    ary = rb_ary_new2(2);
    rb_ary_store(ary, 0, ULONG2NUM(min));
    rb_ary_store(ary, 1, ULONG2NUM(max));

    return ary;
}


/**
 * Return an array of the mean and standard deviation for the channel.
 *
 * Ruby usage:
 *   - @verbatim Image#channel_mean @endverbatim
 *   - @verbatim Image#channel_mean(channel) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return an array [mean, std. deviation]
 */
VALUE
Image_channel_mean(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    ChannelType channels;
    ExceptionInfo exception;
    double mean, stddev;
    volatile VALUE ary;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    // Ensure all arguments consumed.
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);
    (void) GetImageChannelMean(image, channels, &mean, &stddev, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    ary = rb_ary_new2(2);
    rb_ary_store(ary, 0, rb_float_new(mean));
    rb_ary_store(ary, 1, rb_float_new(stddev));

    return ary;
}


/**
 * Return a new image that is a copy of the input image with the edges
 * highlighted.
 *
 * Ruby usage:
 *   - @verbatim Image#charcoal @endverbatim
 *   - @verbatim Image#charcoal(radius) @endverbatim
 *   - @verbatim Image#charcoal(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_charcoal(int argc, VALUE *argv, VALUE self)
{
    return effect_image(self, argc, argv, CharcoalImage);
}


/**
 * If the target image has been destroyed, raise Magick::DestroyedImageError.
 *
 * Ruby usage:
 *   - @verbatim Image#check_destroyed @endverbatim
 *
 * @param self this object
 * @return nil
 * @throw Magick::DestroyedImageError
 */
VALUE
Image_check_destroyed(VALUE self)
{
    (void) rm_check_destroyed(self);
    return Qnil;
}


/**
 * Remove a region of an image and collapses the image to occupy the removed
 * portion.
 *
 * Ruby usage:
 *   - @verbatim Image#chop @endverbatim
 *
 * @param self this object
 * @param x x position of start of region
 * @param y y position of start of region
 * @param width width of region
 * @param height height of region
 * @return a new image
 */
VALUE
Image_chop(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
    (void) rm_check_destroyed(self);
    return xform_image(False, self, x, y, width, height, ChopImage);
}


/**
 * Return the red, green, blue, and white-point chromaticity values as a
 * Magick::ChromaticityInfo.
 *
 * Ruby usage:
 *   - @verbatim Image#chromaticity @endverbatim
 *
 * @param self this object
 * @return the chromaticity values
 */
VALUE
Image_chromaticity(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return ChromaticityInfo_new(&image->chromaticity);
}


/**
 * Set the red, green, blue, and white-point chromaticity values from a
 * Magick::ChromaticityInfo.
 *
 * Ruby usage:
 *   - @verbatim Image#chromaticity= @endverbatim
 *
 * @param self this object
 * @param chroma the chromaticity
 * @return self
 */
VALUE
Image_chromaticity_eq(VALUE self, VALUE chroma)
{
    Image *image = rm_check_frozen(self);
    Export_ChromaticityInfo(&image->chromaticity, chroma);
    return self;
}


/**
 * Copy an image, along with its frozen and tainted state.
 *
 * Ruby usage:
 *   - @verbatim Image#clone @endverbatim
 *
 * @param self this object
 * @return a clone of this object
 */
VALUE
Image_clone(VALUE self)
{
    volatile VALUE clone;

    clone = Image_dup(self);
    if (OBJ_FROZEN(self))
    {
        OBJ_FREEZE(clone);
    }

    return clone;
}


/**
 * Equivalent to -clut option.
 *
 * Ruby usage:
 *   - @verbatim Image#clut_channel @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */
VALUE
Image_clut_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *clut;
    ChannelType channels;
    MagickBooleanType okay;

    image = rm_check_frozen(self);

    // check_destroyed before confirming the arguments
    if (argc >= 1)
    {
        (void) rm_check_destroyed(argv[0]);
        channels = extract_channels(&argc, argv);
        if (argc != 1)
        {
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or more)", argc);
        }
    }
    else
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or more)", argc);
    }

    Data_Get_Struct(argv[0], Image, clut);

    okay = ClutImageChannel(image, channels, clut);
    rm_check_image_exception(image, RetainOnError);
    rm_check_image_exception(clut, RetainOnError);
    if (!okay)
    {
        rb_raise(rb_eRuntimeError, "ClutImageChannel failed.");
    }

    return self;
}


/**
 * Call GetImageHistogram.
 *
 * Ruby usage:
 *   - @verbatim Image_color_histogram(VALUE self); @endverbatim
 *
 * Notes:
 *   - returns hash @verbatim {aPixel=>count} @endverbatim
 *
 * @param self this object
 * @return a histogram
 */
VALUE
Image_color_histogram(VALUE self)
{
    Image *image, *dc_copy = NULL;
    volatile VALUE hash, pixel;
    unsigned long x, colors;
    ColorPacket *histogram;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    // If image not DirectClass make a DirectClass copy.
    if (image->storage_class != DirectClass)
    {
        dc_copy = rm_clone_image(image);
        (void) SyncImage(dc_copy);
        magick_free(dc_copy->colormap);
        dc_copy->colormap = NULL;
        dc_copy->storage_class = DirectClass;
        image = dc_copy;
    }

    GetExceptionInfo(&exception);
    histogram = GetImageHistogram(image, &colors, &exception);

    if (histogram == NULL)
    {
        if (dc_copy)
        {
            (void) DestroyImage(dc_copy);
        }
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    if (exception.severity != UndefinedException)
    {
        (void) RelinquishMagickMemory(histogram);
        rm_check_exception(&exception, dc_copy, DestroyOnError);
    }

    (void) DestroyExceptionInfo(&exception);

    hash = rb_hash_new();
    for (x = 0; x < colors; x++)
    {
        pixel = Pixel_from_PixelPacket(&histogram[x].pixel);
        (void) rb_hash_aset(hash, pixel, ULONG2NUM((unsigned long)histogram[x].count));
    }

    /*
        Christy evidently didn't agree with Bob's memory management.
    */
    (void) RelinquishMagickMemory(histogram);

    if (dc_copy)
    {
        // Do not trace destruction
        (void) DestroyImage(dc_copy);
    }

    return hash;
}


/**
 * Store all the profiles in the profile in the target image. Called from
 * Image_color_profile_eq and Image_iptc_profile_eq.
 *
 * No Ruby usage (internal function)
 * 
 * @param self this object
 * @param name profile name
 * @param profile an IPTC or ICC profile
 * @return self
 */
static VALUE
set_profile(VALUE self, const char *name, VALUE profile)
{
    Image *image, *profile_image;
    ImageInfo *info;
    const MagickInfo *m;
    ExceptionInfo exception;
    char *profile_name;
    char *profile_blob;
    long profile_length;
    const StringInfo *profile_data;

    image = rm_check_frozen(self);

    profile_blob = rm_str2cstr(profile, &profile_length);

    GetExceptionInfo(&exception);
    m = GetMagickInfo(name, &exception);
    CHECK_EXCEPTION()

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    strncpy(info->magick, m->name, MaxTextExtent);
    info->magick[MaxTextExtent-1] = '\0';

    profile_image = BlobToImage(info, profile_blob, (size_t)profile_length, &exception);
    (void) DestroyImageInfo(info);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    ResetImageProfileIterator(profile_image);
    profile_name = GetNextImageProfile(profile_image);
    while (profile_name)
    {
        if (rm_strcasecmp(profile_name, name) == 0)
        {
            profile_data = GetImageProfile(profile_image, profile_name);
            if (profile)
            {
                (void)ProfileImage(image, profile_name, profile_data->datum
                                   , (unsigned long)profile_data->length
                                   , (MagickBooleanType)MagickFalse);
                if (image->exception.severity >= ErrorException)
                {
                    break;
                }
            }
        }
        profile_name = GetNextImageProfile(profile_image);
    }

    (void) DestroyImage(profile_image);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Return the ICC color profile as a String.
 *
 * Ruby usage:
 *   - @verbatim Image#color_profile @endverbatim
 *
 * Notes:
 *   - If there is no profile, returns ""
 *   - This method has no real use but is retained for compatibility with
 *     earlier releases of RMagick, where it had no real use either.
 *
 * @param self this object
 * @return the ICC color profile
 */
VALUE
Image_color_profile(VALUE self)
{
    Image *image;
    const StringInfo *profile;

    image = rm_check_destroyed(self);
    profile = GetImageProfile(image, "icc");
    if (!profile)
    {
        return Qnil;
    }

    return rb_str_new((char *)profile->datum, (long)profile->length);

}


/**
 * Set the ICC color profile.
 *
 * Ruby usage:
 *   - @verbatim Image#color_profile=(String) @endverbatim
 *
 * Notes:
 *   - Pass nil to remove any existing profile.
 *   - Removes any existing profile before adding the new one.
 *
 * @param self this object
 * @param profile the profile to set, as a Ruby string
 * @return self
 */
VALUE
Image_color_profile_eq(VALUE self, VALUE profile)
{
    (void) Image_delete_profile(self, rb_str_new2("ICC"));
    if (profile != Qnil)
    {
        (void) set_profile(self, "ICC", profile);
    }
    return self;
}


/**
 * Change the color value of any pixel that matches target_color and is an
 * immediate neighbor.
 *
 * Ruby usage:
 *   - @verbatim Image#color_flood_fill(target_color, fill_color, x, y, method) @endverbatim
 *
 * Notes:
 *   - Use fuzz= to specify the tolerance amount
 *   - Accepts either the FloodfillMethod or the FillToBorderMethod
 *
 * @param self this object
 * @param target_color the color
 * @param fill_color the color to fill
 * @param xv the x position
 * @param yv the y position
 * @param method the method to call
 * @return a new image
 * @see Image_opaque
 */
VALUE
Image_color_flood_fill( VALUE self, VALUE target_color, VALUE fill_color
                        , VALUE xv, VALUE yv, VALUE method)
{
    Image *image, *new_image;
    PixelPacket target;
    DrawInfo *draw_info;
    PixelPacket fill;
    long x, y;
    int fill_method;

    image = rm_check_destroyed(self);

    // The target and fill args can be either a color name or
    // a Magick::Pixel.
    Color_to_PixelPacket(&target, target_color);
    Color_to_PixelPacket(&fill, fill_color);

    x = NUM2LONG(xv);
    y = NUM2LONG(yv);
    if ((unsigned long)x > image->columns || (unsigned long)y > image->rows)
    {
        rb_raise(rb_eArgError, "target out of range. %lux%lu given, image is %lux%lu"
                 , x, y, image->columns, image->rows);
    }

    VALUE_TO_ENUM(method, fill_method, PaintMethod);
    if (!(fill_method == FloodfillMethod || fill_method == FillToBorderMethod))
    {
        rb_raise(rb_eArgError, "paint method must be FloodfillMethod or "
                 "FillToBorderMethod (%d given)", fill_method);
    }

    draw_info = CloneDrawInfo(NULL, NULL);
    if (!draw_info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    draw_info->fill = fill;

    new_image = rm_clone_image(image);

#if defined(HAVE_FLOODFILLPAINTIMAGE)
    {
        MagickPixelPacket target_mpp;
        MagickBooleanType invert;

        GetMagickPixelPacket(new_image, &target_mpp);
        if (fill_method == FillToBorderMethod)
        {
            invert = MagickTrue;
            target_mpp.red   = (MagickRealType) image->border_color.red;
            target_mpp.green = (MagickRealType) image->border_color.green;
            target_mpp.blue  = (MagickRealType) image->border_color.blue;
        }
        else
        {
            invert = MagickFalse;
            target_mpp.red   = (MagickRealType) target.red;
            target_mpp.green = (MagickRealType) target.green;
            target_mpp.blue  = (MagickRealType) target.blue;
        }

        (void) FloodfillPaintImage(new_image, DefaultChannels, draw_info, &target_mpp, x, y, invert);
    }
#else
    (void) ColorFloodfillImage(new_image, draw_info, target, x, y, (PaintMethod)fill_method);
#endif
    // No need to check for error

    (void) DestroyDrawInfo(draw_info);
    return rm_image_new(new_image);
}


/**
 * Blend the fill color specified by "target" with each pixel in the image.
 * Specify the percentage blend for each r, g, b component.
 *
 * Ruby usage:
 *   - @verbatim Image#colorize(r, g, b, target) @endverbatim
 *   - @verbatim Image#colorize(r, g, b, matte, target) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_colorize(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double red, green, blue, matte;
    char opacity[50];
    PixelPacket target;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    if (argc == 4)
    {
        red   = floor(100*NUM2DBL(argv[0])+0.5);
        green = floor(100*NUM2DBL(argv[1])+0.5);
        blue  = floor(100*NUM2DBL(argv[2])+0.5);
        Color_to_PixelPacket(&target, argv[3]);
        sprintf(opacity, "%f/%f/%f", red, green, blue);
    }
    else if (argc == 5)
    {
        red   = floor(100*NUM2DBL(argv[0])+0.5);
        green = floor(100*NUM2DBL(argv[1])+0.5);
        blue  = floor(100*NUM2DBL(argv[2])+0.5);
        matte = floor(100*NUM2DBL(argv[3])+0.5);
        Color_to_PixelPacket(&target, argv[4]);
        sprintf(opacity, "%f/%f/%f/%f", red, green, blue, matte);
    }
    else
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 4 or 5)", argc);
    }

    GetExceptionInfo(&exception);
    new_image = ColorizeImage(image, opacity, target, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Return the color in the colormap at the specified index. If a new color is
 * specified, replaces the color at the index with the new color.
 *
 * Ruby usage:
 *   - @verbatim Image#colormap(index) @endverbatim
 *   - @verbatim Image#colormap(index, new-color) @endverbatim
 *
 * Notes:
 *   - The "new-color" argument can be either a color name or a Magick::Pixel.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the name of the color
 */
VALUE
Image_colormap(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    unsigned long idx;
    PixelPacket color, new_color;

    image = rm_check_destroyed(self);

    // We can handle either 1 or 2 arguments. Nothing else.
    if (argc == 0 || argc > 2)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
    }

    idx = NUM2ULONG(argv[0]);
    if (idx > QuantumRange)
    {
        rb_raise(rb_eIndexError, "index out of range");
    }

    // If this is a simple "get" operation, ensure the image has a colormap.
    if (argc == 1)
    {
        if (!image->colormap)
        {
            rb_raise(rb_eIndexError, "image does not contain a colormap");
        }
        // Validate the index

        if (idx > image->colors-1)
        {
            rb_raise(rb_eIndexError, "index out of range");
        }
        return rm_pixelpacket_to_color_name(image, &image->colormap[idx]);
    }

    // This is a "set" operation. Things are different.

    rb_check_frozen(self);

    // Replace with new color? The arg can be either a color name or
    // a Magick::Pixel.
    Color_to_PixelPacket(&new_color, argv[1]);

    // Handle no colormap or current colormap too small.
    if (!image->colormap || idx > image->colors-1)
    {
        PixelPacket black;
        unsigned long i;

        memset(&black, 0, sizeof(black));

        if (!image->colormap)
        {
            image->colormap = (PixelPacket *)magick_safe_malloc((idx+1), sizeof(PixelPacket));
            image->colors = 0;
        }
        else
        {
            image->colormap = (PixelPacket *)magick_safe_realloc(image->colormap, (idx+1), sizeof(PixelPacket));
        }

        for (i = image->colors; i < idx; i++)
        {
            image->colormap[i] = black;
        }
        image->colors = idx+1;
    }

    // Save the current color so we can return it. Set the new color.
    color = image->colormap[idx];
    image->colormap[idx] = new_color;

    return rm_pixelpacket_to_color_name(image, &color);
}

/**
 * Get image colors.
 *
 * Ruby usage:
 *   - @verbatim Image#colors @endverbatim
 *
 * @param self this object
 * @return the colors
 */
DEF_ATTR_READER(Image, colors, ulong)

/**
 * Return the Image pixel interpretation. If the colorspace is RGB the pixels
 * are red, green, blue. If matte is true, then red, green, blue, and index. If
 * it is CMYK, the pixels are cyan, yellow, magenta, black. Otherwise the
 * colorspace is ignored.
 *
 * Ruby usage:
 *   - @verbatim Image#colorspace @endverbatim
 *
 * @param self this object
 * @return the colorspace
 */
VALUE
Image_colorspace(VALUE self)
{
    Image *image;

    image = rm_check_destroyed(self);
    return ColorspaceType_new(image->colorspace);
}


/**
 * Set the image's colorspace.
 *
 * Ruby usage:
 *   - @verbatim Image#colorspace=Magick::ColorspaceType @endverbatim
 *
 * @param self this object
 * @param colorspace the colorspace
 * @return self
 * @see Magick::colorSpace in Magick++'s Magick::colorSpace
 */
VALUE
Image_colorspace_eq(VALUE self, VALUE colorspace)
{
    Image *image;
    ColorspaceType new_cs;

    image = rm_check_frozen(self);
    VALUE_TO_ENUM(colorspace, new_cs, ColorspaceType);
    (void) SetImageColorspace(image, new_cs);

    return self;
}


/**
 * Get image columns.
 *
 * Ruby usage:
 *   - @verbatim Image#columns @endverbatim
 *
 * @param self this object
 * @return the columns
 */
DEF_ATTR_READER(Image, columns, int)


/**
 * Combine the Red channel of the first image with the Green channel of the
 * 2nd image and the Blue channel of the 3rd image. Any of the image arguments
 * may be omitted or replaced by nil.
 *
 * Ruby usage:
 *   - @verbatim new_image = Image.combine(red) @endverbatim
 *   - @verbatim new_image = Image.combine(red, green) @endverbatim
 *   - @verbatim new_image = Image.combine(red, green, blue) @endverbatim
 *   - @verbatim new_image = Image.combine(red, green, blue, opacity) @endverbatim
 *
 * Notes:
 *   - Calls CombineImages.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE Image_combine(int argc, VALUE *argv, VALUE self)
{
    ChannelType channel = 0;
    Image *image, *images = NULL, *new_image;
    ExceptionInfo exception;

    self = self;        // defeat "unreferenced argument" message

    switch (argc)
    {
        case 4:
            if (argv[3] != Qnil)
            {
                channel |= OpacityChannel;
                image = rm_check_destroyed(argv[3]);
                AppendImageToList(&images, image);
            }
        case 3:
            if (argv[2] != Qnil)
            {
                channel |= BlueChannel;
                image = rm_check_destroyed(argv[2]);
                AppendImageToList(&images, image);
            }
        case 2:
            if (argv[1] != Qnil)
            {
                channel |= GreenChannel;
                image = rm_check_destroyed(argv[1]);
                AppendImageToList(&images, image);
            }
        case 1:
            if (argv[0] != Qnil)
            {
                channel |= RedChannel;
                image = rm_check_destroyed(argv[0]);
                AppendImageToList(&images, image);
            }
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (1 to 4 expected, got %d)", argc);
    }

    if (channel == 0)
    {
        rb_raise(rb_eArgError, "no images to combine");
    }

    GetExceptionInfo(&exception);
    ReverseImageList(&images);
    new_image = CombineImages(images, channel, &exception);
    rm_check_exception(&exception, images, RetainOnError);
    rm_split(images);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);

}


/**
 * Compare one or more channels in two images and returns the specified
 * distortion metric and a comparison image.
 *
 * Ruby usage:
 *   - @verbatim Image#compare_channel(ref_image, metric) { optional arguments } @endverbatim
 *   - @verbatim Image#compare_channel(ref_image, metric, channel) { optional arguments } @endverbatim
 *   - @verbatim Image#compare_channel(ref_image, metric, channel, ...) { optional arguments } @endverbatim
 *
 * Notes:
 *   - If no channels are specified, the default is AllChannels. That case is
 *     the equivalent of the CompareImages method in ImageMagick.
 *   - Originally this method was called channel_compare, but that doesn't match
 *     the general naming convention that methods which accept multiple optional
 *     ChannelType arguments have names that end in _channel. So I renamed the
 *     method to compare_channel but kept channel_compare as an alias.
 *   - The optional arguments are specified thusly:
 *     - self.highlight_color color
 *     - self.lowlight-color color
 *     where color is either a color name or a Pixel.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return an array of [difference_image,distortion]
 */
VALUE
Image_compare_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *r_image, *difference_image;
    double distortion;
    volatile VALUE ary, ref;
    MetricType metric_type;
    ChannelType channels;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    if (argc > 2)
    {
        raise_ChannelType_error(argv[argc-1]);
    }
    if (argc != 2)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or more)", argc);
    }

    rm_get_optional_arguments(self);

    ref = rm_cur_image(argv[0]);
    r_image = rm_check_destroyed(ref);

    VALUE_TO_ENUM(argv[1], metric_type, MetricType);

    GetExceptionInfo(&exception);
    difference_image = CompareImageChannels(image
                                            , r_image
                                            , channels
                                            , metric_type
                                            , &distortion
                                            , &exception);
    rm_check_exception(&exception, difference_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(difference_image);

    ary = rb_ary_new2(2);
    rb_ary_store(ary, 0, rm_image_new(difference_image));
    rb_ary_store(ary, 1, rb_float_new(distortion));

    return ary;
}


/**
 * Return the composite operator attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#compose @endverbatim
 *
 * @param self this object
 * @return the composite operator
 */
VALUE
Image_compose(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return CompositeOperator_new(image->compose);
}


/**
 * Set the composite operator attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#compose=composite_op @endverbatim
 *
 * @param self this object
 * @param compose_arg the composite operator
 * @return self
 */
VALUE
Image_compose_eq(VALUE self, VALUE compose_arg)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(compose_arg, image->compose, CompositeOperator);
    return self;
}

/**
 * Call CompositeImage.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - The other image can be either an Image or an Image.
 *   - The use of the GravityType to position the composited image is based on
 *     Magick++.
 *   - The `gravity' argument has the same effect as the -gravity option does in
 *     the `composite' utility.
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param channels 
 * @return self if bang, otherwise new composited image
 * @see Image_composite
 * @see Image_composite_bang
 */
static VALUE
composite(int bang, int argc, VALUE *argv, VALUE self, ChannelType channels)
{
    Image *image, *new_image;
    Image *comp_image;
    CompositeOperator operator = UndefinedCompositeOp;
    GravityType gravity;
    volatile VALUE comp;
    signed long x_offset = 0;
    signed long y_offset = 0;

    image = rm_check_destroyed(self);

    if (bang)
    {
        rb_check_frozen(self);
    }
    if (argc < 3 || argc > 5)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 3, 4, or 5)", argc);
    }


    comp = rm_cur_image(argv[0]);
    comp_image = rm_check_destroyed(comp);

    switch (argc)
    {
        case 3:                 // argv[1] is gravity, argv[2] is composite_op
            VALUE_TO_ENUM(argv[1], gravity, GravityType);
            VALUE_TO_ENUM(argv[2], operator, CompositeOperator);

            // convert gravity to x, y offsets
            switch (gravity)
            {
                case ForgetGravity:
                case NorthWestGravity:
                    x_offset = 0;
                    y_offset = 0;
                    break;
                case NorthGravity:
                    x_offset = ((long)(image->columns) - (long)(comp_image->columns)) / 2;
                    y_offset = 0;
                    break;
                case NorthEastGravity:
                    x_offset = (long)(image->columns) - (long)(comp_image->columns);
                    y_offset = 0;
                    break;
                case WestGravity:
                    x_offset = 0;
                    y_offset = ((long)(image->rows) - (long)(comp_image->rows)) / 2;
                    break;
                case StaticGravity:
                case CenterGravity:
                default:
                    x_offset = ((long)(image->columns) - (long)(comp_image->columns)) / 2;
                    y_offset = ((long)(image->rows) - (long)(comp_image->rows)) / 2;
                    break;
                case EastGravity:
                    x_offset = (long)(image->columns) - (long)(comp_image->columns);
                    y_offset = ((long)(image->rows) - (long)(comp_image->rows)) / 2;
                    break;
                case SouthWestGravity:
                    x_offset = 0;
                    y_offset = (long)(image->rows) - (long)(comp_image->rows);
                    break;
                case SouthGravity:
                    x_offset = ((long)(image->columns) - (long)(comp_image->columns)) / 2;
                    y_offset = (long)(image->rows) - (long)(comp_image->rows);
                    break;
                case SouthEastGravity:
                    x_offset = (long)(image->columns) - (long)(comp_image->columns);
                    y_offset = (long)(image->rows) - (long)(comp_image->rows);
                    break;
            }
            break;

        case 4:                 // argv[1], argv[2] is x_off, y_off,
            // argv[3] is composite_op
            x_offset = NUM2LONG(argv[1]);
            y_offset = NUM2LONG(argv[2]);
            VALUE_TO_ENUM(argv[3], operator, CompositeOperator);
            break;

        case 5:
            VALUE_TO_ENUM(argv[1], gravity, GravityType);
            x_offset = NUM2LONG(argv[2]);
            y_offset = NUM2LONG(argv[3]);
            VALUE_TO_ENUM(argv[4], operator, CompositeOperator);

            switch (gravity)
            {
                case NorthEastGravity:
                case EastGravity:
                case SouthEastGravity:
                    x_offset = ((long)(image->columns) - (long)(comp_image->columns)) - x_offset;
                    break;
                case NorthGravity:
                case SouthGravity:
                case CenterGravity:
                case StaticGravity:
                    x_offset += (long)(image->columns/2) - (long)(comp_image->columns/2);
                    break;
                default:
                    break;
            }
            switch (gravity)
            {
                case SouthWestGravity:
                case SouthGravity:
                case SouthEastGravity:
                    y_offset = ((long)(image->rows) - (long)(comp_image->rows)) - y_offset;
                    break;
                case EastGravity:
                case WestGravity:
                case CenterGravity:
                case StaticGravity:
                    y_offset += (long)(image->rows/2) - (long)(comp_image->rows/2);
                    break;
                case NorthEastGravity:
                case NorthGravity:
                default:
                    break;
            }
            break;

    }

    if (bang)
    {
        (void) CompositeImageChannel(image, channels, operator, comp_image, x_offset, y_offset);
        rm_check_image_exception(image, RetainOnError);

        return self;
    }
    else
    {
        new_image = rm_clone_image(image);

        (void) CompositeImageChannel(new_image, channels, operator, comp_image, x_offset, y_offset);
        rm_check_image_exception(new_image, DestroyOnError);

        return rm_image_new(new_image);
    }
}


/**
 * Call CompositeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#composite!(image, x_off, y_off, composite_op) @endverbatim
 *   - @verbatim Image#composite!(image, gravity, composite_op) @endverbatim
 *   - @verbatim Image#composite!(image, gravity, x_off, y_off, composite_op) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see composite
 * @see Image_composite
 */
VALUE
Image_composite_bang(int argc, VALUE *argv, VALUE self)
{
    return composite(True, argc, argv, self, DefaultChannels);
}


/**
 * Call CompositeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#composite(image, x_off, y_off, composite_op) @endverbatim
 *   - @verbatim Image#composite(image, gravity, composite_op) @endverbatim
 *   - @verbatim Image#composite(image, gravity, x_off, y_off, composite_op) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see composite
 * @see Image_composite_bang
 */
VALUE
Image_composite(int argc, VALUE *argv, VALUE self)
{
    return composite(False, argc, argv, self, DefaultChannels);
}


/**
 * Composite the source over the destination image as dictated by the affine
 * transform.
 *
 * Ruby usage:
 *   - @verbatim Image#composite_affine(composite, affine_matrix) @endverbatim
 *
 * @param self this object
 * @param source the source image
 * @param affine_matrix affine transform matrix
 * @return a new image
 */
VALUE
Image_composite_affine(VALUE self, VALUE source, VALUE affine_matrix)
{
    Image *image, *composite_image, *new_image;
    AffineMatrix affine;

    image = rm_check_destroyed(self);
    composite_image = rm_check_destroyed(source);
    new_image = rm_clone_image(image);

    Export_AffineMatrix(&affine, affine_matrix);
    (void) DrawAffineImage(new_image, composite_image, &affine);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call CompositeImageChannel.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see Image_composite_channel
 * @see Image_composite_channel_bang
 */
static VALUE
composite_channel(int bang, int argc, VALUE *argv, VALUE self)
{
    ChannelType channels;

    // Check destroyed before validating the arguments
    (void) rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There must be 3, 4, or 5 remaining arguments.
    if (argc < 3)
    {
        rb_raise(rb_eArgError, "composite operator not specified");
    }
    else if (argc > 5)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    return composite(bang, argc, argv, self, channels);
}


/**
 * Call CompositeImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#composite_channel(src_image, geometry, composite_operator) @endverbatim
 *   - @verbatim Image#composite_channel(src_image, geometry, composite_operator, channel) @endverbatim
 *   - @verbatim Image#composite_channel(src_image, geometry, composite_operator, channel, ...) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see composite_channel
 * @see Image_composite_channel_bang
 */
VALUE
Image_composite_channel(int argc, VALUE *argv, VALUE self)
{
    return composite_channel(False, argc, argv, self);
}


/**
 * Call CompositeImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#composite_channel!(src_image, geometry, composite_operator) @endverbatim
 *   - @verbatim Image#composite_channel!(src_image, geometry, composite_operator, channel) @endverbatim
 *   - @verbatim Image#composite_channel!(src_image, geometry, composite_operator, channel, ...) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see composite_channel
 * @see Image_composite_channel
 */
VALUE
Image_composite_channel_bang(int argc, VALUE *argv, VALUE self)
{
    return composite_channel(True, argc, argv, self);
}


/**
 * Composite using MathematicsCompositeOp.
 *
 * Ruby usage:
 *   - @verbatim img.composite_mathematics(comp_img, A, B, C, D, gravity) @endverbatim
 *   - @verbatim img.composite_mathematics(comp_img, A, B, C, D, x_off, y_off) @endverbatim
 *   - @verbatim img.composite_mathematics(comp_img, A, B, C, D, gravity, x_off, y_off) @endverbatim
 *
 * Notes:
 *   - Default x_off is 0
 *   - Default y_off is 0
 *   - New in ImageMagick 6.5.4-3.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_composite_mathematics(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_ENUM_MATHEMATICSCOMPOSITEOP)
    Image *composite_image;
    VALUE args[5];
    signed long x_off = 0L;
    signed long y_off = 0L;
    GravityType gravity = NorthWestGravity;
    char compose_args[200];

    rm_check_destroyed(self);
    if (argc > 0)
    {
        composite_image = rm_check_destroyed(rm_cur_image(argv[0]));
    }

    switch (argc)
    {
        case 8:
            VALUE_TO_ENUM(argv[5], gravity, GravityType);
            x_off = NUM2LONG(argv[6]);
            y_off = NUM2LONG(argv[7]);
            break;
        case 7:
            x_off = NUM2LONG(argv[5]);
            y_off = NUM2LONG(argv[6]);
            break;
        case 6:
            VALUE_TO_ENUM(argv[5], gravity, GravityType);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (got %d, expected 6 to 8)", argc);
            break;
    }


    (void) sprintf(compose_args, "%-.16g,%-.16g,%-.16g,%-.16g", NUM2DBL(argv[1]), NUM2DBL(argv[2]), NUM2DBL(argv[3]), NUM2DBL(argv[4]));
    SetImageArtifact(composite_image,"compose:args", compose_args);

    // Call composite(False, gravity, x_off, y_off, MathematicsCompositeOp, DefaultChannels)
    args[0] = argv[0];
    args[1] = GravityType_new(gravity);
    args[2] = LONG2FIX(x_off);
    args[3] = LONG2FIX(y_off);
    args[4] = CompositeOperator_new(MathematicsCompositeOp);

    return composite(False, 5, args, self, DefaultChannels);

#else
    rm_not_implemented();
    argc = argc;
    argv = argv;
    self = self;
    return (VALUE)0;
#endif
}


/**
 * Emulate the -tile option to the composite command.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Default composite_op is Magick::OverCompositeOp
 *   - Default channel is AllChannels
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see Image_composite_tiled
 * @see Image_composite_tiled_bang
 * @see wand/composite.c in ImageMagick (6.2.4)
 */
static VALUE
composite_tiled(int bang, int argc, VALUE *argv, VALUE self)
{
    Image *image;
    Image *comp_image;
    CompositeOperator operator = OverCompositeOp;
    long x, y;
    unsigned long columns;
    ChannelType channels;
    MagickStatusType status;

    // Ensure image and composite_image aren't destroyed.
    if (bang)
    {
        image = rm_check_frozen(self);
    }
    else
    {
        image = rm_check_destroyed(self);
    }

    if (argc > 0)
    {
        comp_image = rm_check_destroyed(rm_cur_image(argv[0]));
    }

    channels = extract_channels(&argc, argv);

    switch (argc)
    {
        case 2:
            VALUE_TO_ENUM(argv[1], operator, CompositeOperator);
        case 1:
            break;
        case 0:
            rb_raise(rb_eArgError, "wrong number of arguments (0 for 1 or more)");
            break;
        default:
            raise_ChannelType_error(argv[argc-1]);
            break;
    }

    if (!bang)
    {
        image = rm_clone_image(image);
    }

#if defined(HAVE_SETIMAGEARTIFACT)
    (void) SetImageArtifact(comp_image,"modify-outside-overlay", "false");
#else
    (void) SetImageAttribute(comp_image, "[modify-outside-overlay]", "false");
#endif

    status = MagickTrue;
    columns = comp_image->columns;

    // Tile
    for (y = 0; y < (long) image->rows; y += comp_image->rows)
    {
        for (x = 0; status == MagickTrue && x < (long) image->columns; x += columns)
        {
            status = CompositeImageChannel(image, channels, operator, comp_image, x, y);
            rm_check_image_exception(image, bang ? RetainOnError: DestroyOnError);
        }
    }

    return bang ? self : rm_image_new(image);
}


/**
 * Emulate the -tile option to the composite command.
 *
 * Ruby usage:
 *   - @verbatim Image#composite_tiled(src) @endverbatim
 *   - @verbatim Image#composite_tiled(src, composite_op) @endverbatim
 *   - @verbatim Image#composite_tiled(src, composite_op, channel) @endverbatim
 *   - @verbatim Image#composite_tiled(src, composite_op, channel, ...) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see composite_tiled
 * @see Image_composite_tiled_bang
 */
VALUE
Image_composite_tiled(int argc, VALUE *argv, VALUE self)
{
    return composite_tiled(False, argc, argv, self);
}


/**
 * Emulate the -tile option to the composite command.
 *
 * Ruby usage:
 *   - @verbatim Image#composite_tiled!(src) @endverbatim
 *   - @verbatim Image#composite_tiled!(src, composite_op) @endverbatim
 *   - @verbatim Image#composite_tiled!(src, composite_op, channel) @endverbatim
 *   - @verbatim Image#composite_tiled!(src, composite_op, channel, ...) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see composite_tiled
 * @see Image_composite_tiled_bang
 */
VALUE
Image_composite_tiled_bang(int argc, VALUE *argv, VALUE self)
{
    return composite_tiled(True, argc, argv, self);
}


/**
 * Get/set the compression attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#compression @endverbatim
 *
 * @param self this object
 * @return the compression
 */
VALUE
Image_compression(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return CompressionType_new(image->compression);
}

/**
 * Get/set the compression attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#compression= @endverbatim
 *
 * @param self this object
 * @param compression the compression
 * @return self
 */
VALUE
Image_compression_eq(VALUE self, VALUE compression)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(compression, image->compression, CompressionType);
    return self;
}

/**
 * call CompressImageColormap.
 *
 * Ruby usage:
 *   - @verbatim Image#compress_colormap! @endverbatim
 *
 * Notes:
 *   - API was CompressColormap until 5.4.9
 *
 * @param self this object
 * @return self
 */
VALUE
Image_compress_colormap_bang(VALUE self)
{
    Image *image = rm_check_frozen(self);
    (void) CompressImageColormap(image);
    rm_check_image_exception(image, RetainOnError);

    return self;
}

/**
 * Creates an Image from the supplied pixel data. The pixel data must be in
 * scanline order, top-to-bottom. The pixel data is an array of either all Fixed
 * or all Float elements. If Fixed, the elements must be in the range
 * [0..QuantumRange]. If Float, the elements must be normalized [0..1]. The
 * "map" argument reflects the expected ordering of the pixel array. It can be
 * any combination or order of R = red, G = green, B = blue, A = alpha,
 * C = cyan, Y = yellow, M = magenta, K = black, or I = intensity (for
 * grayscale).
 *
 * The pixel array must have width X height X strlen(map) elements.
 *
 * Ruby usage:
 *   - @verbatim Image.constitute(width, height, map, pixels) @endverbatim
 *
 * @param class the Ruby class for an Image (unused)
 * @param width_arg the width of the array
 * @param height_arg the height of the array
 * @param map_arg the map (expected ordering of the pixel array)
 * @param pixels_arg the pixel array
 * @return a new image
 * @throw ArgumentError
 * @throw TypeError
 */
VALUE
Image_constitute(VALUE class, VALUE width_arg, VALUE height_arg
                 , VALUE map_arg, VALUE pixels_arg)
{
    Image *image;
    ExceptionInfo exception;
    volatile VALUE pixel, pixel0;
    unsigned long width, height;
    long x, npixels;
    char *map;
    long map_l;
    volatile union
    {
        double *f;
        Quantum *i;
        void *v;
    } pixels;
    volatile VALUE pixel_class;
    StorageType stg_type;

    class = class;  // Suppress "never referenced" message from icc

            // rb_Array converts objects that are not Arrays to Arrays if possible,
            // and raises TypeError if it can't.
            pixels_arg = rb_Array(pixels_arg);

    width = NUM2ULONG(width_arg);
    height = NUM2ULONG(height_arg);

    if (width == 0 || height == 0)
    {
        rb_raise(rb_eArgError, "width and height must be non-zero");
    }

    map = rm_str2cstr(map_arg, &map_l);

    npixels = (long)(width * height * map_l);
    if (RARRAY_LEN(pixels_arg) != npixels)
    {
        rb_raise(rb_eArgError, "wrong number of array elements (%ld for %ld)"
                 , RARRAY_LEN(pixels_arg), npixels);
    }

    // Inspect the first element in the pixels array to determine the expected
    // type of all the elements. Allocate the pixel buffer.
    pixel0 = rb_ary_entry(pixels_arg, 0);
    if (rb_obj_is_kind_of(pixel0, rb_cFloat) == Qtrue)
    {
        pixels.f = ALLOC_N(double, npixels);
        stg_type = DoublePixel;
        pixel_class = rb_cFloat;
    }
    else if (rb_obj_is_kind_of(pixel0, rb_cInteger) == Qtrue)
    {
        pixels.i = ALLOC_N(Quantum, npixels);
        stg_type = QuantumPixel;
        pixel_class = rb_cInteger;
    }
    else
    {
        rb_raise(rb_eTypeError, "element 0 in pixel array is %s, must be numeric"
                 , rb_class2name(CLASS_OF(pixel0)));
    }



    // Convert the array elements to the appropriate C type, store in pixel
    // buffer.
    for (x = 0; x < npixels; x++)
    {
        pixel = rb_ary_entry(pixels_arg, x);
        if (rb_obj_is_kind_of(pixel, pixel_class) != Qtrue)
        {
            rb_raise(rb_eTypeError, "element %ld in pixel array is %s, expected %s"
                     , x, rb_class2name(CLASS_OF(pixel)),rb_class2name(CLASS_OF(pixel0)));
        }
        if (pixel_class == rb_cFloat)
        {
            pixels.f[x] = (float) NUM2DBL(pixel);
            if (pixels.f[x] < 0.0 || pixels.f[x] > 1.0)
            {
                rb_raise(rb_eArgError, "element %ld is out of range [0..1]: %f", x, pixels.f[x]);
            }
        }
        else
        {
            pixels.i[x] = NUM2QUANTUM(pixel);
        }
    }

    GetExceptionInfo(&exception);

    // This is based on ConstituteImage in IM 5.5.7
    image = AcquireImage(NULL);
    if (!image)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue.");
    }

    SetImageExtent(image, width, height);
    rm_check_image_exception(image, DestroyOnError);

    (void) SetImageBackgroundColor(image);
    rm_check_image_exception(image, DestroyOnError);

    (void) ImportImagePixels(image, 0, 0, width, height, map, stg_type, (const void *)pixels.v);
    xfree(pixels.v);
    rm_check_image_exception(image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    DestroyConstitute();

    return rm_image_new(image);
}

/**
 * Enhance the intensity differences between the lighter and darker elements of
 * the image. Set sharpen to "true" to increase the image contrast otherwise the
 * contrast is reduced.
 *
 * Ruby usage:
 *   - @verbatim Image#contrast @endverbatim
 *   - @verbatim Image#contrast(sharpen) @endverbatim
 *
 * Notes:
 *   - Default sharpen is 0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_contrast(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    unsigned int sharpen = 0;

    image = rm_check_destroyed(self);
    if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }
    else if (argc == 1)
    {
        sharpen = RTEST(argv[0]);
    }

    new_image = rm_clone_image(image);

    (void) ContrastImage(new_image, sharpen);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Convert percentages to #pixels. If the white-point (2nd) argument is not
 * supplied set it to #pixels - black-point.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - No return value: modifies black_point and white_point directly.
 *
 * @param image the image
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param black_point pointer to the black point
 * @param white_point pointer to the white point
 */
static void
get_black_white_point(Image *image, int argc, VALUE *argv, double *black_point, double *white_point)
{
    double pixels;

    pixels = (double) (image->columns * image->rows);

    switch (argc)
    {
        case 2:
            if (rm_check_num2dbl(argv[0]))
            {
                *black_point = NUM2DBL(argv[0]);
            }
            else
            {
                *black_point = pixels * rm_str_to_pct(argv[0]);
            }
            if (rm_check_num2dbl(argv[1]))
            {
                *white_point = NUM2DBL(argv[1]);
            }
            else
            {
                *white_point = pixels * rm_str_to_pct(argv[1]);
            }
            break;

        case 1:
            if (rm_check_num2dbl(argv[0]))
            {
                *black_point = NUM2DBL(argv[0]);
            }
            else
            {
                *black_point = pixels * rm_str_to_pct(argv[0]);
            }
            *white_point = pixels - *black_point;
            break;

        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    return;
}


/**
 * Call ContrastStretchImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#contrast_stretch_channel(black_point) @endverbatim
 *   - @verbatim Image#contrast_stretch_channel(black_point, white_point) @endverbatim
 *   - @verbatim Image#contrast_stretch_channel(black_point, white_point, channel) @endverbatim
 *   - @verbatim Image#contrast_stretch_channel(black_point, white_point, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default white_point is pixels-black_point
 *   - Default channel is AllChannels
 *   - Both black_point and white_point can be specified as Floats or as
 *     percentages, i.e. "10%"
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_contrast_stretch_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;
    double black_point, white_point;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 2)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    get_black_white_point(image, argc, argv, &black_point, &white_point);

    new_image = rm_clone_image(image);

    (void) ContrastStretchImageChannel(new_image, channels, black_point, white_point);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}

/**
 * Apply a custom convolution kernel to the image.
 *
 * Ruby usage:
 *   - @verbatim Image#convolve(order, kernel) @endverbatim
 *
 * @param self this object
 * @param order_arg the number of rows and columns in the kernel
 * @param kernel_arg an order**2 array of doubles
 * @return a new image
 */
VALUE
Image_convolve(VALUE self, VALUE order_arg, VALUE kernel_arg)
{
    Image *image, *new_image;
    double *kernel;
    unsigned int x, order;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    order = NUM2UINT(order_arg);

    kernel_arg = rb_Array(kernel_arg);
    rm_check_ary_len(kernel_arg, (long)(order*order));

    // Convert the kernel array argument to an array of doubles

    kernel = (double *)ALLOC_N(double, order*order);
    for (x = 0; x < order*order; x++)
    {
        kernel[x] = NUM2DBL(rb_ary_entry(kernel_arg, (long)x));
    }

    GetExceptionInfo(&exception);

    new_image = ConvolveImage((const Image *)image, order, (double *)kernel, &exception);
    xfree((void *)kernel);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * call ConvolveImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#convolve_channel(order, kernel) @endverbatim
 *   - @verbatim Image#convolve_channel(order, kernel, channel) @endverbatim
 *   - @verbatim Image#convolve_channel(order, kernel, channel, ...) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_convolve_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double *kernel;
    volatile VALUE ary;
    unsigned int x, order;
    ChannelType channels;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    // There are 2 required arguments.
    if (argc > 2)
    {
        raise_ChannelType_error(argv[argc-1]);
    }
    if (argc != 2)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or more)", argc);
    }

    order = NUM2UINT(argv[0]);
    ary = argv[1];

    rm_check_ary_len(ary, (long)(order*order));

    kernel = ALLOC_N(double, (long)(order*order));

    // Convert the kernel array argument to an array of doubles
    for (x = 0; x < order*order; x++)
    {
        kernel[x] = NUM2DBL(rb_ary_entry(ary, (long)x));
    }

    GetExceptionInfo(&exception);

    new_image = ConvolveImageChannel(image, channels, order, kernel, &exception);
    xfree((void *)kernel);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}



/**
 * Alias for dup.
 *
 * Ruby usage:
 *   - @verbatim Image#copy @endverbatim
 *
 * @param self this object
 * @return a copy of self
 * @see Image_dup
 */
VALUE
Image_copy(VALUE self)
{
    return rb_funcall(self, rm_ID_dup, 0);
}

/**
 * Initialize copy, clone, dup.
 *
 * Ruby usage:
 *   - @verbatim Image#initialize_copy @endverbatim
 *
 * @param copy the destination image
 * @param orig the source image
 * @return copy
 * @see Image_copy
 * @see Image_clone
 * @see Image_dup
 */
VALUE
Image_init_copy(VALUE copy, VALUE orig)
{
    Image *image, *new_image;

    image = rm_check_destroyed(orig);
    new_image = rm_clone_image(image);
    UPDATE_DATA_PTR(copy, new_image);

    return copy;
}


/**
 * Extract a region of the image defined by width, height, x, y.
 *
 * Ruby usage:
 *   - @verbatim Image#crop(x, y, width, height) @endverbatim
 *   - @verbatim Image#crop(gravity, width, height) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see cropper
 * @see Image_crop_bang
 */
VALUE
Image_crop(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return cropper(False, argc, argv, self);
}


/**
 * Extract a region of the image defined by width, height, x, y.
 *
 * Ruby usage:
 *   - @verbatim Image#crop!(x, y, width, height) @endverbatim
 *   - @verbatim Image#crop!(gravity, width, height) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see cropper
 * @see Image_crop
 */
VALUE
Image_crop_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return cropper(True, argc, argv, self);
}


/**
 * Call CycleColormapImage.
 *
 * Ruby usage:
 *   - @verbatim Image#cycle_colormap @endverbatim
 *
 * @param self this object
 * @param amount amount to cycle the colormap
 * @return a new image
 */
VALUE
Image_cycle_colormap(VALUE self, VALUE amount)
{
    Image *image, *new_image;
    int amt;

    image = rm_check_destroyed(self);

    new_image = rm_clone_image(image);
    amt = NUM2INT(amount);
    (void) CycleColormapImage(new_image, amt);
    // No need to check for an error

    return rm_image_new(new_image);
}


/**
 * Get the x & y resolutions.
 *
 * Ruby usage:
 *   - @verbatim Image#density @endverbatim
 *
 * @param self this object
 * @return a string in the form "XresxYres"
 */
VALUE
Image_density(VALUE self)
{
    Image *image;
    char density[128];

    image = rm_check_destroyed(self);

    sprintf(density, "%gx%g", image->x_resolution, image->y_resolution);
    return rb_str_new2(density);
}


/**
 * Set the x & y resolutions in the image.
 *
 * Ruby usage:
 *   - @verbatim Image#density="XxY" @endverbatim
 *   - @verbatim Image#density=aGeometry @endverbatim
 *
 * Notes:
 *   - The density is a string of the form "XresxYres" or simply "Xres".
 *   - If the y resolution is not specified, set it equal to the x resolution.
 *   - This is equivalent to PerlMagick's handling of density.
 *   - The density can also be a Geometry object. The width attribute is used
 *     for the x resolution. The height attribute is used for the y resolution.
 *     If the height attribute is missing, the width attribute is used for both.
 *
 * @param self this object
 * @param density_arg The density String or Geometry
 * @return self
 */
VALUE
Image_density_eq(VALUE self, VALUE density_arg)
{
    Image *image;
    char *density;
    volatile VALUE x_val, y_val;
    int count;
    double x_res, y_res;

    image = rm_check_frozen(self);

    // Get the Class ID for the Geometry class.
    if (!Class_Geometry)
    {
        Class_Geometry = rb_const_get(Module_Magick, rm_ID_Geometry);
    }

    // Geometry object. Width and height attributes are always positive.
    if (CLASS_OF(density_arg) == Class_Geometry)
    {
        x_val = rb_funcall(density_arg, rm_ID_width, 0);
        x_res = NUM2DBL(x_val);
        y_val = rb_funcall(density_arg, rm_ID_height, 0);
        y_res = NUM2DBL(y_val);
        if (x_res == 0.0)
        {
            rb_raise(rb_eArgError, "invalid x resolution: %f", x_res);
        }
        image->y_resolution = y_res != 0.0 ? y_res : x_res;
        image->x_resolution = x_res;
    }

    // Convert the argument to a string
    else
    {
        density = StringValuePtr(density_arg);
        if (!IsGeometry(density))
        {
            rb_raise(rb_eArgError, "invalid density geometry %s", density);
        }

        count = sscanf(density, "%lfx%lf", &image->x_resolution, &image->y_resolution);
        if (count < 2)
        {
            image->y_resolution = image->x_resolution;
        }

    }

    return self;
}


/**
 * call DecipherImage.
 *
 * Ruby usage:
 *   - @verbatim Image#decipher(passphrase) @endverbatim
 *
 * @param self this object
 * @param passphrase the passphrase
 * @return a new deciphered image
 */
VALUE
Image_decipher(VALUE self, VALUE passphrase)
{
#if defined(HAVE_ENCIPHERIMAGE)
    Image *image, *new_image;
    char *pf;
    ExceptionInfo exception;
    MagickBooleanType okay;

    image = rm_check_destroyed(self);
    pf = StringValuePtr(passphrase);      // ensure passphrase is a string
    GetExceptionInfo(&exception);

    new_image = rm_clone_image(image);

    okay = DecipherImage(new_image, pf, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    if (!okay)
    {
        new_image = DestroyImage(new_image);
        rb_raise(rb_eRuntimeError, "DecipherImage failed for unknown reason.");
    }

    DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
#else
    self = self;
    passphrase = passphrase;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Call SetImageArtifact.
 *
 * Ruby usage:
 *   - @verbatim value = Image#define(artifact, value) @endverbatim
 *
 * Notes:
 *   - Normally a script should never call this method. Any calls to
 *     SetImageArtifact will be part of the methods in which they're needed, or
 *     be called via the OptionalMethodArguments class.
 *   - If value is nil, the artifact will be removed
 *
 * @param self this object
 * @param artifact the artifact to set
 * @param value the value to which to set the artifact
 * @return the value
 */
VALUE
Image_define(VALUE self, VALUE artifact, VALUE value)
{
#if defined(HAVE_SETIMAGEARTIFACT)
    Image *image;
    char *key, *val;
    MagickBooleanType status;

    image = rm_check_frozen(self);
    artifact = rb_String(artifact);
    key = StringValuePtr(artifact);

    if (value == Qnil)
    {
        (void) DeleteImageArtifact(image, key);
    }
    else
    {
        value = rb_String(value);
        val = StringValuePtr(value);
        status = SetImageArtifact(image, key, val);
        if (!status)
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
    }

    return value;
#else
    rm_not_implemented();
    artifact = artifact;
    value = value;
    self = self;
    return(VALUE)0;
#endif
}


DEF_ATTR_ACCESSOR(Image, delay, ulong)


/**
 * Delete the image composite mask.
 *
 * Ruby usage:
 *   - @verbatim Image#delete_compose_mask() @endverbatim
 *
 * @param self this object
 * @return self
 * @see Image_add_compose_mask
 * @see SetImageMask in ImageMagick
 */
VALUE
Image_delete_compose_mask(VALUE self)
{
    Image *image = rm_check_frozen(self);

    // Store a clone of the mask image
    (void) SetImageMask(image, NULL);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Call ProfileImage.
 *
 * Ruby usage:
 *   - @verbatim Image#delete_profile(name) @endverbatim
 *
 * @param self this object
 * @param name the name of the profile to be deleted
 * @return self
 */
VALUE
Image_delete_profile(VALUE self, VALUE name)
{
    Image *image = rm_check_frozen(self);
    (void) ProfileImage(image, StringValuePtr(name), NULL, 0, MagickTrue);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Return the image depth (8 or 16).
 *
 * Ruby usage:
 *   - @verbatim Image#depth @endverbatim
 *
 * Notes:
 *   - If all pixels have lower-order bytes equal to higher-order bytes, the
 *     depth will be reported as 8 even if the depth field in the Image
 *     structure says 16.
 *
 * @param self this object
 * @return the depth
 */
VALUE
Image_depth(VALUE self)
{
    Image *image;
    unsigned long depth = 0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    depth = GetImageDepth(image, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return INT2FIX(depth);
}


/**
 * Implement convert -deskew option.
 *
 * Ruby usage:
 *   - @verbatim Image#deskew @endverbatim
 *   - @verbatim Image#deskew(threshold) @endverbatim
 *   - @verbatim Image#deskew(threshold, auto-crop-width) @endverbatim
 *
 * Notes:
 *   - Default threshold is 0.40
 *   - Default auto-crop-width is the auto crop width of the image
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_deskew(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_DESKEWIMAGE)
    Image *image, *new_image;
    double threshold = 40.0 * QuantumRange / 100.0;
    unsigned long width;
    char auto_crop_width[20];
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 2:
            width = NUM2ULONG(argv[1]);
            memset(auto_crop_width, 0, sizeof(auto_crop_width));
            sprintf(auto_crop_width, "%ld", width);
            SetImageArtifact(image, "deskew:auto-crop", auto_crop_width);
        case 1:
            threshold = rm_percentage(argv[0],1.0) * QuantumRange;
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = DeskewImage(image, threshold, &exception);
    CHECK_EXCEPTION()
    rm_ensure_result(new_image);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
#else
    self = self;        // defeat "unused parameter" message
    argv = argv;
    argc = argc;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Reduce the speckle noise in an image while preserving the edges of the
 * original image.
 *
 * Ruby usage:
 *   - @verbatim Image#despeckle @endverbatim
 *
 * @param self this object
 * @return a new image
 */
VALUE
Image_despeckle(VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    new_image = DespeckleImage(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Free all the memory associated with an image.
 *
 * Ruby usage:
 *   - @verbatim Image#destroy! @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
Image_destroy_bang(VALUE self)
{
    Image *image;

    rb_check_frozen(self);
    Data_Get_Struct(self, Image, image);
    rm_image_destroy(image);
    DATA_PTR(self) = NULL;
    return self;
}


/**
 * Return true if the image has been destroyed, false otherwise.
 *
 * Ruby usage:
 *   - @verbatim Image#destroyed? @endverbatim
 *
 * @param self this object
 * @return true if destroyed, false otherwise
 */
VALUE
Image_destroyed_q(VALUE self)
{
    Image *image;

    Data_Get_Struct(self, Image, image);
    return image ? Qfalse : Qtrue;
}


/**
 * Call the IsImagesEqual function.
 *
 * Ruby usage:
 *   - @verbatim Image#difference @endverbatim
 *
 * Notes:
 *   - "other" can be either an Image or an Image
 *
 * @param self this object
 * @param other another Image
 * @return An array with 3 values: [mean error per pixel, normalized mean error,
 * normalized maximum error]
 */
VALUE
Image_difference(VALUE self, VALUE other)
{
    Image *image;
    Image *image2;
    volatile VALUE mean, nmean, nmax;

    image = rm_check_destroyed(self);
    other = rm_cur_image(other);
    image2 = rm_check_destroyed(other);

    (void) IsImagesEqual(image, image2);
    // No need to check for error

    mean  = rb_float_new(image->error.mean_error_per_pixel);
    nmean = rb_float_new(image->error.normalized_mean_error);
    nmax  = rb_float_new(image->error.normalized_maximum_error);
    return rb_ary_new3(3, mean, nmean, nmax);
}


/**
 * Get image directory.
 *
 * Ruby usage:
 *   - @verbatim Image#directory @endverbatim
 *
 * @param self this object
 * @return the directory
 */
DEF_ATTR_READER(Image, directory, str)


/**
 * Implement the -displace option of xMagick's composite command.
 *
 * Ruby usage:
 *   - @verbatim Image#displace(displacement_map, x_amp) @endverbatim
 *   - @verbatim Image#displace(displacement_map, x_amp, y_amp) @endverbatim
 *   - @verbatim Image#displace(displacement_map, x_amp, y_amp, x_offset) @endverbatim
 *   - @verbatim Image#displace(displacement_map, x_amp, y_amp, x_offset, y_offset) @endverbatim
 *   - @verbatim Image#displace(displacement_map, x_amp, y_amp, gravity) @endverbatim
 *   - @verbatim Image#displace(displacement_map, x_amp, y_amp, gravity, x_offset) @endverbatim
 *   - @verbatim Image#displace(displacement_map, x_amp, y_amp, gravity, x_offset, y_offset) @endverbatim
 *
 * Notes:
 *   - If y_amp is omitted the default is x_amp.
 *   - Default x_offset is 0
 *   - Default y_offset is 0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see special_composite
 */
VALUE
Image_displace(int argc, VALUE *argv, VALUE self)
{
    Image *image, *displacement_map;
    volatile VALUE dmap;
    double x_amplitude = 0.0, y_amplitude = 0.0;
    long x_offset = 0L, y_offset = 0L;

    image = rm_check_destroyed(self);

    if (argc < 2)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
    }

    dmap = rm_cur_image(argv[0]);
    displacement_map = rm_check_destroyed(dmap);

    if (argc > 3)
    {
        get_composite_offsets(argc-3, &argv[3], image, displacement_map, &x_offset, &y_offset);
        // There must be 3 arguments left
        argc = 3;
    }

    switch (argc)
    {
        case 3:
            y_amplitude = NUM2DBL(argv[2]);
            x_amplitude = NUM2DBL(argv[1]);
            break;
        case 2:
            x_amplitude = NUM2DBL(argv[1]);
            y_amplitude = x_amplitude;
            break;
    }

    return special_composite(image, displacement_map, x_amplitude, y_amplitude
                             , x_offset, y_offset, DisplaceCompositeOp);
}


/**
 * Extract pixel data from the image and returns it as an array of pixels. The
 * "x", "y", "width" and "height" parameters specify the rectangle to be
 * extracted. The "map" parameter reflects the expected ordering of the pixel
 * array. It can be any combination or order of R = red, G = green, B = blue,
 * A = alpha, C = cyan, Y = yellow, M = magenta, K = black, or I = intensity
 * (for grayscale). If the "float" parameter is specified and true, the pixel
 * data is returned as floating-point numbers in the range [0..1]. By default
 * the pixel data is returned as integers in the range [0..QuantumRange].
 *
 * Ruby usage:
 *   - @verbatim Image#dispatch(x, y, columns, rows, map) @endverbatim
 *   - @verbatim Image#dispatch(x, y, columns, rows, map, float) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return an Array of pixel data
 * @throw ArgumentError
 */
VALUE
Image_dispatch(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    long x, y;
    unsigned long columns, rows, n, npixels;
    volatile VALUE pixels_ary;
    StorageType stg_type = QuantumPixel;
    char *map;
    long mapL;
    MagickBooleanType okay;
    ExceptionInfo exception;
    volatile union
    {
        Quantum *i;
        double *f;
        void *v;
    } pixels;

    (void) rm_check_destroyed(self);

    if (argc < 5 || argc > 6)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 5 or 6)", argc);
    }

    x       = NUM2LONG(argv[0]);
    y       = NUM2LONG(argv[1]);
    columns = NUM2ULONG(argv[2]);
    rows    = NUM2ULONG(argv[3]);
    map     = rm_str2cstr(argv[4], &mapL);
    if (argc == 6)
    {
        stg_type = RTEST(argv[5]) ? DoublePixel : QuantumPixel;
    }

    // Compute the size of the pixel array and allocate the memory.
    npixels = columns * rows * mapL;
    pixels.v = stg_type == QuantumPixel ? (void *) ALLOC_N(Quantum, npixels)
               : (void *) ALLOC_N(double, npixels);

    // Create the Ruby array for the pixels. Return this even if ExportImagePixels fails.
    pixels_ary = rb_ary_new();

    Data_Get_Struct(self, Image, image);

    GetExceptionInfo(&exception);
    okay = ExportImagePixels(image, x, y, columns, rows, map, stg_type, (void *)pixels.v, &exception);

    if (!okay)
    {
        goto exit;
    }

    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    // Convert the pixel data to the appropriate Ruby type
    if (stg_type == QuantumPixel)
    {
        for (n = 0; n < npixels; n++)
        {
            (void) rb_ary_push(pixels_ary, QUANTUM2NUM(pixels.i[n]));
        }
    }
    else
    {
        for (n = 0; n < npixels; n++)
        {
            (void) rb_ary_push(pixels_ary, rb_float_new(pixels.f[n]));
        }
    }

    exit:
    xfree((void *)pixels.v);
    return pixels_ary;
}


/**
 * Display the image to an X window screen.
 *
 * Ruby usage:
 *   - @verbatim Image#display @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
Image_display(VALUE self)
{
    Image *image;
    Info *info;
    volatile VALUE info_obj;

    image = rm_check_destroyed(self);

    if (image->rows == 0 || image->columns == 0)
    {
        rb_raise(rb_eArgError, "invalid image geometry (%lux%lu)", image->rows, image->columns);
    }

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    (void) DisplayImages(info, image);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Return the dispose attribute as a DisposeType enum.
 *
 * Ruby usage:
 *   - @verbatim Image#dispose @endverbatim
 * 
 * @param self this object
 * @return the dispose
 */
VALUE
Image_dispose(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return DisposeType_new(image->dispose);
}


/**
 * Set the dispose attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#dispose= @endverbatim
 *
 * @param self this object
 * @param dispose the dispose
 * @return self
 */
VALUE
Image_dispose_eq(VALUE self, VALUE dispose)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(dispose, image->dispose, DisposeType);
    return self;
}


/**
 * Corresponds to the composite_image -dissolve operation.
 *
 * Ruby usage:
 *   - @verbatim Image#dissolve(overlay, src_percent) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, x_offset) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, x_offset, y_offset) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, gravity) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, gravity, x_offset) @endverbatim
 *   - @verbatim Image#dissolve(overlay, src_percent, dst_percent, gravity, x_offset, y_offset) @endverbatim
 *
 * Notes:
 *   - `percent' can be a number or a string in the form "NN%"
 *   - Default dst_percent is -1.0 (tells blend_geometry to leave it out of the
 *     geometry string)
 *   - Default x_offset is 0
 *   - Default y_offset is 0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see special_composite
 */
VALUE
Image_dissolve(int argc, VALUE *argv, VALUE self)
{
    Image *image, *overlay;
    double src_percent, dst_percent = -1.0;
    long x_offset = 0L, y_offset = 0L;
    volatile VALUE composite_image, ovly;

    image = rm_check_destroyed(self);

    if (argc < 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
    }

    ovly = rm_cur_image(argv[0]);
    overlay = rm_check_destroyed(ovly);

    if (argc > 3)
    {
        get_composite_offsets(argc-3, &argv[3], image, overlay, &x_offset, &y_offset);
        // There must be 3 arguments left
        argc = 3;
    }

    switch (argc)
    {
        case 3:
            dst_percent = rm_percentage(argv[2],1.0) * 100.0;
        case 2:
            src_percent = rm_percentage(argv[1],1.0) * 100.0;
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
            break;
    }

    composite_image =  special_composite(image, overlay, src_percent, dst_percent
                                         , x_offset, y_offset, DissolveCompositeOp);

    return composite_image;
}


/**
 * Call DistortImage.
 *
 * Ruby usage:
 *   - @verbatim Image#distort(type, points) { optional arguments } @endverbatim
 *   - @verbatim Image#distort(type, points, bestfit) { optional arguments } @endverbatim
 *
 * Notes:
 *   - Default bestfit is false
 *   - Points is an Array of Numeric values
 *   - Optional arguments are:
 *     - self.define "distort:viewport", WxH+X+Y
 *     - self.define "distort:scale", N
 *     - self.verbose true
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_distort(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    volatile VALUE pts;
    unsigned long n, npoints;
    DistortImageMethod distortion_method;
    double *points;
    MagickBooleanType bestfit = MagickFalse;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    rm_get_optional_arguments(self);

    switch (argc)
    {
        case 3:
            bestfit = RTEST(argv[2]);
        case 2:
            // Ensure pts is an array
            pts = rb_Array(argv[1]);
            VALUE_TO_ENUM(argv[0], distortion_method, DistortImageMethod);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (expected 2 or 3, got %d)", argc);
            break;
    }

    npoints = RARRAY_LEN(pts);
    // Allocate points array from Ruby's memory. If an error occurs Ruby will
    // be able to clean it up.
    points = ALLOC_N(double, npoints);

    for (n = 0; n < npoints; n++)
    {
        points[n] = NUM2DBL(rb_ary_entry(pts, n));
    }

    GetExceptionInfo(&exception);
    new_image = DistortImage(image, distortion_method, npoints, points, bestfit, &exception);
    xfree(points);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call GetImageChannelDistortion.
 *
 * Ruby usage:
 *   - @verbatim Image#distortion_channel(reconstructed_image, metric) @endverbatim
 *   - @verbatim Image#distortion_channel(reconstructed_image, metric, channel) @endverbatim
 *   - @verbatim Image#distortion_channel(reconstructed_image, metric, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the image channel distortion (Ruby float)
 */
VALUE
Image_distortion_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *reconstruct;
    ChannelType channels;
    ExceptionInfo exception;
    MetricType metric;
    volatile VALUE rec;
    double distortion;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 2)
    {
        raise_ChannelType_error(argv[argc-1]);
    }
    if (argc < 2)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or more)", argc);
    }

    rec = rm_cur_image(argv[0]);
    reconstruct = rm_check_destroyed(rec);
    VALUE_TO_ENUM(argv[1], metric, MetricType);
    GetExceptionInfo(&exception);
    (void) GetImageChannelDistortion(image, reconstruct, channels
                                     , metric, &distortion, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return rb_float_new(distortion);
}


/**
 * Implement marshalling.
 *
 * Ruby usage:
 *   - @verbatim Image#_dump(aDepth) @endverbatim
 *
 * Notes:
 *   - Uses ImageToBlob - use the MIFF format in the blob since it's the most
 *     general
 *
 * @param self this object
 * @param depth the depth to which to dump (unused)
 * @return a string representing the dumped image
 */
VALUE
Image__dump(VALUE self, VALUE depth)
{
    Image *image;
    ImageInfo *info;
    void *blob;
    size_t length;
    DumpedImage mi;
    volatile VALUE str;
    ExceptionInfo exception;

    depth = depth;  // Suppress "never referenced" message from icc

    image = rm_check_destroyed(self);

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    strcpy(info->magick, image->magick);

    GetExceptionInfo(&exception);
    blob = ImageToBlob(info, image, &length, &exception);

    // Free ImageInfo first - error handling may raise an exception
    (void) DestroyImageInfo(info);

    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    if (!blob)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    // Create a header for the blob: ID and version
    // numbers, followed by the length of the magick
    // string stored as a byte, followed by the
    // magick string itself.
    mi.id = DUMPED_IMAGE_ID;
    mi.mj = DUMPED_IMAGE_MAJOR_VERS;
    mi.mi = DUMPED_IMAGE_MINOR_VERS;
    strcpy(mi.magick, image->magick);
    mi.len = (unsigned char) min((size_t)UCHAR_MAX, strlen(mi.magick));

    // Concatenate the blob onto the header & return the result
    str = rb_str_new((char *)&mi, (long)(mi.len+offsetof(DumpedImage,magick)));
    str = rb_str_buf_cat(str, (char *)blob, (long)length);
    magick_free((void*)blob);
    return str;
}


/**
 * Construct a new image object and call initialize_copy.
 *
 * Ruby usage:
 *   - @verbatim Image#dup @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see Image_copy
 * @see Image_init_copy
 */
VALUE
Image_dup(VALUE self)
{
    volatile VALUE dup;

    (void) rm_check_destroyed(self);
    dup = Data_Wrap_Struct(CLASS_OF(self), NULL, rm_image_destroy, NULL);
    if (rb_obj_tainted(self))
    {
        (void) rb_obj_taint(dup);
    }
    return rb_funcall(dup, rm_ID_initialize_copy, 1, self);
}


/**
 * Iterate over image profiles.
 *
 * Ruby usage:
 *   - @verbatim Image#each_profile @endverbatim
 *
 * Notes:
 *   - ImageMagick only
 *
 * @param self this object
 * @return iterator over image profiles
 */
VALUE
Image_each_profile(VALUE self)
{
    Image *image;
    volatile VALUE ary, val;
    char *name;
    const StringInfo *profile;

    image = rm_check_destroyed(self);
    ResetImageProfileIterator(image);

    ary = rb_ary_new2(2);

    name = GetNextImageProfile(image);
    while (name)
    {
        rb_ary_store(ary, 0, rb_str_new2(name));

        profile = GetImageProfile(image, name);
        if (!profile)
        {
            rb_ary_store(ary, 1, Qnil);
        }
        else
        {
            rb_ary_store(ary, 1, rb_str_new((char *)profile->datum, (long)profile->length));
        }
        val = rb_yield(ary);
        name = GetNextImageProfile(image);
    }

    return val;
}


/**
 * Find edges in an image. "radius" defines the radius of the convolution
 * filter.
 *
 * Ruby usage:
 *   - @verbatim Image#edge @endverbatim
 *   - @verbatim Image#edge(radius) @endverbatim
 *
 * Notes:
 *   - Default radius is 0 (have edge select a suitable radius)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_edge(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double radius = 0.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = EdgeImage(image, radius, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call one of the effects methods.
 *
 * No Ruby usage (internal function)
 *
 * @param self this object
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param effector the effector to call
 * @return a new image
 */
static VALUE
effect_image(VALUE self, int argc, VALUE *argv, effector_t effector)
{
    Image *image, *new_image;
    ExceptionInfo exception;
    double radius = 0.0, sigma = 1.0;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 2:
            sigma = NUM2DBL(argv[1]);
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
            break;
    }

    if (sigma == 0.0)
    {
        rb_raise(rb_eArgError, "sigma must be != 0.0");
    }

    GetExceptionInfo(&exception);
    new_image = (effector)(image, radius, sigma, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Create a grayscale image with a three-dimensional effect.
 *
 * Ruby usage:
 *   - @verbatim Image#emboss @endverbatim
 *   - @verbatim Image#emboss(radius) @endverbatim
 *   - @verbatim Image#emboss(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see effect_image
 */
VALUE
Image_emboss(int argc, VALUE *argv, VALUE self)
{
    return effect_image(self, argc, argv, EmbossImage);
}


/**
 * Call EncipherImage.
 *
 * Ruby usage:
 *   - @verbatim Image#encipher(passphrase) @endverbatim
 *
 * @param self this object
 * @param passphrase the passphrase with which to encipher
 * @return a new image
 */
VALUE
Image_encipher(VALUE self, VALUE passphrase)
{
#if defined(HAVE_ENCIPHERIMAGE)
    Image *image, *new_image;
    char *pf;
    ExceptionInfo exception;
    MagickBooleanType okay;

    image = rm_check_destroyed(self);
    pf = StringValuePtr(passphrase);      // ensure passphrase is a string
    GetExceptionInfo(&exception);

    new_image = rm_clone_image(image);

    okay = EncipherImage(new_image, pf, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    if (!okay)
    {
        new_image = DestroyImage(new_image);
        rb_raise(rb_eRuntimeError, "EncipherImage failed for unknown reason.");
    }

    DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
#else
    self = self;
    passphrase = passphrase;
    rm_not_implemented();
    return(VALUE)0;
#endif
}



/**
 * Return endian option for images that support it.
 *
 * Ruby usage:
 *   - @verbatim Image#endian @endverbatim
 *
 * @param self this object
 * @return the endian option
 */
VALUE
Image_endian(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return EndianType_new(image->endian);
}


/**
 * Set endian option for images that support it.
 *
 * Ruby usage:
 *   - @verbatim Image#endian= @endverbatim
 *
 * @param self this object
 * @param type the endian type
 * @return self
 */
VALUE
Image_endian_eq(VALUE self, VALUE type)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(type, image->endian, EndianType);
    return self;
}

/**
 * Apply a digital filter that improves the quality of a noisy image.
 *
 * Ruby usage:
 *   - @verbatim Image#enhance @endverbatim
 *
 * @param self this object
 * @return a new image
 */
VALUE
Image_enhance(VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    new_image = EnhanceImage(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Apply a histogram equalization to the image.
 *
 * Ruby usage:
 *   - @verbatim Image#equalize @endverbatim
 *
 * @param self this object
 * @return a new image
 */
VALUE
Image_equalize(VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);
    new_image = rm_clone_image(image);

    (void) EqualizeImage(new_image);
    rm_check_image_exception(new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Call EqualizeImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#equalize_channel @endverbatim
 *   - @verbatim Image#equalize_channel(channel) @endverbatim
 *   - @verbatim Image#equalize_channel(channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_equalize_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_EQUALIZEIMAGECHANNEL)
    Image *image, *new_image;
    ExceptionInfo exception;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    new_image = rm_clone_image(image);

    GetExceptionInfo(&exception);

    (void) EqualizeImageChannel(new_image, channels);

    rm_check_image_exception(new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
#else
    argc = argc;
    argv = argv;
    self = self;
    rm_not_implemented();
    return(VALUE) 0;
#endif
}


/**
 * Reset the image to the background color.
 *
 * Ruby usage:
 *   - @verbatim Image#erase! @endverbatim
 *
 * Notes:
 *   - One of the very few Image methods that do not return a new image.
 *
 * @param self this object
 * @return self
 */
VALUE
Image_erase_bang(VALUE self)
{
    Image *image = rm_check_frozen(self);

    (void) SetImageBackgroundColor(image);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Lightweight crop.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - christy says "does not respect the virtual page offset (-page) and does
 *     not update the page offset and its more efficient than cropping."
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @param x the x position for the start of the rectangle
 * @param y the y position for the start of the rectangle
 * @param width the width of the rectancle
 * @param height the height of the rectangle
 * @return self if bang, otherwise a new image
 * @see Image_excerpt
 * @see Image_excerpt_bang
 * @see Image_crop
 * @see Image_crop_bang
 */
static VALUE
excerpt(int bang, VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
    Image *image, *new_image;
    RectangleInfo rect;
    ExceptionInfo exception;

    memset(&rect,'\0', sizeof(rect));
    rect.x = NUM2LONG(x);
    rect.y = NUM2LONG(y);
    rect.width = NUM2ULONG(width);
    rect.height = NUM2ULONG(height);

    Data_Get_Struct(self, Image, image);

    GetExceptionInfo(&exception);
    new_image = ExcerptImage(image, &rect, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);
}


/**
 * Lightweight crop.
 *
 * Ruby usage:
 *   - @verbatim Image#excerpt(x, y, width, height) @endverbatim
 *
 * @param self this object
 * @param x the x position for the start of the rectangle
 * @param y the y position for the start of the rectangle
 * @param width the width of the rectancle
 * @param height the height of the rectangle
 * @return self if bang, otherwise a new image
 * @see excerpt
 * @see Image_excerpt_bang
 * @see Image_crop
 * @see Image_crop_bang
 */
VALUE
Image_excerpt(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
    (void) rm_check_destroyed(self);
    return excerpt(False, self, x, y, width, height);
}


/**
 * Lightweight crop.
 *
 * Ruby usage:
 *   - @verbatim Image#excerpt!(x, y, width, height) @endverbatim
 *
 * @param self this object
 * @param x the x position for the start of the rectangle
 * @param y the y position for the start of the rectangle
 * @param width the width of the rectancle
 * @param height the height of the rectangle
 * @return self
 * @see excerpt
 * @see Image_excerpt
 * @see Image_crop
 * @see Image_crop_bang
 */
VALUE
Image_excerpt_bang(VALUE self, VALUE x, VALUE y, VALUE width, VALUE height)
{
    (void) rm_check_frozen(self);
    return excerpt(True, self, x, y, width, height);
}


/**
 * Extract image pixels in the form of an array.
 *
 * Ruby usage:
 *   - @verbatim Image#export_pixels @endverbatim
 *   - @verbatim Image#export_pixels(x) @endverbatim
 *   - @verbatim Image#export_pixels(x, y) @endverbatim
 *   - @verbatim Image#export_pixels(x, y, cols) @endverbatim
 *   - @verbatim Image#export_pixels(x, y, cols, rows) @endverbatim
 *   - @verbatim Image#export_pixels(x, y, cols, rows, map) @endverbatim
 *
 * Notes:
 *   - Default x is 0
 *   - Default y is 0
 *   - Default cols is self.columns
 *   - Default rows is self.rows
 *   - Default map is "RGB"
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return array of pixels
 */
VALUE
Image_export_pixels(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    long x_off = 0L, y_off = 0L;
    unsigned long cols, rows;
    long n, npixels;
    unsigned int okay;
    const char *map = "RGB";
    Quantum *pixels;
    volatile VALUE ary;
    ExceptionInfo exception;


    image = rm_check_destroyed(self);
    cols = image->columns;
    rows = image->rows;

    switch (argc)
    {
        case 5:
            map   = StringValuePtr(argv[4]);
        case 4:
            rows  = NUM2ULONG(argv[3]);
        case 3:
            cols  = NUM2ULONG(argv[2]);
        case 2:
            y_off = NUM2LONG(argv[1]);
        case 1:
            x_off = NUM2LONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 5)", argc);
            break;
    }

    if (   x_off < 0 || (unsigned long)x_off > image->columns
           || y_off < 0 || (unsigned long)y_off > image->rows
           || cols == 0 || rows == 0)
    {
        rb_raise(rb_eArgError, "invalid extract geometry");
    }


    npixels = (long)(cols * rows * strlen(map));
    pixels = ALLOC_N(Quantum, npixels);
    if (!pixels)    // app recovered from exception
    {
        return rb_ary_new2(0L);
    }

    GetExceptionInfo(&exception);

    okay = ExportImagePixels(image, x_off, y_off, cols, rows, map, QuantumPixel, (void *)pixels, &exception);
    if (!okay)
    {
        xfree((void *)pixels);
        CHECK_EXCEPTION()

        // Should never get here...
        rm_magick_error("ExportImagePixels failed with no explanation.", NULL);
    }

    (void) DestroyExceptionInfo(&exception);

    ary = rb_ary_new2(npixels);
    for (n = 0; n < npixels; n++)
    {
        (void) rb_ary_push(ary, QUANTUM2NUM(pixels[n]));
    }

    xfree((void *)pixels);

    return ary;
}


/**
 * Call ExtentImage.
 *
 * Ruby usage:
 *   - @verbatim Image#extent(width, height) @endverbatim
 *   - @verbatim Image#extent(width, height, x) @endverbatim
 *   - @verbatim Image#extent(width, height, x, y) @endverbatim
 *
 * Notes:
 *   - Default x is 0
 *   - Default y is 0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_extent(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    RectangleInfo geometry;
    long height, width;
    ExceptionInfo exception;

    (void) rm_check_destroyed(self);

    if (argc < 2 || argc > 4)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (expected 2 to 4, got %d)", argc);
    }

    geometry.y = geometry.x = 0L;
    switch (argc)
    {
        case 4:
            geometry.y = NUM2LONG(argv[3]);
        case 3:
            geometry.x = NUM2LONG(argv[2]);
        default:
            geometry.height = height = NUM2LONG(argv[1]);
            geometry.width = width = NUM2LONG(argv[0]);
            break;
    }

    // Use the signed versions of these two values to test for < 0
    if (height <= 0L || width <= 0L)
    {
        if (geometry.x == 0 && geometry.y == 0)
        {
            rb_raise(rb_eArgError, "invalid extent geometry %ldx%ld", width, height);
        }
        else
        {
            rb_raise(rb_eArgError, "invalid extent geometry %ldx%ld+%ld+%ld"
                     , width, height, geometry.x, geometry.y);
        }
    }


    Data_Get_Struct(self, Image, image);
    GetExceptionInfo(&exception);

    new_image = ExtentImage(image, &geometry, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);
    return rm_image_new(new_image);
}


/**
 * Extract image pixels to a Ruby string.
 *
 * Ruby usage:
 *   - @verbatim Image#export_pixels_to_str @endverbatim
 *   - @verbatim Image#export_pixels_to_str(x) @endverbatim
 *   - @verbatim Image#export_pixels_to_str(x, y) @endverbatim
 *   - @verbatim Image#export_pixels_to_str(x, y, cols) @endverbatim
 *   - @verbatim Image#export_pixels_to_str(x, y, cols, rows) @endverbatim
 *   - @verbatim Image#export_pixels_to_str(x, y, cols, rows, map) @endverbatim
 *   - @verbatim Image#export_pixels_to_str(x, y, cols, rows, map, type) @endverbatim
 *
 * Notes:
 *   - Default x is 0
 *   - Default y is 0
 *   - Default cols is self.columns
 *   - Default rows is self.rows
 *   - Default map is "RGB"
 *   - Default type is Magick::CharPixel
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return pixels as a string
 */
VALUE
Image_export_pixels_to_str(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    long x_off = 0L, y_off = 0L;
    unsigned long cols, rows;
    unsigned long npixels;
    size_t sz;
    unsigned int okay;
    const char *map = "RGB";
    StorageType type = CharPixel;
    volatile VALUE string;
    char *str;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    cols = image->columns;
    rows = image->rows;

    switch (argc)
    {
        case 6:
            VALUE_TO_ENUM(argv[5], type, StorageType);
        case 5:
            map   = StringValuePtr(argv[4]);
        case 4:
            rows  = NUM2ULONG(argv[3]);
        case 3:
            cols  = NUM2ULONG(argv[2]);
        case 2:
            y_off = NUM2LONG(argv[1]);
        case 1:
            x_off = NUM2LONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 6)", argc);
            break;
    }

    if (   x_off < 0 || (unsigned long)x_off > image->columns
           || y_off < 0 || (unsigned long)y_off > image->rows
           || cols == 0 || rows == 0)
    {
        rb_raise(rb_eArgError, "invalid extract geometry");
    }


    npixels = cols * rows * strlen(map);
    switch (type)
    {
        case CharPixel:
            sz = sizeof(unsigned char);
            break;
        case ShortPixel:
            sz = sizeof(unsigned short);
            break;
        case DoublePixel:
            sz = sizeof(double);
            break;
        case FloatPixel:
            sz = sizeof(float);
            break;
        case IntegerPixel:
            sz = sizeof(unsigned int);
            break;
        case LongPixel:
            sz = sizeof(unsigned long);
            break;
        case QuantumPixel:
            sz = sizeof(Quantum);
            break;
        case UndefinedPixel:
        default:
            rb_raise(rb_eArgError, "undefined storage type");
            break;
    }

    // Allocate a string long enough to hold the exported pixel data.
    // Get a pointer to the buffer.
    string = rb_str_new2("");
    (void) rb_str_resize(string, (long)(sz * npixels));
    str = StringValuePtr(string);

    GetExceptionInfo(&exception);

    okay = ExportImagePixels(image, x_off, y_off, cols, rows, map, type, (void *)str, &exception);
    if (!okay)
    {
        // Let GC have the string buffer.
        (void) rb_str_resize(string, 0);
        CHECK_EXCEPTION()

        // Should never get here...
        rm_magick_error("ExportImagePixels failed with no explanation.", NULL);
    }

    (void) DestroyExceptionInfo(&exception);

    return string;
}


/**
 * The extract_info attribute reader.
 *
 * Ruby usage:
 *   - @verbatim Image#extract_info @endverbatim
 *
 * @param self this object
 * @return extract_info
 */
VALUE
Image_extract_info(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return Import_RectangleInfo(&image->extract_info);
}


/**
 * The extract_info attribute reader.
 *
 * Ruby usage:
 *   - @verbatim Image#extract_info= @endverbatim
 *
 * @param self this object
 * @param rect extract_info
 * @return self
 */
VALUE
Image_extract_info_eq(VALUE self, VALUE rect)
{
    Image *image = rm_check_frozen(self);
    Export_RectangleInfo(&image->extract_info, rect);
    return self;
}


/**
 * Get image filename.
 *
 * Ruby usage:
 *   - @verbatim Image#filename @endverbatim
 *
 * @param self this object
 * @return the filename
 */
DEF_ATTR_READER(Image, filename, str)


/**
 * Return the image filesize.
 *
 * Ruby usage:
 *   - @verbatim Image#filesize @endverbatim
 *
 * @param self this object
 * @return the filesize
 */
VALUE Image_filesize(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return INT2FIX(GetBlobSize(image));
}


/**
 * Get filter type.
 *
 * Ruby usage:
 *   - @verbatim Image#filter @endverbatim
 *
 * @param self this object
 * @return the filter
 */
VALUE
Image_filter(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return FilterTypes_new(image->filter);
}


/**
 * Set filter type.
 *
 * Ruby usage:
 *   - @verbatim Image#filter= @endverbatim
 *
 * @param self this object
 * @param filter the filter
 * @return self
 */
VALUE
Image_filter_eq(VALUE self, VALUE filter)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(filter, image->filter, FilterTypes);
    return self;
}


/**
 * Search for a region in the image that is "similar" to the target image.
 *
 * Ruby usage:
 *   - @verbatim Image#find_similar_region(target) @endverbatim
 *   - @verbatim Image#find_similar_region(target, x) @endverbatim
 *   - @verbatim Image#find_similar_region(target, x, y) @endverbatim
 *
 * Notes:
 *   - Default x is 0
 *   - Default y is 0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the region
 */
VALUE
Image_find_similar_region(int argc, VALUE *argv, VALUE self)
{
    Image *image, *target;
    volatile VALUE region, targ;
    long x = 0L, y = 0L;
    ExceptionInfo exception;
    unsigned int okay;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 3:
            y = NUM2LONG(argv[2]);
        case 2:
            x = NUM2LONG(argv[1]);
        case 1:
            targ = rm_cur_image(argv[0]);
            target = rm_check_destroyed(targ);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 3)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    okay = IsImageSimilar(image, target, &x, &y, &exception);
    CHECK_EXCEPTION();
    (void) DestroyExceptionInfo(&exception);

    if (!okay)
    {
        return Qnil;
    }

    region = rb_ary_new2(2);
    rb_ary_store(region, 0L, LONG2NUM(x));
    rb_ary_store(region, 1L, LONG2NUM(y));

    return region;
}


/**
 * Call a flipflopper (a function that either flips or flops the image).
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @param flipflopper the flip/flop method to call
 * @return self if bang, otherwise a new image
 * @see Image_flip
 * @see Image_flip_bang
 * @see Image_flop
 * @see Image_flop_bang
 */
static VALUE
flipflop(int bang, VALUE self, flipper_t flipflopper)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);
    GetExceptionInfo(&exception);

    new_image = (flipflopper)(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);
}


/**
 * Create a vertical mirror image by reflecting the pixels around the central
 * x-axis.
 *
 * Ruby usage:
 *   - @verbatim Image#flip @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see flipflop
 * @see Image_flip_bang
 * @see Image_flop
 * @see Image_flop_bang
 */
VALUE
Image_flip(VALUE self)
{
    (void) rm_check_destroyed(self);
    return flipflop(False, self, FlipImage);
}


/**
 * Create a vertical mirror image by reflecting the pixels around the central
 * x-axis.
 *
 * Ruby usage:
 *   - @verbatim Image#flip! @endverbatim
 *
 * @param self this object
 * @return self
 * @see flipflop
 * @see Image_flip
 * @see Image_flop
 * @see Image_flop_bang
 */
VALUE
Image_flip_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return flipflop(True, self, FlipImage);
}


/**
 * Create a horizonal mirror image by reflecting the pixels around the central
 * y-axis.
 *
 * Ruby usage:
 *   - @verbatim Image#flop @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see flipflop
 * @see Image_flop_bang
 * @see Image_flip
 * @see Image_flip_bang
 */
VALUE
Image_flop(VALUE self)
{
    (void) rm_check_destroyed(self);
    return flipflop(False, self, FlopImage);
}


/**
 * Create a horizonal mirror image by reflecting the pixels around the central
 * y-axis.
 *
 * Ruby usage:
 *   - @verbatim Image#flop! @endverbatim
 *
 * @param self this object
 * @return self
 * @see flipflop
 * @see Image_flop
 * @see Image_flip
 * @see Image_flip_bang
 */
VALUE
Image_flop_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return flipflop(True, self, FlopImage);
}


/**
 * Return the image encoding format.
 *
 * Ruby usage:
 *   - @verbatim Image#format @endverbatim
 *
 * Notes:
 *   - This is what PerlMagick does for "format".
 *
 * @param self this object
 * @return the encoding format
 */
VALUE
Image_format(VALUE self)
{
    Image *image;
    const MagickInfo *magick_info;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    if (*image->magick)
    {
        // Deliberately ignore the exception info!
        GetExceptionInfo(&exception);
        magick_info = GetMagickInfo(image->magick, &exception);
        (void) DestroyExceptionInfo(&exception);
        return magick_info ? rb_str_new2(magick_info->name) : Qnil;
    }

    return Qnil;
}


/**
 * Set the image encoding format.
 *
 * Ruby usage:
 *   - @verbatim Image#format= @endverbatim
 *
 * @param self this object
 * @param magick the encoding format
 * @return self
 */
VALUE
Image_format_eq(VALUE self, VALUE magick)
{
    Image *image;
    const MagickInfo *m;
    char *mgk;
    ExceptionInfo exception;

    image = rm_check_frozen(self);

    GetExceptionInfo(&exception);

    mgk = StringValuePtr(magick);
    m = GetMagickInfo(mgk, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    if (!m)
    {
        rb_raise(rb_eArgError, "unknown format: %s", mgk);
    }


    strncpy(image->magick, m->name, MaxTextExtent-1);
    return self;
}


/**
 * Add a simulated three-dimensional border around the image. "Width" and
 * "height" specify the width and height of the frame. The "x" and "y" arguments
 * position the image within the frame. If the image is supposed to be centered
 * in the frame, x and y should be 1/2 the width and height of the frame. (I.e.,
 * if the frame is 50 pixels high and 50 pixels wide, x and y should both be
 * 25). "Inner_bevel" and "outer_bevel" indicate the width of the inner and
 * outer shadows of the frame. They should be much smaller than the frame and
 * cannot be > 1/2 the frame width or height of the image.
 *
 * Ruby usage:
 *   - @verbatim Image#frame @endverbatim
 *   - @verbatim Image#frame(width) @endverbatim
 *   - @verbatim Image#frame(width, height) @endverbatim
 *   - @verbatim Image#frame(width, height, x) @endverbatim
 *   - @verbatim Image#frame(width, height, x, y) @endverbatim
 *   - @verbatim Image#frame(width, height, x, y, inner_bevel) @endverbatim
 *   - @verbatim Image#frame(width, height, x, y, inner_bevel, outer_bevel) @endverbatim
 *   - @verbatim Image#frame(width, height, x, y, inner_bevel, outer_bevel, color) @endverbatim
 *
 * Notes:
 *   - The defaults are the same as they are in Magick++
 *   - Default width is image-columns+25*2
 *   - Default height is image-rows+25*2
 *   - Default x is 25
 *   - Default y is 25
 *   - Default inner is 6
 *   - Default outer is 6
 *   - Default color is image matte_color (which defaults to "#bdbdbd", whatever
 *     self.matte_color was set to when the image was created, or whatever
 *     image.matte_color is currently set to)
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image.
 */
VALUE
Image_frame(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;
    FrameInfo frame_info;

    image = rm_check_destroyed(self);

    frame_info.width = image->columns + 50;
    frame_info.height = image->rows + 50;
    frame_info.x = 25;
    frame_info.y = 25;
    frame_info.inner_bevel = 6;
    frame_info.outer_bevel = 6;

    switch (argc)
    {
        case 7:
            Color_to_PixelPacket(&image->matte_color, argv[6]);
        case 6:
            frame_info.outer_bevel = NUM2LONG(argv[5]);
        case 5:
            frame_info.inner_bevel = NUM2LONG(argv[4]);
        case 4:
            frame_info.y = NUM2LONG(argv[3]);
        case 3:
            frame_info.x = NUM2LONG(argv[2]);
        case 2:
            frame_info.height = image->rows + 2*NUM2LONG(argv[1]);
        case 1:
            frame_info.width = image->columns + 2*NUM2LONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 7)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = FrameImage(image, &frame_info, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call BlobToImage.
 *
 * Ruby usage:
 *   - @verbatim Image.from_blob(blob) <{ parm block }> @endverbatim
 *
 * @param class the Ruby Image class (unused)
 * @param blob_arg the blog as a Ruby string
 * @return an array of new images
 */
VALUE
Image_from_blob(VALUE class, VALUE blob_arg)
{
    Image *images;
    Info *info;
    volatile VALUE info_obj;
    ExceptionInfo exception;
    void *blob;
    long length;

    class = class;          // defeat gcc message
            blob_arg = blob_arg;    // defeat gcc message

    blob = (void *) rm_str2cstr(blob_arg, &length);

    // Get a new Info object - run the parm block if supplied
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    GetExceptionInfo(&exception);
    images = BlobToImage(info,  blob, (size_t)length, &exception);
    rm_check_exception(&exception, images, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(images);
    rm_set_user_artifact(images, info);

    return array_from_images(images);
}


/**
 * Set the function on a channel.
 *
 * Ruby usage:
 *   - @verbatim Image#function_channel(function, args) @endverbatim
 *   - @verbatim Image#function_channel(function, args, channel) @endverbatim
 *   - @verbatim Image#function_channel(function, args, channel, ...) @endverbatim
 *
 *  Notes:
 *    - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_function_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_FUNCTIONIMAGECHANNEL)
    Image *image, *new_image;
    MagickFunction function;
    unsigned long n, nparms;
    volatile double *parameters;
    double *parms;
    ChannelType channels;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // The number of parameters depends on the function.
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "no function specified");
    }

    VALUE_TO_ENUM(argv[0], function, MagickFunction);
    argc -= 1;
    argv += 1;

    switch (function)
    {
#if defined(HAVE_ENUM_POLYNOMIALFUNCTION)
        case PolynomialFunction:
            if (argc == 0)
            {
                rb_raise(rb_eArgError, "PolynomialFunction requires at least one argument.");
            }
            break;
#endif
#if defined(HAVE_ENUM_SINUSOIDFUNCTION)
        case SinusoidFunction:
#endif
#if defined(HAVE_ENUM_ARCSINFUNCTION)
        case ArcsinFunction:
#endif
#if defined(HAVE_ENUM_ARCTANFUNCTION)
        case ArctanFunction:
#endif
           if (argc < 1 || argc > 4)
           {
               rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 4)", argc);
           }
           break;
        default:
            rb_raise(rb_eArgError, "undefined function");
            break;
    }

    nparms = argc;
    parameters = parms = ALLOC_N(double, nparms);

    for (n = 0; n < nparms; n++)
    {
        parms[n] = NUM2DBL(argv[n]);
    }

    GetExceptionInfo(&exception);
    new_image = rm_clone_image(image);
    (void) FunctionImageChannel(new_image, channels, function, nparms, parms, &exception);
    (void) xfree(parms);
    rm_check_exception(&exception, new_image, DestroyOnError);
    DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
#else
    rm_not_implemented();
    return (VALUE)0;
    argc = argc;
    argv = argv;
    self = self;
#endif
}


/**
 * Get image fuzz.
 *
 * Ruby usage:
 *   - @verbatim Image#fuzz @endverbatim
 *
 * @param self this object
 * @return the fuzz
 * @see Info_fuzz
 */
DEF_ATTR_READER(Image, fuzz, dbl)


/**
 * Set image fuzz.
 *
 * Ruby usage:
 *   - @verbatim Image#fuzz=number @endverbatim
 *   - @verbatim Image#fuzz=NN% @endverbatim
 *
 * @param self this object
 * @param fuzz the fuzz
 * @return self
 * @see Info_fuzz_eq
 */
VALUE
Image_fuzz_eq(VALUE self, VALUE fuzz)
{
    Image *image = rm_check_frozen(self);
    image->fuzz = rm_fuzz_to_dbl(fuzz);
    return self;
}


DEF_ATTR_ACCESSOR(Image, gamma, dbl)


/**
 * Apply gamma to a channel.
 *
 * Ruby usage:
 *   - @verbatim Image#gamma_channel(gamma) @endverbatim
 *   - @verbatim Image#gamma_channel(gamma, channel) @endverbatim
 *   - @verbatim Image#gamma_channel(gamma, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_gamma_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There must be exactly one remaining argument.
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "missing gamma argument");
    }
    else if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    new_image = rm_clone_image(image);

    (void)GammaImageChannel(new_image, channels, NUM2DBL(argv[0]));
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * gamma-correct an image.
 *
 * Ruby usage:
 *   - @verbatim Image#gamma_correct(red_gamma) @endverbatim
 *   - @verbatim Image#gamma_correct(red_gamma, green_gamma) @endverbatim
 *   - @verbatim Image#gamma_correct(red_gamma, green_gamma, blue_gamma) @endverbatim
 *
 * Notes:
 *   - Default green_gamma is red_gamma
 *   - Default blue_gamma is green_gamma
 *   - For backward compatibility accept a 4th argument but ignore it.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_gamma_correct(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double red_gamma, green_gamma, blue_gamma;
    char gamma_arg[50];

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:
            red_gamma   = NUM2DBL(argv[0]);

            // Can't have all 4 gamma values == 1.0. Also, very small values
            // cause ImageMagick to segv.
            if (red_gamma == 1.0 || fabs(red_gamma) < 0.003)
            {
                rb_raise(rb_eArgError, "invalid gamma value (%f)", red_gamma);
            }
            green_gamma = blue_gamma = red_gamma;
            break;
        case 2:
            red_gamma   = NUM2DBL(argv[0]);
            green_gamma = NUM2DBL(argv[1]);
            blue_gamma  = green_gamma;
            break;
        case 3:
        case 4:
            red_gamma     = NUM2DBL(argv[0]);
            green_gamma   = NUM2DBL(argv[1]);
            blue_gamma    = NUM2DBL(argv[2]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 3)", argc);
            break;
    }

    sprintf(gamma_arg, "%f,%f,%f", red_gamma, green_gamma, blue_gamma);

    new_image = rm_clone_image(image);

    (void) GammaImage(new_image, gamma_arg);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Blur the image.
 *
 * Ruby usage:
 *   - @verbatim Image#gaussian_blur @endverbatim
 *   - @verbatim Image#gaussian_blur(radius) @endverbatim
 *   - @verbatim Image#gaussian_blur(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see effect_image
 */
VALUE
Image_gaussian_blur(int argc, VALUE *argv, VALUE self)
{
    return effect_image(self, argc, argv, GaussianBlurImage);
}


/**
 * Blur the image on a channel.
 * Ruby usage:
 *   - @verbatim Image#gaussian_blur_channel @endverbatim
 *   - @verbatim Image#gaussian_blur_channel(radius) @endverbatim
 *   - @verbatim Image#gaussian_blur_channel(radius, sigma) @endverbatim
 *   - @verbatim Image#gaussian_blur_channel(radius, sigma, channel) @endverbatim
 *   - @verbatim Image#gaussian_blur_channel(radius, sigma, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default channel is AllChannels
 *   - New in IM 6.0.0
 *
 *
 */
VALUE
Image_gaussian_blur_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;
    ExceptionInfo exception;
    double radius = 0.0, sigma = 1.0;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There can be 0, 1, or 2 remaining arguments.
    switch (argc)
    {
        case 2:
            sigma = NUM2DBL(argv[1]);
            /* Fall thru */
        case 1:
            radius = NUM2DBL(argv[0]);
            /* Fall thru */
        case 0:
            break;
        default:
            raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);
    new_image = GaussianBlurImageChannel(image, channels, radius, sigma, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Get the preferred size of the image when encoding.
 *
 * Ruby usage:
 *   - @verbatim Image#geometry @endverbatim
 *
 * @param self this object
 * @return the geometry
 */
DEF_ATTR_READER(Image, geometry, str)


/**
 * Set the preferred size of the image when encoding.
 *
 * Ruby usage:
 *   - @verbatim Image#geometry= @endverbatim
 *
 * @param self this object
 * @param geometry the geometry
 * @return self
 */
VALUE
Image_geometry_eq(
                 VALUE self,
                 VALUE geometry)
{
    Image *image;
    volatile VALUE geom_str;
    char *geom;

    image = rm_check_frozen(self);

    if (geometry == Qnil)
    {
        magick_free(image->geometry);
        image->geometry = NULL;
        return self;
    }


    geom_str = rm_to_s(geometry);
    geom = StringValuePtr(geom_str);
    if (!IsGeometry(geom))
    {
        rb_raise(rb_eTypeError, "invalid geometry: %s", geom);
    }
    magick_clone_string(&image->geometry, geom);
    return self;
}


/**
 * Call AcquireImagePixels.
 *
 * Ruby usage:
 *   - @verbatim Image#get_pixels(x, y, columns. rows) @endverbatim
 *
 * Notes:
 *   - This is the complement of store_pixels. Notice that the return value is
 *     an array object even when only one pixel is returned. store_pixels calls
 *     GetImagePixels, then SyncImage
 *
 * @param self this object
 * @param x_arg x position of start of region
 * @param y_arg y position of start of region
 * @param cols_arg width of region
 * @param rows_arg height of region
 * @return An array of Magick::Pixel objects corresponding to the pixels in the
 * rectangle defined by the geometry parameters.
 * @see Image_store_pixels
 */
VALUE
Image_get_pixels(VALUE self, VALUE x_arg, VALUE y_arg, VALUE cols_arg, VALUE rows_arg)
{
    Image *image;
    const PixelPacket *pixels;
    ExceptionInfo exception;
    long x, y;
    unsigned long columns, rows;
    long size, n;
    VALUE pixel_ary;

    image = rm_check_destroyed(self);
    x       = NUM2LONG(x_arg);
    y       = NUM2LONG(y_arg);
    columns = NUM2ULONG(cols_arg);
    rows    = NUM2ULONG(rows_arg);

    if ((x+columns) > image->columns || (y+rows) > image->rows)
    {
        rb_raise(rb_eRangeError, "geometry (%lux%lu%+ld%+ld) exceeds image bounds"
                 , columns, rows, x, y);
    }

    // Cast AcquireImagePixels to get rid of the const qualifier. We're not going
    // to change the pixels but I don't want to make "pixels" const.
    GetExceptionInfo(&exception);
#if defined(HAVE_GETVIRTUALPIXELS)
    pixels = GetVirtualPixels(image, x, y, columns, rows, &exception);
#else
    pixels = AcquireImagePixels(image, x, y, columns, rows, &exception);
#endif
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    // If the function failed, return a 0-length array.
    if (!pixels)
    {
        return rb_ary_new();
    }

    // Allocate an array big enough to contain the PixelPackets.
    size = (long)(columns * rows);
    pixel_ary = rb_ary_new2(size);

    // Convert the PixelPackets to Magick::Pixel objects
    for (n = 0; n < size; n++)
    {
        rb_ary_store(pixel_ary, n, Pixel_from_PixelPacket(&pixels[n]));
    }

    return pixel_ary;
}


/**
 * Run a function testing whether this image has an attribute.
 *
 * No Ruby usage (internal function)
 *
 * @param self this object
 * @param attr_test the attribute testing function
 * @return the result of attr_test.
 */
static VALUE
has_attribute(VALUE self, MagickBooleanType (attr_test)(const Image *, ExceptionInfo *))
{
    Image *image;
    ExceptionInfo exception;
    MagickBooleanType r;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    r = (attr_test)(image, &exception);
    CHECK_EXCEPTION()

    return r ? Qtrue : Qfalse;
}


/**
 * Return true if all the pixels in the image have the same red, green, and blue
 * intensities.
 *
 * Ruby usage:
 *   - @verbatim Image#gray? @endverbatim
 *
 * @param self this object
 * @return true if image is gray, false otherwise
 * @see has_attribute
 */
VALUE
Image_gray_q(VALUE self)
{
    return has_attribute(self, (MagickBooleanType (*)(const Image *, ExceptionInfo *))IsGrayImage);
}


/**
 * Return true if has 1024 unique colors or less.
 *
 * Ruby usage:
 *   - @verbatim Image#histogram? @endverbatim
 *
 * @param self this object
 * @return true if image has <=1024 unique colors
 * @see has_attribute
 */
VALUE
Image_histogram_q(VALUE self)
{
    return has_attribute(self, IsHistogramImage);
}


/**
 * Implode the image by the specified percentage.
 *
 * Ruby usage:
 *   - @verbatim Image#implode @endverbatim
 *   - @verbatim Image#implode(amount) @endverbatim
 *
 * Notes:
 *   - Default amount is 0.50
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_implode(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double amount = 0.50;
    ExceptionInfo exception;

    switch (argc)
    {
        case 1:
            amount = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    new_image = ImplodeImage(image, amount, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Store image pixel data from an array.
 *
 * Ruby usage:
 *   - @verbatim Image#import_pixels @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see Image_export_pixels
 */
VALUE
Image_import_pixels(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    long x_off, y_off;
    unsigned long cols, rows;
    unsigned long n, npixels;
    long buffer_l;
    char *map;
    volatile VALUE pixel_arg, pixel_ary;
    StorageType stg_type = CharPixel;
    size_t type_sz, map_l;
    Quantum *pixels = NULL;
    double *fpixels = NULL;
    void *buffer;
    unsigned int okay;

    image = rm_check_frozen(self);

    switch (argc)
    {
        case 7:
            VALUE_TO_ENUM(argv[6], stg_type, StorageType);
        case 6:
            x_off = NUM2LONG(argv[0]);
            y_off = NUM2LONG(argv[1]);
            cols = NUM2ULONG(argv[2]);
            rows = NUM2ULONG(argv[3]);
            map = StringValuePtr(argv[4]);
            pixel_arg = argv[5];
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 6 or 7)", argc);
            break;
    }

    if (x_off < 0 || y_off < 0 || cols <= 0 || rows <= 0)
    {
        rb_raise(rb_eArgError, "invalid import geometry");
    }

    map_l = strlen(map);
    npixels = cols * rows * map_l;

    // Assume that any object that responds to :to_str is a string buffer containing
    // binary pixel data.
    if (rb_respond_to(pixel_arg, rb_intern("to_str")))
    {
        buffer = (void *)rm_str2cstr(pixel_arg, &buffer_l);
        switch (stg_type)
        {
            case CharPixel:
                type_sz = 1;
                break;
            case ShortPixel:
                type_sz = sizeof(unsigned short);
                break;
            case IntegerPixel:
                type_sz = sizeof(unsigned int);
                break;
            case LongPixel:
                type_sz = sizeof(unsigned long);
                break;
            case DoublePixel:
                type_sz = sizeof(double);
                break;
            case FloatPixel:
                type_sz = sizeof(float);
                break;
            case QuantumPixel:
                type_sz = sizeof(Quantum);
                break;
            default:
                rb_raise(rb_eArgError, "unsupported storage type %s", StorageType_name(stg_type));
                break;
        }

        if (buffer_l % type_sz != 0)
        {
            rb_raise(rb_eArgError, "pixel buffer must be an exact multiple of the storage type size");
        }
        if ((buffer_l / type_sz) % map_l != 0)
        {
            rb_raise(rb_eArgError, "pixel buffer must contain an exact multiple of the map length");
        }
        if ((unsigned long)(buffer_l / type_sz) < npixels)
        {
            rb_raise(rb_eArgError, "pixel buffer too small (need %lu channel values, got %ld)"
                     , npixels, buffer_l/type_sz);
        }
    }
    // Otherwise convert the argument to an array and convert the array elements
    // to binary pixel data.
    else
    {
        // rb_Array converts an object that is not an array to an array if possible,
        // and raises TypeError if it can't. It usually is possible.
        pixel_ary = rb_Array(pixel_arg);

        if (RARRAY_LEN(pixel_ary) % map_l != 0)
        {
            rb_raise(rb_eArgError, "pixel array must contain an exact multiple of the map length");
        }
        if ((unsigned long)RARRAY_LEN(pixel_ary) < npixels)
        {
            rb_raise(rb_eArgError, "pixel array too small (need %lu elements, got %ld)"
                     , npixels, RARRAY_LEN(pixel_ary));
        }

        if (stg_type == DoublePixel || stg_type == FloatPixel)
        {
            // Get an array for double pixels. Use Ruby's memory so GC will clean up after
            // us in case of an exception.
            fpixels = ALLOC_N(double, npixels);
            for (n = 0; n < npixels; n++)
            {
                fpixels[n] = NUM2DBL(rb_ary_entry(pixel_ary, n));
            }
            buffer = (void *) fpixels;
            stg_type = DoublePixel;
        }
        else
        {
            // Get array for Quantum pixels. Use Ruby's memory so GC will clean up after us
            // in case of an exception.
            pixels = ALLOC_N(Quantum, npixels);
            for (n = 0; n < npixels; n++)
            {
                volatile VALUE p = rb_ary_entry(pixel_ary, n);
                pixels[n] = NUM2QUANTUM(p);
            }
            buffer = (void *) pixels;
            stg_type = QuantumPixel;
        }
    }


    okay = ImportImagePixels(image, x_off, y_off, cols, rows, map, stg_type, buffer);

    // Free pixel array before checking for errors.
    if (pixels)
    {
        xfree((void *)pixels);
    }
    if (fpixels)
    {
        xfree((void *)fpixels);
    }

    if (!okay)
    {
        rm_check_image_exception(image, RetainOnError);
        // Shouldn't get here...
        rm_magick_error("ImportImagePixels failed with no explanation.", NULL);
    }

    return self;
}


/**
 * Override Object#inspect - return a string description of the image.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - This is essentially the IdentifyImage except the description is built in
 *     a char buffer instead of being written to a file.
 *
 * @param image the image to inspect
 * @param buffer buffer for the output string
 * @param len length of buffer
 * @see Image_inspect
 */
static void
build_inspect_string(Image *image, char *buffer, size_t len)
{
    unsigned long quantum_depth;
    int x = 0;                  // # bytes used in buffer

    // Print magick filename if different from current filename.
    if (*image->magick_filename != '\0' && strcmp(image->magick_filename, image->filename) != 0)
    {
        x += sprintf(buffer+x, "%.1024s=>", image->magick_filename);
    }
    // Print current filename.
    x += sprintf(buffer+x, "%.1024s", image->filename);
    // Print scene number.
    if ((GetPreviousImageInList(image) != NULL) && (GetNextImageInList(image) != NULL) && image->scene > 0)
    {
        x += sprintf(buffer+x, "[%lu]", image->scene);
    }
    // Print format
    x += sprintf(buffer+x, " %s ", image->magick);

    // Print magick columnsXrows if different from current.
    if (image->magick_columns != 0 || image->magick_rows != 0)
    {
        if (image->magick_columns != image->columns || image->magick_rows != image->rows)
        {
            x += sprintf(buffer+x, "%lux%lu=>", image->magick_columns, image->magick_rows);
        }
    }

    x += sprintf(buffer+x, "%lux%lu ", image->columns, image->rows);

    // Print current columnsXrows
    if (   image->page.width != 0 || image->page.height != 0
           || image->page.x != 0     || image->page.y != 0)
    {
        x += sprintf(buffer+x, "%lux%lu%+ld%+ld ", image->page.width, image->page.height
                     , image->page.x, image->page.y);
    }

    if (image->storage_class == DirectClass)
    {
        x += sprintf(buffer+x, "DirectClass ");
        if (image->total_colors != 0)
        {
            if (image->total_colors >= (unsigned long)(1 << 24))
            {
                x += sprintf(buffer+x, "%lumc ", image->total_colors/1024/1024);
            }
            else
            {
                if (image->total_colors >= (unsigned long)(1 << 16))
                {
                    x += sprintf(buffer+x, "%lukc ", image->total_colors/1024);
                }
                else
                {
                    x += sprintf(buffer+x, "%luc ", image->total_colors);
                }
            }
        }
    }
    else
    {
        // Cast `image->colors' to long to suppress gcc warnings when
        // building with GM. GM defines that field as an unsigned int.
        if (image->total_colors <= image->colors)
        {
            x += sprintf(buffer+x, "PseudoClass %ldc ", (long) image->colors);
        }
        else
        {
            x += sprintf(buffer+x, "PseudoClass %lu=>%ldc ", image->total_colors
                         , (long)image->colors);
            if (image->error.mean_error_per_pixel != 0.0)
            {
                x += sprintf(buffer+x, "%ld/%.6f/%.6fdb "
                             , (long) (image->error.mean_error_per_pixel+0.5)
                             , image->error.normalized_mean_error
                             , image->error.normalized_maximum_error);
            }
        }
    }

    // Print bit depth
    quantum_depth = GetImageQuantumDepth(image, MagickTrue);
    x += sprintf(buffer+x, "%lu-bit", quantum_depth);

    // Print blob info if appropriate.
    if (GetBlobSize(image) != 0)
    {
        if (GetBlobSize(image) >= (1 << 24))
        {
            x += sprintf(buffer+x, " %lumb", (unsigned long) (GetBlobSize(image)/1024/1024));
        }
        else if (GetBlobSize(image) >= 1024)
        {
            x += sprintf(buffer+x, " %lukb", (unsigned long) (GetBlobSize(image)/1024));
        }
        else
        {
            x += sprintf(buffer+x, " %lub", (unsigned long) GetBlobSize(image));
        }
    }


#if defined(HAVE_SETIMAGEARTIFACT)
    if (len-1-x > 6)
    {
        size_t value_l;
        const char *value = GetImageArtifact(image, "user");
        if (value)
        {
            strcpy(buffer+x, " user:");
            x += 6;
            value_l = len - x - 1;
            value_l = min(strlen(value), value_l);
            memcpy(buffer+x, value, value_l);
            x += value_l;
        }
    }
#endif

    assert(x < (int)(len-1));
    buffer[x] = '\0';

    return;
}


/**
 * Override Object#inspect - return a string description of the image.
 *
 * Ruby usage:
 *   - @verbatim Image#inspect @endverbatim
 *
 * Notes:
 *   - This is essentially the IdentifyImage except the description is built in
 *     a char buffer instead of being written to a file.
 *
 * @param self this object
 * @return the string
 * @see build_inspect_string
 */
VALUE
Image_inspect(VALUE self)
{
    Image *image;
    char buffer[MaxTextExtent];          // image description buffer

    Data_Get_Struct(self, Image, image);
    if (!image)
    {
        return rb_str_new2("#<Magick::Image: (destroyed)>");
    }
    build_inspect_string(image, buffer, sizeof(buffer));
    return rb_str_new2(buffer);
}


/**
 * Get the interlace attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#interlace @endverbatim
 *
 * @param self this object
 * @return the interlace
 */
VALUE
Image_interlace(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return InterlaceType_new(image->interlace);
}


/**
 * Set the interlace attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#interlace= @endverbatim
 *
 * @param self this object
 * @param interlace the interlace
 * @return self
 */
VALUE
Image_interlace_eq(VALUE self, VALUE interlace)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(interlace, image->interlace, InterlaceType);
    return self;
}


/**
 * Return the IPTC profile as a String.
 *
 * Ruby usage:
 *   - @verbatim Image#iptc_profile @endverbatim
 *
 * @param self
 * @return the IPTC profile if it exists, otherwise nil
 */
VALUE
Image_iptc_profile(VALUE self)
{
    Image *image;
    const StringInfo *profile;

    image = rm_check_destroyed(self);
    profile = GetImageProfile(image, "iptc");
    rm_check_image_exception(image, RetainOnError);
    if (!profile)
    {
        return Qnil;
    }

    return rb_str_new((char *)profile->datum, (long)profile->length);

}



/**
 * Set the IPTC profile. The argument is a string.
 *
 * Ruby usage:
 *   - @verbatim Image#iptc_profile= @endverbatim
 *
 * Notes:
 *   - Pass nil to remove any existing profile
 *
 * @param self
 * @param profile the IPTC profile (as a string)
 * @return self
 */
VALUE
Image_iptc_profile_eq(VALUE self, VALUE profile)
{
    (void) Image_delete_profile(self, rb_str_new2("IPTC"));
    if (profile != Qnil)
    {
        (void) set_profile(self, "IPTC", profile);
    }
    return self;
}


/*
 *  These are undocumented methods. The writer is
 *  called only by Image#iterations=.
 *  The reader is only used by the unit tests!
 */
DEF_ATTR_ACCESSOR(Image, iterations, int)

/**
 * Adjust the levels of an image given these points: black, mid, and white.
 *
 * Ruby usage:
 *   - @verbatim Image#level @endverbatim
 *   - @verbatim Image#level(black_point) @endverbatim
 *   - @verbatim Image#level(black_point, white_point) @endverbatim
 *   - @verbatim Image#level(black_point, white_point, gamma) @endverbatim
 *
 * Notes:
 *   - Default black_point is 0.0
 *   - Default white_point is QuantumRange
 *   - Default gamma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_level2(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double black_point = 0.0, gamma_val = 1.0, white_point = (double)QuantumRange;
    char level[50];

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 0:             // take all the defaults
            break;
        case 1:
            black_point = NUM2DBL(argv[0]);
            white_point = QuantumRange - black_point;
            break;
        case 2:
            black_point = NUM2DBL(argv[0]);
            white_point = NUM2DBL(argv[1]);
            break;
        case 3:
            black_point = NUM2DBL(argv[0]);
            white_point = NUM2DBL(argv[1]);
            gamma_val   = NUM2DBL(argv[2]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 3)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    sprintf(level, "%gx%g+%g", black_point, white_point, gamma_val);
    (void) LevelImage(new_image, level);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Similar to Image#level but applies to a single channel only.
 *
 * Ruby usage:
 *   - @verbatim Image#level_channel(aChannelType) @endverbatim
 *   - @verbatim Image#level_channel(aChannelType, black) @endverbatim
 *   - @verbatim Image#level_channel(aChannelType, black, white) @endverbatim
 *   - @verbatim Image#level_channel(aChannelType, black, white, gamma) @endverbatim
 *
 * Notes:
 *   - Default black is 0.0
 *   - Default white is QuantumRange
 *   - Default gamma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see Image_level2
 */
VALUE
Image_level_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double black_point = 0.0, gamma_val = 1.0, white_point = (double)QuantumRange;
    ChannelType channel;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:             // take all the defaults
            break;
        case 2:
            black_point = NUM2DBL(argv[1]);
            white_point = QuantumRange - black_point;
            break;
        case 3:
            black_point = NUM2DBL(argv[1]);
            white_point = NUM2DBL(argv[2]);
            break;
        case 4:
            black_point = NUM2DBL(argv[1]);
            white_point = NUM2DBL(argv[2]);
            gamma_val   = NUM2DBL(argv[3]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 4)", argc);
            break;
    }

    VALUE_TO_ENUM(argv[0], channel, ChannelType);

    new_image = rm_clone_image(image);

    (void) LevelImageChannel(new_image, channel, black_point, white_point, gamma_val);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Implement +level_colors blank_color,white_color.
 *
 * Ruby usage:
 *   - @verbatim Image#level_colors @endverbatim
 *   - @verbatim Image#level_colors(black_color) @endverbatim
 *   - @verbatim Image#level_colors(black_color, white_color) @endverbatim
 *   - @verbatim Image#level_colors(black_color, white_color, invert) @endverbatim
 *   - @verbatim Image#level_colors(black_color, white_color, invert, channel) @endverbatim
 *   - @verbatim Image#level_colors(black_color, white_color, invert, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default black_color is "black"
 *   - Default white_color is "white"
 *   - Default invert is true
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_level_colors(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_LEVELIMAGECOLORS) || defined(HAVE_LEVELCOLORSIMAGECHANNEL)
    Image *image, *new_image;
    MagickPixelPacket black_color, white_color;
    ChannelType channels;
    ExceptionInfo exception;
    MagickBooleanType invert = MagickTrue;
    MagickBooleanType status;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    switch (argc)
    {
        case 3:
            invert = RTEST(argv[2]);

        case 2:
            Color_to_MagickPixelPacket(image, &white_color, argv[1]);
            Color_to_MagickPixelPacket(image, &black_color, argv[0]);
            break;

        case 1:
            Color_to_MagickPixelPacket(image, &black_color, argv[0]);
            GetExceptionInfo(&exception);

            GetMagickPixelPacket(image, &white_color);
            (void) QueryMagickColor("white", &white_color, &exception);
            CHECK_EXCEPTION()

            DestroyExceptionInfo(&exception);

        case 0:
            GetExceptionInfo(&exception);

            GetMagickPixelPacket(image, &white_color);
            (void) QueryMagickColor("white", &white_color, &exception);
            CHECK_EXCEPTION()

            GetMagickPixelPacket(image, &black_color);
            (void) QueryMagickColor("black", &black_color, &exception);
            CHECK_EXCEPTION()

            DestroyExceptionInfo(&exception);
            break;

        default:
            raise_ChannelType_error(argv[argc-1]);
            break;
    }

    new_image = rm_clone_image(image);

#if defined(HAVE_LEVELCOLORSIMAGECHANNEL)      // new in 6.5.6-4
    status = LevelColorsImageChannel(new_image, channels, &black_color, &white_color, invert);
#else
    status = LevelImageColors(new_image, channels, &black_color, &white_color, invert);
#endif
    rm_check_image_exception(new_image, DestroyOnError);
    if (!status)
    {
        rb_raise(rb_eRuntimeError, "LevelImageColors failed for unknown reason.");
    }

    return rm_image_new(new_image);

#else
    rm_not_implemented();
    self = self;
    argc = argc;
    argv = argv;
    return(VALUE)0;
#endif
}



/**
 * Levelize on a channel.
 *
 * Ruby usage:
 *   - @verbatim Image#levelize_channel(black_point) @endverbatim
 *   - @verbatim Image#levelize_channel(black_point, white_point) @endverbatim
 *   - @verbatim Image#levelize_channel(black_point, white_point, gamma) @endverbatim
 *   - @verbatim Image#levelize_channel(black_point, white_point, gamma, channel) @endverbatim
 *   - @verbatim Image#levelize_channel(black_point, white_point, gamma, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default white_point is QuantumRange
 *   - Default gamma is 1.0
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_levelize_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_LEVELIZEIMAGECHANNEL)
    Image *image, *new_image;
    ChannelType channels;
    double black_point, white_point;
    double gamma = 1.0;
    MagickBooleanType status;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 3)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    switch (argc)
    {
        case 3:
            gamma = NUM2DBL(argv[2]);
        case 2:
            white_point = NUM2DBL(argv[1]);
            black_point = NUM2DBL(argv[0]);
            break;
        case 1:
            black_point = NUM2DBL(argv[0]);
            white_point = QuantumRange - black_point;
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or more)", argc);
            break;
    }

    new_image = rm_clone_image(image);
    status = LevelizeImageChannel(new_image, channels, black_point, white_point, gamma);

    rm_check_image_exception(new_image, DestroyOnError);
    if (!status)
    {
        rb_raise(rb_eRuntimeError, "LevelizeImageChannel failed for unknown reason.");
    }
    return rm_image_new(new_image);
#else
    rm_not_implemented();
    self = self;
    argc = argc;
    argv = argv;
    return(VALUE)0;
#endif
}


/**
 * Call LinearStretchImage.
 *
 * Ruby usage:
 *   - @verbatim Image_linear_stretch(black_point) @endverbatim
 *   - @verbatim Image_linear_stretch(black_point , white_point) @endverbatim
 *
 * Notes:
 *   - Default white_point is pixels-black_point
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see Image_contrast_stretch_channel.
 * @see get_black_white_point
 */
VALUE
Image_linear_stretch(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double black_point, white_point;

    image = rm_check_destroyed(self);
    get_black_white_point(image, argc, argv, &black_point, &white_point);
    new_image = rm_clone_image(image);

    (void) LinearStretchImage(new_image, black_point, white_point);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call the LiquidRescaleImage API.
 *
 * Ruby usage:
 *   - @verbatim Image#liquid_rescale(columns, rows) @endverbatim
 *   - @verbatim Image#liquid_rescale(columns, rows, delta_x) @endverbatim
 *   - @verbatim Image#liquid_rescale(columns, rows, delta_x, rigidity) @endverbatim
 *
 * Notes:
 *   - Default delta_x is 0.0
 *   - Default rigidity is 0.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_liquid_rescale(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_LIQUIDRESCALEIMAGE)
    Image *image, *new_image;
    unsigned long cols, rows;
    double delta_x = 0.0;
    double rigidity = 0.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 4:
            rigidity = NUM2DBL(argv[3]);
        case 3:
            delta_x = NUM2DBL(argv[2]);
        case 2:
            rows = NUM2ULONG(argv[1]);
            cols = NUM2ULONG(argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 4)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = LiquidRescaleImage(image, cols, rows, delta_x, rigidity, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
#else
    argc = argc;    // defeat "unused parameter" messages
    argv = argv;
    self = self;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Implement marshalling.
 *
 * Ruby usage:
 *   - @verbatim Image._load @endverbatim
 *
 * Notes:
 *   - calls BlobToImage
 *
 * @param class Ruby class for Image
 * @param str the marshalled string
 * @return a new image
 * @see Image__dump
 */
VALUE
Image__load(VALUE class, VALUE str)
{
    Image *image;
    ImageInfo *info;
    DumpedImage mi;
    ExceptionInfo exception;
    char *blob;
    long length;

    class = class;  // Suppress "never referenced" message from icc

    info = CloneImageInfo(NULL);

    blob = rm_str2cstr(str, &length);

    // Must be as least as big as the 1st 4 fields in DumpedImage
    if (length <= (long)(sizeof(DumpedImage)-MaxTextExtent))
    {
        rb_raise(rb_eTypeError, "image is invalid or corrupted (too short)");
    }

    // Retrieve & validate the image format from the header portion
    mi.id = ((DumpedImage *)blob)->id;
    if (mi.id != DUMPED_IMAGE_ID)
    {
        rb_raise(rb_eTypeError, "image is invalid or corrupted (invalid header)");
    }

    mi.mj = ((DumpedImage *)blob)->mj;
    mi.mi = ((DumpedImage *)blob)->mi;
    if (   mi.mj != DUMPED_IMAGE_MAJOR_VERS
           || mi.mi > DUMPED_IMAGE_MINOR_VERS)
    {
        rb_raise(rb_eTypeError, "incompatible image format (can't be read)\n"
                 "\tformat version %d.%d required; %d.%d given"
                 , DUMPED_IMAGE_MAJOR_VERS, DUMPED_IMAGE_MINOR_VERS
                 , mi.mj, mi.mi);
    }

    mi.len = ((DumpedImage *)blob)->len;

    // Must be bigger than the header
    if (length <= (long)(mi.len+sizeof(DumpedImage)-MaxTextExtent))
    {
        rb_raise(rb_eTypeError, "image is invalid or corrupted (too short)");
    }

    memcpy(info->magick, ((DumpedImage *)blob)->magick, mi.len);
    info->magick[mi.len] = '\0';

    GetExceptionInfo(&exception);

    blob += offsetof(DumpedImage,magick) + mi.len;
    length -= offsetof(DumpedImage,magick) + mi.len;
    image = BlobToImage(info, blob, (size_t) length, &exception);
    (void) DestroyImageInfo(info);

    rm_check_exception(&exception, image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(image);

    return rm_image_new(image);
}


/**
 * Scale an image proportionally to twice its size.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @param magnifier function to use for magnification
 * @return self if bang, otherwise a new image
 */
static VALUE
magnify(int bang, VALUE self, magnifier_t magnifier)
{
    Image *image;
    Image *new_image;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);
    GetExceptionInfo(&exception);

    new_image = (magnifier)(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);
}


/**
 * Scale an image proportionally to twice its size.
 *
 * Ruby usage:
 *   - @verbatim Image#magnify @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see magnify
 * @see Image_magnify_bang
 */
VALUE
Image_magnify(VALUE self)
{
    (void) rm_check_destroyed(self);
    return magnify(False, self, MagnifyImage);
}


/**
 * Scale an image proportionally to twice its size.
 *
 * Ruby usage:
 *   - @verbatim Image#magnify! @endverbatim
 *
 * @param self this object
 * @return self
 * @see magnify
 * @see Image_magnify
 */
VALUE
Image_magnify_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return magnify(True, self, MagnifyImage);
}


/**
 * Call MapImage.
 *
 * Ruby usage:
 *   - @verbatim Image#map(map_image) @endverbatim
 *   - @verbatim Image#map(map_image, dither) @endverbatim
 *
 * Notes:
 *   - Default dither is false
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_map(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    Image *map;
    volatile VALUE map_obj, map_arg;
    unsigned int dither = MagickFalse;

    image = rm_check_destroyed(self);

#if defined(HAVE_REMAPIMAGE)
    rb_warning("Image#map is deprecated. Use Image#remap instead");
#endif

    switch (argc)
    {
        case 2:
            dither = RTEST(argv[1]);
        case 1:
            map_arg = argv[0];
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    map_obj = rm_cur_image(map_arg);
    map = rm_check_destroyed(map_obj);
    (void) MapImage(new_image, map, dither);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Support Marshal.dump >= 1.8.
 *
 * Ruby usage:
 *   - @verbatim Image#marshal_dump @endverbatim
 *
 * @param self this object
 * @return [img.filename, img.to_blob]
 */
VALUE
Image_marshal_dump(VALUE self)
{
    Image *image;
    Info *info;
    unsigned char *blob;
    size_t length;
    VALUE ary;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Info object");
    }

    ary = rb_ary_new2(2);
    if (image->filename)
    {
        rb_ary_store(ary, 0, rb_str_new2(image->filename));
    }
    else
    {
        rb_ary_store(ary, 0, Qnil);
    }

    GetExceptionInfo(&exception);
    blob = ImageToBlob(info, image, &length, &exception);

    // Destroy info before raising an exception
    DestroyImageInfo(info);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    rb_ary_store(ary, 1, rb_str_new((char *)blob, (long)length));
    magick_free((void*)blob);

    return ary;
}


/**
 * Support Marshal.load >= 1.8.
 *
 * Ruby usage:
 *   - @verbatim Image#marshal_load @endverbatim
 *
 * @param self this object
 * @param ary the array returned from marshal_dump
 * @return self
 */
VALUE
Image_marshal_load(VALUE self, VALUE ary)
{
    VALUE blob, filename;
    Info *info;
    Image *image;
    ExceptionInfo exception;

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Info object");
    }

    filename = rb_ary_shift(ary);
    blob = rb_ary_shift(ary);

    GetExceptionInfo(&exception);
    if (filename != Qnil)
    {
        strcpy(info->filename, RSTRING_PTR(filename));
    }
    image = BlobToImage(info, RSTRING_PTR(blob), RSTRING_LEN(blob), &exception);

    // Destroy info before raising an exception
    DestroyImageInfo(info);
    CHECK_EXCEPTION();
    (void) DestroyExceptionInfo(&exception);

    UPDATE_DATA_PTR(self, image);

    return self;
}


/**
 * Return the image's clip mask, or nil if it doesn't have a clip mask.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Distinguish from Image#clip_mask
 *
 * @param image the image
 * @return copy of the current clip-mask or nil
 */
static VALUE
get_image_mask(Image *image)
{
    Image *mask;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);

    // The returned clip mask is a clone, ours to keep.
    mask = GetImageClipMask(image, &exception);
    rm_check_exception(&exception, mask, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    return mask ? rm_image_new(mask) : Qnil;
}


/**
 * Set the image mask.
 *
 * Ruby usage:
 *   - @verbatim Image#mask= @endverbatim
 *
 * @param self this object
 * @param mask the mask to use
 * @return copy of the current clip-mask or nil
 * @deprecated Please use Image_mask(mask-image).
 * @see Image_mask(mask-image)
 * @see get_image_mask
 */
VALUE
Image_mask_eq(VALUE self, VALUE mask)
{
    VALUE v[1];
    v[0] = mask;
    return Image_mask(1, v, self);
}


/**
 * Associate a clip mask with the image.
 *
 * Ruby usage:
 *   - @verbatim Image#mask @endverbatim
 *   - @verbatim Image#mask(mask-image) @endverbatim
 *
 * Notes:
 *   - Omit the argument to get a copy of the current clip mask.
 *   - Pass "nil" for the mask-image to remove the current clip mask.
 *   - If the clip mask is not the same size as the target image, resizes the
 *     clip mask to match the target.
 *   - Distinguish from Image#clip_mask=
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return copy of the current clip-mask or nil
 * @see get_image_mask
 */
VALUE
Image_mask(int argc, VALUE *argv, VALUE self)
{
    volatile VALUE mask;
    Image *image, *mask_image, *resized_image;
    Image *clip_mask;
    long x, y;
    PixelPacket *q;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    if (argc == 0)
    {
        return get_image_mask(image);
    }
    if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (expected 0 or 1, got %d)", argc);
    }

    rb_check_frozen(self);
    mask = argv[0];

    if (mask != Qnil)
    {
        mask = rm_cur_image(mask);
        mask_image = rm_check_destroyed(mask);
        clip_mask = rm_clone_image(mask_image);

        // Resize if necessary
        if (clip_mask->columns != image->columns || clip_mask->rows != image->rows)
        {
            GetExceptionInfo(&exception);
            resized_image = ResizeImage(clip_mask, image->columns, image->rows
                                        , UndefinedFilter, 0.0, &exception);
            rm_check_exception(&exception, resized_image, DestroyOnError);
            (void) DestroyExceptionInfo(&exception);
            rm_ensure_result(resized_image);
            (void) DestroyImage(clip_mask);
            clip_mask = resized_image;
        }

        // The following section is copied from mogrify.c (6.2.8-8)
#if defined(HAVE_SYNCAUTHENTICPIXELS)
        GetExceptionInfo(&exception);
#endif
        for (y = 0; y < (long) clip_mask->rows; y++)
        {
#if defined(HAVE_GETAUTHENTICPIXELS)
            q = GetAuthenticPixels(clip_mask, 0, y, clip_mask->columns, 1, &exception);
            rm_check_exception(&exception, clip_mask, DestroyOnError);
#else
            q = GetImagePixels(clip_mask, 0, y, clip_mask->columns, 1);
            rm_check_image_exception(clip_mask, DestroyOnError);
#endif
            if (!q)
            {
                break;
            }
            for (x = 0; x < (long) clip_mask->columns; x++)
            {
                if (clip_mask->matte == MagickFalse)
                {
                    q->opacity = PIXEL_INTENSITY(q);
                }
                q->red = q->opacity;
                q->green = q->opacity;
                q->blue = q->opacity;
                q += 1;
            }

#if defined(HAVE_SYNCAUTHENTICPIXELS)
            SyncAuthenticPixels(clip_mask, &exception);
            rm_check_exception(&exception, clip_mask, DestroyOnError);
#else
            SyncImagePixels(clip_mask);
            rm_check_image_exception(clip_mask, DestroyOnError);
#endif
        }
#if defined(HAVE_SYNCAUTHENTICPIXELS)
        (void) DestroyExceptionInfo(&exception);
#endif

        SetImageStorageClass(clip_mask, DirectClass);
        rm_check_image_exception(clip_mask, DestroyOnError);

        clip_mask->matte = MagickTrue;

        // SetImageClipMask clones the clip_mask image. We can
        // destroy our copy after SetImageClipMask is done with it.

        (void) SetImageClipMask(image, clip_mask);
        (void) DestroyImage(clip_mask);
    }
    else
    {
        (void) SetImageClipMask(image, NULL);
    }

    // Always return a copy of the mask!
    return get_image_mask(image);
}


/**
 * Get matte attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#matte @endverbatim
 *
 * @param self this object
 * @return the matte
 * @deprecated Deprecated as of ImageMagick 6.3.6. See Image_alpha
 * @see Image_alpha
 * @see Image_alpha_eq
 */
VALUE
Image_matte(VALUE self)
{
    Image *image;

    image = rm_check_destroyed(self);
    return image->matte ? Qtrue : Qfalse;
}


/**
 * Set matte attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#matte= @endverbatim
 *
 * @param self this object
 * @param matte the matte
 * @return the matte
 * @deprecated Deprecated as of ImageMagick 6.3.6. See Image_alpha_eq
 * @see Image_alpha_eq
 * @see Image_alpha
 */
VALUE
Image_matte_eq(VALUE self, VALUE matte)
{
#if defined(HAVE_SETIMAGEALPHACHANNEL)
    VALUE alpha_channel_type;

    if (RTEST(matte))
    {
        alpha_channel_type = rb_const_get(Module_Magick, rb_intern("ActivateAlphaChannel"));
    }
    else
    {
        alpha_channel_type = rb_const_get(Module_Magick, rb_intern("DeactivateAlphaChannel"));
    }

    return Image_alpha_eq(self, alpha_channel_type);
#else
    Image *image = rm_check_frozen(self);
    image->matte = RTEST(matte) ? MagickTrue : MagickFalse;
    return matte;
#endif
}


/**
 * Return the matte color.
 *
 * Ruby usage:
 *   - @verbatim Image#matte_color @endverbatim
 *
 * @param self this object
 * @return the matte color
 */
VALUE
Image_matte_color(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return rm_pixelpacket_to_color_name(image, &image->matte_color);
}

/**
 * Set the matte color.
 *
 * Ruby usage:
 *   - @verbatim Image#matte_color= @endverbatim
 *
 * @param self this object
 * @param color the matte color
 * @return self
 */
VALUE
Image_matte_color_eq(VALUE self, VALUE color)
{
    Image *image = rm_check_frozen(self);
    Color_to_PixelPacket(&image->matte_color, color);
    return self;
}


/**
 * Call MatteFloodFillImage.
 *
 * Ruby usage:
 *   - @verbatim Image#matte_flood_fill(color, opacity, x, y, method_obj) @endverbatim
 *
 * @param self this object
 * @param color the color
 * @param opacity the opacity
 * @param x_obj x position
 * @param y_obj y position
 * @param method_obj which method to call: FloodfillMethod or FillToBorderMethod
 * @return a new image
 */
VALUE
Image_matte_flood_fill(VALUE self, VALUE color, VALUE opacity, VALUE x_obj, VALUE y_obj, VALUE method_obj)
{
    Image *image, *new_image;
    PixelPacket target;
    Quantum op;
    long x, y;
    PaintMethod method;

    image = rm_check_destroyed(self);
    Color_to_PixelPacket(&target, color);

    op = APP2QUANTUM(opacity);

    VALUE_TO_ENUM(method_obj, method, PaintMethod);
    if (!(method == FloodfillMethod || method == FillToBorderMethod))
    {
        rb_raise(rb_eArgError, "paint method_obj must be FloodfillMethod or "
                 "FillToBorderMethod (%d given)", method);
    }
    x = NUM2LONG(x_obj);
    y = NUM2LONG(y_obj);
    if ((unsigned long)x > image->columns || (unsigned long)y > image->rows)
    {
        rb_raise(rb_eArgError, "target out of range. %ldx%ld given, image is %lux%lu"
                 , x, y, image->columns, image->rows);
    }


    new_image = rm_clone_image(image);

#if defined(HAVE_FLOODFILLPAINTIMAGE)
    {
        DrawInfo *draw_info;
        MagickPixelPacket target_mpp;
        MagickBooleanType invert;

        // FloodfillPaintImage looks for the opacity in the DrawInfo.fill field.
        draw_info = CloneDrawInfo(NULL, NULL);
        if (!draw_info)
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        draw_info->fill.opacity = op;

        if (method == FillToBorderMethod)
        {
            invert = MagickTrue;
            target_mpp.red   = (MagickRealType) image->border_color.red;
            target_mpp.green = (MagickRealType) image->border_color.green;
            target_mpp.blue  = (MagickRealType) image->border_color.blue;
        }
        else
        {
            invert = MagickFalse;
            target_mpp.red   = (MagickRealType) target.red;
            target_mpp.green = (MagickRealType) target.green;
            target_mpp.blue  = (MagickRealType) target.blue;
        }

        (void) FloodfillPaintImage(new_image, OpacityChannel, draw_info, &target_mpp, x, y, invert);
    }
#else
    (void) MatteFloodfillImage(new_image, target, op, x, y, method);
#endif
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Apply a digital filter that improves the quality of a noisy image. Each pixel
 * is replaced by the median in a set of neighboring pixels as defined by
 * radius.
 *
 * Ruby usage:
 *   - @verbatim Image#median_filter @endverbatim
 *   - @verbatim Image#median_filter(radius) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_median_filter(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double radius = 0.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = MedianFilterImage(image, radius, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Get image mean error per pixel
 *
 * Ruby usage:
 *   - @verbatim Image#mean_error_per_pixel @endverbatim
 *
 * @param self this object
 * @return the mean error per pixel
 */
DEF_ATTR_READERF(Image, mean_error_per_pixel, error.mean_error_per_pixel, dbl)


/**
 * Return the officially registered (or de facto) MIME media-type corresponding
 * to the image format.
 *
 * Ruby usage:
 *   - @verbatim Image#mime_type @endverbatim
 *
 * @param self this object
 * @return the mime type
 */
VALUE
Image_mime_type(VALUE self)
{
    Image *image;
    char *type;
    volatile VALUE mime_type;

    image = rm_check_destroyed(self);
    type = MagickToMime(image->magick);
    if (!type)
    {
        return Qnil;
    }
    mime_type = rb_str_new2(type);

    // The returned string must be deallocated by the user.
    magick_free(type);

    return mime_type;
}


/**
 * Scale an image proportionally to half its size.
 *
 * Ruby usage:
 *   - @verbatim Image#minify @endverbatim
 *
 * @return minify: a new image 1/2x the size of the input image
 * @return minify!: self, 1/2x
 * @return a new image
 * @see Image_minify_bang
 */
VALUE
Image_minify(VALUE self)
{
    (void) rm_check_destroyed(self);
    return magnify(False, self, MinifyImage);
}


/**
 * Scale an image proportionally to half its size.
 *
 * Ruby usage:
 *   - @verbatim Image#minify! @endverbatim
 *
 * @param self this object
 * @return self
 * @see Image_minify
 */
VALUE
Image_minify_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return magnify(True, self, MinifyImage);
}


/**
 * Control the brightness, saturation, and hue of an image.
 *
 * Ruby usage:
 *   - @verbatim Image#modulate @endverbatim
 *   - @verbatim Image#modulate(brightness) @endverbatim
 *   - @verbatim Image#modulate(brightness, saturation) @endverbatim
 *   - @verbatim Image#modulate(brightness, saturation, hue) @endverbatim
 *
 * Notes:
 *   - Default brightness is 100.0
 *   - Default saturation is 100.0
 *   - Default hue is 100.0
 *   - all three arguments are optional and default to 100%
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_modulate(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double pct_brightness = 100.0,
    pct_saturation = 100.0,
    pct_hue        = 100.0;
    char modulate[100];

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 3:
            pct_hue        = 100*NUM2DBL(argv[2]);
        case 2:
            pct_saturation = 100*NUM2DBL(argv[1]);
        case 1:
            pct_brightness = 100*NUM2DBL(argv[0]);
            break;
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 3)", argc);
            break;
    }

    if (pct_brightness <= 0.0)
    {
        rb_raise(rb_eArgError, "brightness is %g%%, must be positive", pct_brightness);
    }
    sprintf(modulate, "%f%%,%f%%,%f%%", pct_brightness, pct_saturation, pct_hue);

    new_image = rm_clone_image(image);

    (void) ModulateImage(new_image, modulate);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Establish a progress monitor.
 *
 * Ruby usage:
 *   - @verbatim Image#monitor= proc @endverbatim
 *
 * Notes:
 *   - A progress monitor is a callable object. Save the monitor proc as the
 *     client_data and establish `progress_monitor' as the monitor exit. When
 *     `progress_monitor' is called, retrieve the proc and call it.
 *
 * @param self this object
 * @param monitor the progress monitor
 * @return self
 */
VALUE
Image_monitor_eq(VALUE self, VALUE monitor)
{
    Image *image = rm_check_frozen(self);

    if (NIL_P(monitor))
    {
        image->progress_monitor = NULL;
    }
    else
    {
        (void) SetImageProgressMonitor(image, rm_progress_monitor, (void *)monitor);
    }


    return self;
}


/**
 * Return true if all the pixels in the image have the same red, green, and blue
 * intensities and the intensity is either 0 or QuantumRange.
 *
 * Ruby usage:
 *   - @verbatim Image#monochrome? @endverbatim
 *
 * @param self this object
 * @return true if monochrome, false otherwise
 */
VALUE
Image_monochrome_q(VALUE self)
{
    return has_attribute(self, (MagickBooleanType (*)(const Image *, ExceptionInfo *))IsMonochromeImage);
}


/**
 * Tile size and offset within an image montage. Only valid for montage images.
 *
 * Ruby usage:
 *   - @verbatim Image#montage @endverbatim
 *
 * @param self this object
 * @return the tile size and offset
 */
DEF_ATTR_READER(Image, montage, str)


/**
 * Called from Image_motion_blur and Image_sketch.
 *
 * No Ruby usage (internal function)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param fp the blur function to call
 * @return a new image
 * @see Image_motion_blur
 * @see Image_sketch
 */
static VALUE
motion_blur(int argc, VALUE *argv, VALUE self
            , Image *fp(const Image *, const double, const double, const double, ExceptionInfo *))
{
    Image *image, *new_image;
    double radius = 0.0;
    double sigma = 1.0;
    double angle = 0.0;
    ExceptionInfo exception;

    switch (argc)
    {
        case 3:
            angle = NUM2DBL(argv[2]);
        case 2:
            sigma = NUM2DBL(argv[1]);
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 3)", argc);
            break;
    }

    if (sigma == 0.0)
    {
        rb_raise(rb_eArgError, "sigma must be != 0.0");
    }

    Data_Get_Struct(self, Image, image);

    GetExceptionInfo(&exception);
    new_image = (fp)(image, radius, sigma, angle, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Simulate motion blur. Convolve the image with a Gaussian operator of the
 * given radius and standard deviation (sigma). For reasonable results, radius
 * should be larger than sigma. Use a radius of 0 and motion_blur selects a
 * suitable radius for you. Angle gives the angle of the blurring motion.
 *
 * Ruby usage:
 *   - @verbatim Image#motion_blur @endverbatim
 *   - @verbatim Image#motion_blur(radius) @endverbatim
 *   - @verbatim Image#motion_blur(radius, sigma) @endverbatim
 *   - @verbatim Image#motion_blur(radius, sigma, angle) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default angle is 0.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_motion_blur(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return motion_blur(argc, argv, self, MotionBlurImage);
}


/**
 * Negate the colors in the reference image. The grayscale option means that
 * only grayscale values within the image are negated.
 *
 * Ruby usage:
 *   - @verbatim Image#negate @endverbatim
 *   - @verbatim Image#negate(grayscale) @endverbatim
 *
 * Notes:
 *   - Default grayscale is false.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_negate(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    unsigned int grayscale = MagickFalse;

    image = rm_check_destroyed(self);
    if (argc == 1)
    {
        grayscale = RTEST(argv[0]);
    }
    else if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }

    new_image = rm_clone_image(image);

    (void) NegateImage(new_image, grayscale);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Negate the colors on a particular channel. The grayscale option means that
 * only grayscale values within the image are negated.
 *
 * Ruby usage:
 *   - @verbatim Image#negate_channel(grayscale=false, channel=AllChannels) @endverbatim
 *
 * Ruby usage:
 *   - @verbatim Image#negate_channel @endverbatim
 *   - @verbatim Image#negate_channel(grayscale) @endverbatim
 *   - @verbatim Image#negate_channel(grayscale, channel) @endverbatim
 *   - @verbatim Image#negate_channel(grayscale, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default grayscale is false.
 *   - Default channel is AllChannels.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_negate_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;
    unsigned int grayscale = MagickFalse;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There can be at most 1 remaining argument.
    if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }
    else if (argc == 1)
    {
        grayscale = RTEST(argv[0]);
    }

    Data_Get_Struct(self, Image, image);

    new_image = rm_clone_image(image);

    (void)NegateImageChannel(new_image, channels, grayscale);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * "Allocate" a new Image object
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Actually we defer allocating the image until the initialize method so we
 *     can run the parm block if it's present.
 * 
 * @param class the Ruby class for an Image
 * @return a newly allocated image
 */
VALUE
Image_alloc(VALUE class)
{
    volatile VALUE image_obj;

    image_obj = Data_Wrap_Struct(class, NULL, rm_image_destroy, NULL);
    return image_obj;
}

/**
 * Initialize a new Image object If the fill argument is omitted, fill with
 * background color.
 *
 * Ruby usage:
 *   - @verbatim Image#initialize(cols,rows) @endverbatim
 *   - @verbatim Image#initialize(cols,rows,fill) @endverbatim
 *
 * Notes:
 *   - Default fill is false
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */
VALUE
Image_initialize(int argc, VALUE *argv, VALUE self)
{
    volatile VALUE fill = 0;
    Info *info;
    volatile VALUE info_obj;
    Image *image;
    unsigned long cols, rows;

    switch (argc)
    {
        case 3:
            fill = argv[2];
        case 2:
            rows = NUM2ULONG(argv[1]);
            cols = NUM2ULONG(argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
            break;
    }

    // Create a new Info object to use when creating this image.
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    image = AcquireImage(info);
    if (!image)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    rm_set_user_artifact(image, info);

    // NOW store a real image in the image object.
    UPDATE_DATA_PTR(self, image);

    SetImageExtent(image, cols, rows);

    // If the caller did not supply a fill argument, call SetImageBackgroundColor
    // to fill the image using the background color. The background color can
    // be set by specifying it when creating the Info parm block.
    if (!fill)
    {
        (void) SetImageBackgroundColor(image);
    }
    // fillobj.fill(self)
    else
    {
        (void) rb_funcall(fill, rm_ID_fill, 1, self);
    }

    return self;
}


/**
 * Create a new Image object from an Image structure.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Since the Image is already created we don't need to call Image_alloc or
 *     Image_initialize.
 *
 * @param image the Image structure
 * @return a new image
 */
VALUE
rm_image_new(Image *image)
{
    if (!image)
    {
        rb_bug("rm_image_new called with NULL argument");
    }

    (void) rm_trace_creation(image);

    return Data_Wrap_Struct(Class_Image, NULL, rm_image_destroy, image);
}


/**
 * Enhance the contrast of a color image by adjusting the pixels color to span
 * the entire range of colors available.
 *
 * Ruby usage:
 *   - @verbatim Image#normalize @endverbatim
 *
 * @param self this object
 * @return a new image
 */
VALUE
Image_normalize(VALUE self)
{
    Image *image, *new_image;

    image = rm_check_destroyed(self);
    new_image = rm_clone_image(image);

    (void) NormalizeImage(new_image);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call NormalizeImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#normalize_channel @endverbatim
 *   - @verbatim Image#normalize_channel(channel) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_normalize_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    // Ensure all arguments consumed.
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    new_image = rm_clone_image(image);

    (void) NormalizeImageChannel(new_image, channels);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Get image normalized mean error
 *
 * Ruby usage:
 *   - @verbatim Image#normalized_mean_error @endverbatim
 *
 * @param self this object
 * @return the normalized mean error
 */
DEF_ATTR_READERF(Image, normalized_mean_error, error.normalized_mean_error, dbl)

/**
 * Get image normalized maximum error
 *
 * Ruby usage:
 *   - @verbatim Image#normalized_maximum_error @endverbatim
 *
 * @param self this object
 * @return the normalized maximum error
 */
DEF_ATTR_READERF(Image, normalized_maximum_error, error.normalized_maximum_error, dbl)


/**
 * Return the number of unique colors in the image.
 *
 * Ruby usage:
 *   - @verbatim Image#number_colors @endverbatim
 *
 * @param self this object
 * @return number of unique colors
 */
VALUE
Image_number_colors(VALUE self)
{
    Image *image;
    ExceptionInfo exception;
    unsigned long n = 0;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    n = (unsigned long) GetNumberColors(image, NULL, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return ULONG2NUM(n);
}


DEF_ATTR_ACCESSOR(Image, offset, long)


/**
 * Apply a special effect filter that simulates an oil painting.
 *
 * Ruby usage:
 *   - @verbatim Image#oil_paint @endverbatim
 *   - @verbatim Image#oil_paint(radius) @endverbatim
 *
 * Notes:
 *   - Default radius is 3.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_oil_paint(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double radius = 3.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = OilPaintImage(image, radius, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Change any pixel that matches target with the color defined by fill.
 *
 * Ruby usage:
 *   - @verbatim Image#opaque(target-color-name, fill-color-name) @endverbatim
 *   - @verbatim Image#opaque(target-pixel, fill-pixel) @endverbatim
 *
 * Notes:
 *   - By default a pixel must match the specified target color exactly.
 *   - Use Image_fuzz_eq to set the amount of tolerance acceptable to consider
 *     two colors as the same.
 *
 * @param self this object
 * @param target either the color name or the pixel
 * @param fill the color for filling
 * @see Image_fuzz_eq
 */
VALUE
Image_opaque(VALUE self, VALUE target, VALUE fill)
{
    Image *image, *new_image;
    MagickPixelPacket target_pp;
    MagickPixelPacket fill_pp;
    MagickBooleanType okay;

    image = rm_check_destroyed(self);
    new_image = rm_clone_image(image);

    // Allow color name or Pixel
    Color_to_MagickPixelPacket(image, &target_pp, target);
    Color_to_MagickPixelPacket(image, &fill_pp, fill);

#if defined(HAVE_OPAQUEPAINTIMAGECHANNEL)
    okay = OpaquePaintImageChannel(new_image, DefaultChannels, &target_pp, &fill_pp, MagickFalse);
#else
    okay =  PaintOpaqueImageChannel(new_image, DefaultChannels, &target_pp, &fill_pp);
#endif
    rm_check_image_exception(new_image, DestroyOnError);

    if (!okay)
    {
        // Force exception
        DestroyImage(new_image);
        rm_ensure_result(NULL);
    }

    return rm_image_new(new_image);
}


/**
 * Improved Image#opaque available in ImageMagick 6.3.7-10.
 *
 * Ruby usage:
 *   - @verbatim Image#opaque_channel @endverbatim
 *   - @verbatim opaque_channel(target, fill) @endverbatim
 *   - @verbatim opaque_channel(target, fill, invert) @endverbatim
 *   - @verbatim opaque_channel(target, fill, invert, fuzz) @endverbatim
 *   - @verbatim opaque_channel(target, fill, invert, fuzz, channel) @endverbatim
 *   - @verbatim opaque_channel(target, fill, invert, fuzz, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default invert is false
 *   - Default fuzz is the image's fuzz (see Image_fuzz_eq)
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_opaque_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_OPAQUEPAINTIMAGECHANNEL)
    Image *image, *new_image;
    MagickPixelPacket target_pp, fill_pp;
    ChannelType channels;
    double keep, fuzz;
    MagickBooleanType okay, invert = MagickFalse;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 4)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    // Default fuzz value is image's fuzz attribute.
    fuzz = image->fuzz;

    switch (argc)
    {
        case 4:
            fuzz = NUM2DBL(argv[3]);
            if (fuzz < 0.0)
            {
                rb_raise(rb_eArgError, "fuzz must be >= 0.0 (%g given)", fuzz);
            }
        case 3:
            invert = RTEST(argv[2]);
        case 2:
            // Allow color name or Pixel
            Color_to_MagickPixelPacket(image, &fill_pp, argv[1]);
            Color_to_MagickPixelPacket(image, &target_pp, argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (got %d, expected 2 or more)", argc);
            break;
    }

    new_image = rm_clone_image(image);
    keep = new_image->fuzz;
    new_image->fuzz = fuzz;

    okay = OpaquePaintImageChannel(new_image, channels, &target_pp, &fill_pp, invert);

    // Restore saved fuzz value
    new_image->fuzz = keep;
    rm_check_image_exception(new_image, DestroyOnError);

    if (!okay)
    {
        // Force exception
        DestroyImage(new_image);
        rm_ensure_result(NULL);
    }

    return rm_image_new(new_image);

#else
    argc = argc;    // defeat "unused parameter" messages
    argv = argv;
    self = self;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Return true if any of the pixels in the image have an opacity value other
 * than opaque ( 0 ).
 *
 * Ruby usage:
 *   - @verbatim Image#opaque? @endverbatim
 *
 * @param self this object
 * @return true if opaque, false otherwise
 */
VALUE
Image_opaque_q(VALUE self)
{
    return has_attribute(self, IsOpaqueImage);
}


/**
 * Perform ordered dither on image.
 *
 * Ruby usage:
 *   - @verbatim Image#ordered_dither @endverbatim
 *   - @verbatim Image#ordered_dither(threshold_map) @endverbatim
 *
 * Notes:
 *   - Default threshold_map is '2x2'
 *   - Order of threshold_map must be 2, 3, or 4.
 *   - If using ImageMagick >= 6.3.0, order can be any of the threshold strings
 *     listed by "convert -list Thresholds"
 *   - Does not call OrderedDitherImages anymore. Sometime after ImageMagick
 *     6.0.0 it quit working. Uses the same routines as ImageMagick and
 *     GraphicsMagick for their "ordered-dither" option.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_ordered_dither(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    int order;
    const char *threshold_map = "2x2";
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }
    if (argc == 1)
    {
        if (TYPE(argv[0]) == T_STRING)
        {
            threshold_map = StringValuePtr(argv[0]);
        }
        else
        {
            order = NUM2INT(argv[0]);
            if (order == 3)
            {
                threshold_map = "3x3";
            }
            else if (order == 4)
            {
                threshold_map = "4x4";
            }
            else if (order != 2)
            {
                rb_raise(rb_eArgError, "order must be 2, 3, or 4 (%d given)", order);
            }
        }
    }

    new_image = rm_clone_image(image);

    GetExceptionInfo(&exception);

    // ImageMagick >= 6.2.9
    (void) OrderedPosterizeImage(new_image, threshold_map, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Return the orientation attribute as an OrientationType enum value.
 *
 * Ruby usage:
 *   - @verbatim Image#orientation @endverbatim
 *
 * @param self this object
 * @return the orientation
 */
VALUE
Image_orientation(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return OrientationType_new(image->orientation);
}


/**
 * Set the orientation attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#orientation= @endverbatim
 *
 * @param self this object
 * @param orientation the orientation
 * @return self
 */
VALUE
Image_orientation_eq(VALUE self, VALUE orientation)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(orientation, image->orientation, OrientationType);
    return self;
}


/**
 * The page attribute getter.
 *
 * Ruby usage:
 *   - @verbatim Image#page @endverbatim
 *
 * @param self
 * @return the page rectangle
 */
VALUE
Image_page(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return Import_RectangleInfo(&image->page);
}


/**
 * The page attribute setter.
 *
 * Ruby usage:
 *   - @verbatim Image#page= @endverbatim
 *
 * @param self this object
 * @param rect the page rectangle
 * @return self
 */
VALUE
Image_page_eq(VALUE self, VALUE rect)
{
    Image *image = rm_check_frozen(self);
    Export_RectangleInfo(&image->page, rect);
    return self;
}


/**
 * Improved version of Image#transparent available in ImageMagick 6.3.7-10.
 *
 * Ruby usage:
 *   - @verbatim Image#paint_transparent(target) @endverbatim
 *   - @verbatim Image#paint_transparent(target, opacity) @endverbatim
 *   - @verbatim Image#paint_transparent(target, opacity, invert) @endverbatim
 *   - @verbatim Image#paint_transparent(target, opacity, invert, fuzz) @endverbatim
 *
 * Notes:
 *   - Default opacity is TransparentOpacity
 *   - Default invert is false
 *   - Default fuzz is the image's fuzz (see Image_fuzz_eq)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_paint_transparent(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_TRANSPARENTPAINTIMAGE)
    Image *image, *new_image;
    MagickPixelPacket color;
    Quantum opacity = TransparentOpacity;
    double keep, fuzz;
    MagickBooleanType okay, invert = MagickFalse;

    image = rm_check_destroyed(self);

    // Default fuzz value is image's fuzz attribute.
    fuzz = image->fuzz;

    switch (argc)
    {
        case 4:
            fuzz = NUM2DBL(argv[3]);
        case 3:
            invert = RTEST(argv[2]);
        case 2:
            opacity = APP2QUANTUM(argv[1]);
        case 1:
            Color_to_MagickPixelPacket(image, &color, argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 4)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    // Use fuzz value from caller
    keep = new_image->fuzz;
    new_image->fuzz = fuzz;

    okay = TransparentPaintImage(new_image, (const MagickPixelPacket *)&color, opacity, invert);
    new_image->fuzz = keep;

    // Is it possible for TransparentPaintImage to silently fail?
    rm_check_image_exception(new_image, DestroyOnError);
    if (!okay)
    {
        // Force exception
        DestroyImage(new_image);
        rm_ensure_result(NULL);
    }

    return rm_image_new(new_image);
#else
    argc = argc;    // defeat "unused parameter" messages
    argv = argv;
    self = self;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Return true if the image is PseudoClass and has 256 unique colors or less.
 *
 * Ruby usage:
 *   - @verbatim Image#palette? @endverbatim
 *
 * @param self this object
 * @return true if palette, otherwise false
 */
VALUE
Image_palette_q(VALUE self)
{
    return has_attribute(self, IsPaletteImage);
}


/**
 * Call ImagePing.
 *
 * Ruby usage:
 *   - @verbatim Image.ping(file) @endverbatim
 *
 * @param class the Ruby class for an Image
 * @param file_arg the file containing image info
 * @return an array of 1 or more new image objects (without pixel data)
 * @see Image_read
 * @see rd_image
 */
VALUE
Image_ping(VALUE class, VALUE file_arg)
{
    return rd_image(class, file_arg, PingImage);
}


/**
 * Get/set the color of the pixel at x,y.
 *
 * Ruby usage:
 *   - @verbatim Image#pixel_color(x, y) @endverbatim
 *   - @verbatim Image#pixel_color(x, y, color) @endverbatim
 *
 * Notes:
 *   - Without color, does a get. With color, does a set.
 *   - "color", if present, may be either a color name or a Magick::Pixel.
 *   - Based on Magick++'s Magick::pixelColor methods
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return Magick::Pixel for pixel x,y. If called to set a new color, the
 * return value is the old color.
 */
VALUE
Image_pixel_color(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    PixelPacket old_color, new_color, *pixel;
    ExceptionInfo exception;
    long x, y;
    unsigned int set = False;
    MagickBooleanType okay;

    memset(&old_color, 0, sizeof(old_color));

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 3:
            rb_check_frozen(self);
            set = True;
            // Replace with new color? The arg can be either a color name or
            // a Magick::Pixel.
            Color_to_PixelPacket(&new_color, argv[2]);
        case 2:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
            break;
    }

    x = NUM2LONG(argv[0]);
    y = NUM2LONG(argv[1]);

    // Get the color of a pixel
    if (!set)
    {
        GetExceptionInfo(&exception);
#if defined(HAVE_GETVIRTUALPIXELS)
        old_color = *GetVirtualPixels(image, x, y, 1, 1, &exception);
#else
        old_color = *AcquireImagePixels(image, x, y, 1, 1, &exception);
#endif
        CHECK_EXCEPTION()

        (void) DestroyExceptionInfo(&exception);

        // PseudoClass
        if (image->storage_class == PseudoClass)
        {
#if defined(HAVE_GETAUTHENTICINDEXQUEUE)
            IndexPacket *indexes = GetAuthenticIndexQueue(image);
#else
            IndexPacket *indexes = GetIndexes(image);
#endif
            old_color = image->colormap[*indexes];
        }
        if (!image->matte)
        {
            old_color.opacity = OpaqueOpacity;
        }
        return Pixel_from_PixelPacket(&old_color);
    }

    // ImageMagick segfaults if the pixel location is out of bounds.
    // Do what IM does and return the background color.
    if (x < 0 || y < 0 || (unsigned long)x >= image->columns || (unsigned long)y >= image->rows)
    {
        return Pixel_from_PixelPacket(&image->background_color);
    }

    // Set the color of a pixel. Return previous color.
    // Convert to DirectClass
    if (image->storage_class == PseudoClass)
    {
        okay = SetImageStorageClass(image, DirectClass);
        rm_check_image_exception(image, RetainOnError);
        if (!okay)
        {
            rb_raise(Class_ImageMagickError, "SetImageStorageClass failed. Can't set pixel color.");
        }
    }


#if defined(HAVE_GETAUTHENTICPIXELS) || defined(HAVE_SYNCAUTHENTICPIXELS)
    GetExceptionInfo(&exception);
#endif

#if defined(HAVE_GETAUTHENTICPIXELS)
    pixel = GetAuthenticPixels(image, x, y, 1, 1, &exception);
    CHECK_EXCEPTION()
#else
    pixel = GetImagePixels(image, x, y, 1, 1);
    rm_check_image_exception(image, RetainOnError);
#endif

    if (pixel)
    {
        old_color = *pixel;
        if (!image->matte)
        {
            old_color.opacity = OpaqueOpacity;
        }
    }
    *pixel = new_color;

#if defined(HAVE_SYNCAUTHENTICPIXELS)
    SyncAuthenticPixels(image, &exception);
    CHECK_EXCEPTION()
#else
    SyncImagePixels(image);
    rm_check_image_exception(image, RetainOnError);
#endif

#if defined(HAVE_GETAUTHENTICPIXELS) || defined(HAVE_SYNCAUTHENTICPIXELS)
    (void) DestroyExceptionInfo(&exception);
#endif

    return Pixel_from_PixelPacket(&old_color);
}


/**
 * Get the "interpolate" field in the Image structure.
 *
 * Ruby usage:
 *   - @verbatim Image.pixel_interpolation_method @endverbatim
 *
 * @param self this object
 * @return the interpolate field
 * @see Image_pixel_interpolation_method_eq
 * @see Image.interpolate_pixel_color
 */
VALUE
Image_pixel_interpolation_method(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return InterpolatePixelMethod_new(image->interpolate);
}


/**
 * Set the "interpolate" field in the Image structure.
 *
 * Ruby usage:
 *   - @verbatim Image.pixel_interpolation_method=method @endverbatim
 *
 * @param self this object
 * @param method the interpolate field
 * @return self
 * @see Image_pixel_interpolation_method
 * @see Image.interpolate_pixel_color
 */
VALUE
Image_pixel_interpolation_method_eq(VALUE self, VALUE method)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(method, image->interpolate, InterpolatePixelMethod);
    return self;
}


/**
 * Call PolaroidImage.
 *
 * Ruby usage:
 *   - @verbatim Image#polaroid { optional parms } @endverbatim
 *   - @verbatim Image#polaroid(angle) { optional parms } @endverbatim
 *
 * Notes:
 *   - Default angle is -5
 *   - Accepts an options block to get Draw attributes for drawing the label.
 *     Specify self.border_color to set a non-default border color.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_polaroid(int argc, VALUE *argv, VALUE self)
{
    Image *image, *clone, *new_image;
    volatile VALUE options;
    double angle = -5.0;
    Draw *draw;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 1:
            angle = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }

    options = rm_polaroid_new();
    Data_Get_Struct(options, Draw, draw);

    clone = rm_clone_image(image);
    clone->background_color = draw->shadow_color;
    clone->border_color = draw->info->border_color;

    GetExceptionInfo(&exception);
    new_image = PolaroidImage(clone, draw->info, angle, &exception);
    rm_check_exception(&exception, clone, DestroyOnError);

    (void) DestroyImage(clone);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call PosterizeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#posterize(levels=4, dither=false) @endverbatim
 *
 * Notes:
 *   - Default levels is 4
 *   - Default dither is false
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_posterize(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    MagickBooleanType dither = MagickFalse;
    unsigned long levels = 4;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 2:
            dither = (MagickBooleanType) RTEST(argv[1]);
            /* fall through */
        case 1:
            levels = NUM2ULONG(argv[0]);
            /* fall through */
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
    }

    new_image = rm_clone_image(image);

    (void) PosterizeImage(new_image, levels, dither);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call PreviewImage.
 *
 * Ruby usage:
 *   - @verbatim Image#preview(preview) @endverbatim
 *
 * @param self this object
 * @param preview the preview
 * @return a new image
 */
VALUE
Image_preview(VALUE self, VALUE preview)
{
    Image *image, *new_image;
    PreviewType preview_type;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    image = rm_check_destroyed(self);
    VALUE_TO_ENUM(preview, preview_type, PreviewType);

    new_image = PreviewImage(image, preview_type, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Set the image profile. If "profile" is nil, deletes the profile. Otherwise
 * "profile" must be a string containing the specified profile.
 *
 * Ruby usage:
 *   - @verbatim Image#profile!(name, profile) @endverbatim
 *
 * @param self this object
 * @param name the profile name
 * @param profile the profile
 * @return self
 */
VALUE
Image_profile_bang(VALUE self, VALUE name, VALUE profile)
{

    if (profile == Qnil)
    {
        return Image_delete_profile(self, name);
    }
    else
    {
        return set_profile(self, StringValuePtr(name), profile);
    }

}


/**
 * Get image quality.
 *
 * Ruby usage:
 *   - @verbatim Image#quality @endverbatim
 *
 * @param self this object
 * @return the quality
 */
DEF_ATTR_READER(Image, quality, ulong)


/**
 * Return image depth to nearest quantum.
 *
 * Ruby usage:
 *   - @verbatim Image#quantum_depth -> 8, 16, or 32 @endverbatim
 *
 * Notes:
 *   - IM 6.0.0 introduced GetImageQuantumDepth
 *   - IM 6.0.5 added a 2nd argument. The MagickFalse argument gives the 6.0.5
 *     version the same behavior as before.
 *
 * @param self this object
 * @return image depth
 */
VALUE
Image_quantum_depth(VALUE self)
{
    Image *image;
    unsigned long quantum_depth;

    image = rm_check_destroyed(self);
    quantum_depth = GetImageQuantumDepth(image, MagickFalse);

    rm_check_image_exception(image, RetainOnError);

    return ULONG2NUM(quantum_depth);
}


/**
 * This method is an adapter method that calls the EvaluateImageChannel method.
 *
 * Ruby usage:
 *   - @verbatim Image#quantum_operator(operator, rvalue) @endverbatim
 *   - @verbatim Image#quantum_operator(operator, rvalue, channel) @endverbatim
 *   - @verbatim Image#quantum_operator(operator, rvalue, channel, ...) @endverbatim
 *
 * Notes:
 *   - Historically this method used QuantumOperatorRegionImage in
 *     GraphicsMagick. By necessity this method implements the "lowest common
 *     denominator" of the two implementations.
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */
VALUE
Image_quantum_operator(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    QuantumExpressionOperator operator;
    MagickEvaluateOperator qop;
    double rvalue;
    ChannelType channel;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    // The default channel is AllChannels
    channel = AllChannels;

    /*
        If there are 3 arguments, argument 2 is a ChannelType argument.
        Arguments 1 and 0 are required and are the rvalue and operator,
        respectively.
    */
    switch (argc)
    {
        case 3:
            VALUE_TO_ENUM(argv[2], channel, ChannelType);
            /* Fall through */
        case 2:
            rvalue = NUM2DBL(argv[1]);
            VALUE_TO_ENUM(argv[0], operator, QuantumExpressionOperator);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
            break;
    }

    // Map QuantumExpressionOperator to MagickEvaluateOperator
    switch (operator)
    {
        default:
        case UndefinedQuantumOperator:
            qop = UndefinedEvaluateOperator;
            break;
        case AddQuantumOperator:
            qop = AddEvaluateOperator;
            break;
        case AndQuantumOperator:
            qop = AndEvaluateOperator;
            break;
        case DivideQuantumOperator:
            qop = DivideEvaluateOperator;
            break;
        case LShiftQuantumOperator:
            qop = LeftShiftEvaluateOperator;
            break;
        case MaxQuantumOperator:
            qop = MaxEvaluateOperator;
            break;
        case MinQuantumOperator:
            qop = MinEvaluateOperator;
            break;
        case MultiplyQuantumOperator:
            qop = MultiplyEvaluateOperator;
            break;
        case OrQuantumOperator:
            qop = OrEvaluateOperator;
            break;
        case RShiftQuantumOperator:
            qop = RightShiftEvaluateOperator;
            break;
        case SubtractQuantumOperator:
            qop = SubtractEvaluateOperator;
            break;
        case XorQuantumOperator:
            qop = XorEvaluateOperator;
            break;
#if defined(HAVE_ENUM_POWEVALUATEOPERATOR)
        case PowQuantumOperator:
            qop = PowEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_LOGEVALUATEOPERATOR)
        case LogQuantumOperator:
            qop = LogEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_THRESHOLDEVALUATEOPERATOR)
        case ThresholdQuantumOperator:
            qop = ThresholdEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_THRESHOLDBLACKEVALUATEOPERATOR)
        case ThresholdBlackQuantumOperator:
            qop = ThresholdBlackEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_THRESHOLDWHITEEVALUATEOPERATOR)
        case ThresholdWhiteQuantumOperator:
            qop = ThresholdWhiteEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_GAUSSIANNOISEEVALUATEOPERATOR)
        case GaussianNoiseQuantumOperator:
            qop = GaussianNoiseEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_IMPULSENOISEEVALUATEOPERATOR)
        case ImpulseNoiseQuantumOperator:
            qop = ImpulseNoiseEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_LAPLACIANNOISEEVALUATEOPERATOR)
        case LaplacianNoiseQuantumOperator:
            qop = LaplacianNoiseEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_MULTIPLICATIVENOISEEVALUATEOPERATOR)
        case MultiplicativeNoiseQuantumOperator:
            qop = MultiplicativeNoiseEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_POISSONNOISEEVALUATEOPERATOR)
        case PoissonNoiseQuantumOperator:
            qop = PoissonNoiseEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_UNIFORMNOISEEVALUATEOPERATOR)
        case UniformNoiseQuantumOperator:
            qop = UniformNoiseEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_COSINEEVALUATEOPERATOR)
        case CosineQuantumOperator:
            qop = CosineEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_SINEEVALUATEOPERATOR)
        case SineQuantumOperator:
            qop = SineEvaluateOperator;
            break;
#endif
#if defined(HAVE_ENUM_ADDMODULUSEVALUATEOPERATOR)
        case AddModulusQuantumOperator:
            qop = AddModulusEvaluateOperator;
            break;
#endif
    }

    GetExceptionInfo(&exception);
    (void) EvaluateImageChannel(image, channel, qop, rvalue, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return self;
}


/**
 * Call QuantizeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#quantize @endverbatim
 *   - @verbatim Image#quantize(number_colors) @endverbatim
 *   - @verbatim Image#quantize(number_colors, colorspace) @endverbatim
 *   - @verbatim Image#quantize(number_colors, colorspace, dither) @endverbatim
 *   - @verbatim Image#quantize(number_colors, colorspace, dither, tree_depth) @endverbatim
 *   - @verbatim Image#quantize(number_colors, colorspace, dither, tree_depth, measure_error) @endverbatim
 *
 * Notes:
 *   - Default number_colors is 256
 *   - Default colorspace is Magick::RGBColorspace
 *   - Default dither is true
 *   - Default tree_depth is 0
 *   - Default measure_error is false
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_quantize(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    QuantizeInfo quantize_info;

    image = rm_check_destroyed(self);
    GetQuantizeInfo(&quantize_info);

    switch (argc)
    {
        case 5:
            quantize_info.measure_error = (MagickBooleanType) RTEST(argv[4]);
        case 4:
            quantize_info.tree_depth = NUM2UINT(argv[3]);
        case 3:
#if defined(HAVE_TYPE_DITHERMETHOD) && defined(HAVE_ENUM_NODITHERMETHOD)
            if (rb_obj_is_kind_of(argv[2], Class_DitherMethod))
            {
                VALUE_TO_ENUM(argv[2], quantize_info.dither_method, DitherMethod);
                quantize_info.dither = quantize_info.dither_method != NoDitherMethod;
            }
#else
            quantize_info.dither = (MagickBooleanType) RTEST(argv[2]);
#endif
        case 2:
            VALUE_TO_ENUM(argv[1], quantize_info.colorspace, ColorspaceType);
        case 1:
            quantize_info.number_colors = NUM2UINT(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 5)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    (void) QuantizeImage(&quantize_info, new_image);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call RadialBlurImage.
 *
 * Ruby usage:
 *   - @verbatim Image#radial_blur(angle) @endverbatim
 *
 * @param self this object
 * @param angle the angle (in degrees)
 * @return a new image
 */
VALUE
Image_radial_blur(VALUE self, VALUE angle)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    new_image = RadialBlurImage(image, NUM2DBL(angle), &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call RadialBlurImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#radial_blur_channel(angle) @endverbatim
 *   - @verbatim Image#radial_blur_channel(angle, channel) @endverbatim
 *   - @verbatim Image#radial_blur_channel(angle, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *   - Angle is in degrees
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_radial_blur_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There must be 1 remaining argument.
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (0 for 1 or more)");
    }
    else if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);

    new_image = RadialBlurImageChannel(image, channels, NUM2DBL(argv[0]), &exception);

    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call RandomThresholdImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#random_threshold_channel(geometry_str) @endverbatim
 *   - @verbatim Image#random_threshold_channel(geometry_str, channel) @endverbatim
 *   - @verbatim Image#random_threshold_channel(geometry_str, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_random_threshold_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;
    char *thresholds;
    volatile VALUE geom_str;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    channels = extract_channels(&argc, argv);

    // There must be 1 remaining argument.
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "missing threshold argument");
    }
    else if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    // Accept any argument that has a to_s method.
    geom_str = rm_to_s(argv[0]);
    thresholds = StringValuePtr(geom_str);

    new_image = rm_clone_image(image);

    GetExceptionInfo(&exception);

    (void) RandomThresholdImageChannel(new_image, channels, thresholds, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Create a simulated three-dimensional button-like effect by lightening and
 * darkening the edges of the image. The "width" and "height" arguments define
 * the width of the vertical and horizontal edge of the effect. If "raised" is
 * true, creates a raised effect, otherwise a lowered effect.
 *
 * Ruby usage:
 *   - @verbatim Image#raise @endverbatim
 *   - @verbatim Image#raise(width) @endverbatim
 *   - @verbatim Image#raise(width, height) @endverbatim
 *   - @verbatim Image#raise(width, height, raised) @endverbatim
 *
 * Notes:
 *   - Default width is 6
 *   - Default height is 6
 *   - Default raised is true
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_raise(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    RectangleInfo rect;
    int raised = MagickTrue;      // default

    memset(&rect, 0, sizeof(rect));
    rect.width = 6;         // default
    rect.height = 6;        // default

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 3:
            raised = RTEST(argv[2]);
        case 2:
            rect.height = NUM2ULONG(argv[1]);
        case 1:
            rect.width = NUM2ULONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 3)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    (void) RaiseImage(new_image, &rect, raised);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call ReadImage.
 *
 * Ruby usage:
 *   - @verbatim Image.read(file) @endverbatim
 *
 * @param class the Ruby class for an Image
 * @param file_arg the file containing image data
 * @return an array of 1 or more new image objects
 * @see rd_image
 */
VALUE
Image_read(VALUE class, VALUE file_arg)
{
    return rd_image(class, file_arg, ReadImage);
}


/**
 * Called when `rm_obj_to_s' raised an exception.
 *
 * No Ruby usage (internal function)
 *
 * @param arg the bad arg given
 * @return 0
 */
static VALUE
file_arg_rescue(VALUE arg)
{
    rb_raise(rb_eTypeError, "argument must be path name or open file (%s given)",
             rb_class2name(CLASS_OF(arg)));
    return(VALUE)0;
}


/**
 * Transform arguments, call either ReadImage or PingImage.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Yields to a block to get Image::Info attributes before calling
 *     Read/PingImage
 *
 * @param class the Ruby class for an Image
 * @param file the file containing image data
 * @param reader which image reader to use (ReadImage or PingImage)
 * @return an array of 1 or more new image objects
 * @see Image_read
 * @see Image_ping
 * @see array_from_images
 */
static VALUE
rd_image(VALUE class, VALUE file, reader_t reader)
{
    char *filename;
    long filename_l;
    Info *info;
    volatile VALUE info_obj;
    Image *images;
    ExceptionInfo exception;

    class = class;  // defeat gcc message

    // Create a new Info structure for this read/ping
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    if (TYPE(file) == T_FILE)
    {
        OpenFile *fptr;

        // Ensure file is open - raise error if not
        GetOpenFile(file, fptr);
        rb_io_check_readable(fptr);
        SetImageInfoFile(info, GetReadFile(fptr));
    }
    else
    {
        // Convert arg to string. If an exception occurs raise an error condition.
        file = rb_rescue(rb_String, file, file_arg_rescue, file);

        filename = rm_str2cstr(file, &filename_l);
        filename_l = min(filename_l, MaxTextExtent-1);
        memcpy(info->filename, filename, (size_t)filename_l);
        info->filename[filename_l] = '\0';
        SetImageInfoFile(info, NULL);
    }

    GetExceptionInfo(&exception);

    images = (reader)(info, &exception);
    rm_check_exception(&exception, images, DestroyOnError);
    rm_set_user_artifact(images, info);
    (void) DestroyExceptionInfo(&exception);

    return array_from_images(images);
}


/**
 * Call RecolorImage.
 *
 * Ruby usage:
 *   - @verbatim Image#recolor(matrix) @endverbatim
 *
 * @param self this object
 * @param color_matrix the matrix
 * @return a new image
 */
VALUE
Image_recolor(VALUE self, VALUE color_matrix)
{
    Image *image, *new_image;
    unsigned long order;
    long x, len;
    double *matrix;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    // Allocate color matrix from Ruby's memory
    len = RARRAY_LEN(color_matrix);
    matrix = ALLOC_N(double, len);

    for (x = 0; x < len; x++)
    {
        matrix[x] = NUM2DBL(rb_ary_entry(color_matrix, x));
    }

    order = (unsigned long)sqrt((double)(len + 1.0));

    // RecolorImage sets the ExceptionInfo and returns a NULL image if an error occurs.
    new_image = RecolorImage(image, order, matrix, &exception);
    xfree((void *)matrix);

    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Read a Base64-encoded image.
 *
 * Ruby usage:
 *   - @verbatim Image.read_inline(content) @endverbatim
 *
 * Notes:
 *   - This is similar to, but not the same as ReadInlineImage. ReadInlineImage
 *     requires a comma preceeding the image data. This method allows but does
 *     not require a comma.
 *
 * @param self this object
 * @param content the content
 * @return an array of new images
 * @see array_from_images
 */
VALUE
Image_read_inline(VALUE self, VALUE content)
{
    volatile VALUE info_obj;
    Image *images;
    ImageInfo *info;
    char *image_data;
    long x, image_data_l;
    unsigned char *blob;
    size_t blob_l;
    ExceptionInfo exception;

    self = self;    // defeat gcc message

    image_data = rm_str2cstr(content, &image_data_l);

    // Search for a comma. If found, we'll set the start of the
    // image data just following the comma. Otherwise we'll assume
    // the image data starts with the first byte.
    for (x = 0; x < image_data_l; x++)
    {
        if (image_data[x] == ',')
        {
            break;
        }
    }
    if (x < image_data_l)
    {
        image_data += x + 1;
    }

    blob = Base64Decode(image_data, &blob_l);
    if (blob_l == 0)
    {
        rb_raise(rb_eArgError, "can't decode image");
    }

    GetExceptionInfo(&exception);

    // Create a new Info structure for this read. About the
    // only useful attribute that can be set is `format'.
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    images = BlobToImage(info, blob, blob_l, &exception);
    magick_free((void *)blob);

    rm_check_exception(&exception, images, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    rm_set_user_artifact(images, info);

    return array_from_images(images);
}


/**
 * Convert a list of images to an array of Image objects.
 *
 * No Ruby usage (internal function)
 *
 * @param images the images
 * @return array of images
 */
static VALUE
array_from_images(Image *images)
{
    volatile VALUE image_obj, image_ary;
    Image *image;

    // Orphan the image, create an Image object, add it to the array.

    image_ary = rb_ary_new();
    while (images)
    {
        image = RemoveFirstImageFromList(&images);
        image_obj = rm_image_new(image);
        (void) rb_ary_push(image_ary, image_obj);
    }

    return image_ary;
}


/**
 * Smooth the contours of an image while still preserving edge information.
 *
 * Ruby usage:
 *   - @verbatim Image#reduce_noise(radius) @endverbatim
 *
 * @param self this object
 * @param radius the radius
 * @return a new image
 */
VALUE
Image_reduce_noise(VALUE self, VALUE radius)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    GetExceptionInfo(&exception);
    new_image = ReduceNoiseImage(image, NUM2DBL(radius), &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Call RemapImage.
 *
 * Ruby usage:
 *   - @verbatim Image#remap(remap_image) @endverbatim
 *   - @verbatim Image#remap(remap_image, dither_method) @endverbatim
 *
 * Notes:
 *   - Default dither_method is RiemersmaDitherMethod
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */
VALUE
Image_remap(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_REMAPIMAGE) || defined(HAVE_AFFINITYIMAGE)
    Image *image, *remap_image;
    QuantizeInfo quantize_info;

    image = rm_check_frozen(self);
    if (argc > 0)
    {
        volatile VALUE t = rm_cur_image(argv[0]);
        remap_image = rm_check_destroyed(t);
    }

    GetQuantizeInfo(&quantize_info);

    switch (argc)
    {
        case 2:
            VALUE_TO_ENUM(argv[1], quantize_info.dither_method, DitherMethod);
            quantize_info.dither = MagickTrue;
            break;
        case 1:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

#if defined(HAVE_REMAPIMAGE)
    (void) RemapImage(&quantize_info, image, remap_image);
#else
    (void) AffinityImage(&quantize_info, image, remap_image);
#endif
    rm_check_image_exception(image, RetainOnError);

    return self;
#else
    self = self;
    argc = argc;
    argv = argv;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Get rendering_intent.
 *
 * Ruby usage:
 *   - @verbatim Image#rendering_intent @endverbatim
 *
 * @param self this object
 * @return the rendering intent
 */
VALUE
Image_rendering_intent(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return RenderingIntent_new(image->rendering_intent);
}


/**
 * Set rendering_intent.
 *
 * Ruby usage:
 *   - @verbatim Image#rendering_intent= @endverbatim
 *
 * @param self this object
 * @param ri the rendering intent
 * @return self
 */
VALUE
Image_rendering_intent_eq(VALUE self, VALUE ri)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(ri, image->rendering_intent, RenderingIntent);
    return self;
}


/**
 * Scale an image to the desired dimensions using the specified filter and blur
 * factor.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see Image_resize
 * @see Image_resize_bang
 */
static VALUE
resize(int bang, int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double scale_arg;
    FilterTypes filter;
    unsigned long rows, columns;
    double blur, drows, dcols;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);

    // Set up defaults
    filter  = image->filter;
    blur    = image->blur;
    rows    = image->rows;
    columns = image->columns;

    switch (argc)
    {
        case 4:
            blur = NUM2DBL(argv[3]);
        case 3:
            VALUE_TO_ENUM(argv[2], filter, FilterTypes);
        case 2:
            rows = NUM2ULONG(argv[1]);
            columns = NUM2ULONG(argv[0]);
            if (columns == 0 || rows == 0)
            {
                rb_raise(rb_eArgError, "invalid result dimension (%lu, %lu given)", columns, rows);
            }
            break;
        case 1:
            scale_arg = NUM2DBL(argv[0]);
            if (scale_arg < 0.0)
            {
                rb_raise(rb_eArgError, "invalid scale_arg value (%g given)", scale_arg);
            }
            drows = scale_arg * image->rows + 0.5;
            dcols = scale_arg * image->columns + 0.5;
            if (drows > (double)ULONG_MAX || dcols > (double)ULONG_MAX)
            {
                rb_raise(rb_eRangeError, "resized image too big");
            }
            rows = (unsigned long) drows;
            columns = (unsigned long) dcols;
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 4)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = ResizeImage(image, columns, rows, filter, blur, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }
    return rm_image_new(new_image);
}


/**
 * Scale an image to the desired dimensions using the specified filter and blur
 * factor.
 *
 * Ruby usage:
 *   - @verbatim Image#resize(scale) @endverbatim
 *   - @verbatim Image#resize(cols, rows) @endverbatim
 *   - @verbatim Image#resize(cols, rows, filter) @endverbatim
 *   - @verbatim Image#resize(cols, rows, filter, blur) @endverbatim
 *
 * Notes:
 *   - Default filter is image->filter
 *   - Default blur is image->blur
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see resize
 * @see Image_resize_bang
 */
VALUE
Image_resize(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return resize(False, argc, argv, self);
}


/**
 * Scale an image to the desired dimensions using the specified filter and blur
 * factor.
 *
 * Ruby usage:
 *   - @verbatim Image#resize!(scale) @endverbatim
 *   - @verbatim Image#resize!(cols, rows) @endverbatim
 *   - @verbatim Image#resize!(cols, rows, filter) @endverbatim
 *   - @verbatim Image#resize!(cols, rows, filter, blur) @endverbatim
 *
 * Notes:
 *   - Default filter is image->filter
 *   - Default blur is image->blur
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see resize
 * @see Image_resize
 */
VALUE
Image_resize_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return resize(True, argc, argv, self);
}


/**
 * Offset an image as defined by x_offset and y_offset.
 *
 * Ruby usage:
 *   - @verbatim Image#roll(x_offset, y_offset) @endverbatim
 *
 * @param self this object
 * @param x_offset the x offset
 * @param y_offset the y offset
 * @return a new image
 */
VALUE
Image_roll(VALUE self, VALUE x_offset, VALUE y_offset)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    GetExceptionInfo(&exception);
    new_image = RollImage(image, NUM2LONG(x_offset), NUM2LONG(y_offset), &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Rotate the image.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see Image_rotate
 * @see Image_rotate_bang
 */
static VALUE
rotate(int bang, int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double degrees;
    char *arrow;
    long arrow_l;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);

    switch (argc)
    {
        case 2:
            arrow = rm_str2cstr(argv[1], &arrow_l);
            if (arrow_l != 1 || (*arrow != '<' && *arrow != '>'))
            {
                rb_raise(rb_eArgError, "second argument must be '<' or '>', '%s' given", arrow);
            }
            if (*arrow == '>' && image->columns <= image->rows)
            {
                return Qnil;
            }
            if (*arrow == '<' && image->columns >= image->rows)
            {
                return Qnil;
            }
        case 1:
            degrees = NUM2DBL(argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = RotateImage(image, degrees, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }
    return rm_image_new(new_image);
}


/**
 * Rotate the image.
 *
 * Ruby usage:
 *   - @verbatim Image#rotate(degrees) @endverbatim
 *   - @verbatim Image#rotate(degrees, '<') @endverbatim
 *   - @verbatim Image#rotate(degrees, '>') @endverbatim
 *
 * Notes:
 *   - If the 2nd argument is '<' rotate only if width < height. If the 2nd
 *     argument is '>' rotate only if width > height.
 *   - Default is to always rotate
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see rotate
 * @see Image_rotate_bang
 */
VALUE
Image_rotate(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return rotate(False, argc, argv, self);
}


/**
 * Rotate the image.
 *
 * Ruby usage:
 *   - @verbatim Image#rotate!(degrees) @endverbatim
 *   - @verbatim Image#rotate!(degrees, '<') @endverbatim
 *   - @verbatim Image#rotate!(degrees, '>') @endverbatim
 *
 * Notes:
 *   - If the 2nd argument is '<' rotate only if width < height. If the 2nd
 *     argument is '>' rotate only if width > height.
 *   - Default is to always rotate
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see rotate
 * @see Image_rotate
 */
VALUE
Image_rotate_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return rotate(True, argc, argv, self);
}


/**
 * Return image rows.
 *
 * Ruby usage:
 *   - @verbatim Image#rows @endverbatim
 *
 * @param self this object
 * @return the image rows
 */
DEF_ATTR_READER(Image, rows, int)


/**
 * Scale an image to the desired dimensions with pixel sampling.
 *
 * Ruby usage:
 *   - @verbatim Image#sample(scale) @endverbatim
 *   - @verbatim Image#sample(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see scale
 * @see Image_sample_bang
 */
VALUE
Image_sample(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return scale(False, argc, argv, self, SampleImage);
}


/**
 * Scale an image to the desired dimensions with pixel sampling.
 *
 * Ruby usage:
 *   - @verbatim Image#sample!(scale) @endverbatim
 *   - @verbatim Image#sample!(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see scale
 * @see Image_sample
 */
VALUE
Image_sample_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return scale(True, argc, argv, self, SampleImage);
}


/**
 * Change the size of an image to the given dimensions.
 *
 * Ruby usage:
 *   - @verbatim Image#scale(scale) @endverbatim
 *   - @verbatim Image#scale(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see scale
 * @see Image_scale_bang
 */
VALUE
Image_scale(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return scale(False, argc, argv, self, ScaleImage);
}


/**
 * Change the size of an image to the given dimensions.
 *
 * Ruby usage:
 *   - @verbatim Image#scale!(scale) @endverbatim
 *   - @verbatim Image#scale!(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see scale
 * @see Image_scale
 */
VALUE
Image_scale_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return scale(True, argc, argv, self, ScaleImage);
}


/**
 * Call ScaleImage or SampleImage
 *
 * Notes:
 *   - If 1 argument > 0, multiply current size by this much.
 *   - If 2 arguments, (cols, rows).
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param scaler which scalar to use (ScaleImage or SampleImage)
 * @return self if bang, otherwise a new image
 * @see Image_sample
 * @see Image_sample_bang
 * @see Image_scale
 * @see Image_scale_bang
 */
static VALUE
scale(int bang, int argc, VALUE *argv, VALUE self, scaler_t scaler)
{
    Image *image, *new_image;
    unsigned long columns, rows;
    double scale_arg, drows, dcols;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);

    switch (argc)
    {
        case 2:
            columns = NUM2ULONG(argv[0]);
            rows    = NUM2ULONG(argv[1]);
            if (columns == 0 || rows == 0)
            {
                rb_raise(rb_eArgError, "invalid result dimension (%lu, %lu given)", columns, rows);
            }
            break;
        case 1:
            scale_arg = NUM2DBL(argv[0]);
            if (scale_arg <= 0)
            {
                rb_raise(rb_eArgError, "invalid scale value (%g given)", scale_arg);
            }
            drows = scale_arg * image->rows + 0.5;
            dcols = scale_arg * image->columns + 0.5;
            if (drows > (double)ULONG_MAX || dcols > (double)ULONG_MAX)
            {
                rb_raise(rb_eRangeError, "resized image too big");
            }
            rows = (unsigned long) drows;
            columns = (unsigned long) dcols;
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = (scaler)(image, columns, rows, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);
}


/**
 * Return image scene.
 *
 * Ruby usage:
 *   - @verbatim Image#scene @endverbatim
 *
 * @param self this object
 * @return the image scene
 */
DEF_ATTR_READER(Image, scene, ulong)


/**
 * Call SelectiveBlurImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#selective_blur_channel(radius, sigma, threshold) @endverbatim
 *   - @verbatim Image#selective_blur_channel(radius, sigma, threshold, channel) @endverbatim
 *   - @verbatim Image#selective_blur_channel(radius, sigma, threshold, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_selective_blur_channel(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_SELECTIVEBLURIMAGECHANNEL)
    Image *image, *new_image;
    double radius, sigma, threshold;
    ExceptionInfo exception;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 3)
    {
        raise_ChannelType_error(argv[argc-1]);
    }
    if (argc != 3)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 3 or more)", argc);
    }
    radius = NUM2DBL(argv[0]);
    sigma = NUM2DBL(argv[1]);

    // threshold is either a floating-point number or a string in the form "NN%".
    // Either way it's supposed to represent a percentage of the QuantumRange.
    threshold = rm_percentage(argv[2],1.0) * QuantumRange;

    GetExceptionInfo(&exception);
    new_image = SelectiveBlurImageChannel(image, channels, radius, sigma, threshold, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);

#else
    rm_not_implemented();
    argc = argc;
    argv = argv;
    self = self;
    return (VALUE)0;
#endif
}


/**
 * Call SetImageChannelDepth.
 *
 * Ruby usage:
 *   - @verbatim Image#set_channel_depth(channel, depth) @endverbatim
 *
 * @param self this object
 * @param channel_arg the channel
 * @param depth the depth
 * @return self
 */
VALUE
Image_set_channel_depth(VALUE self, VALUE channel_arg, VALUE depth)
{
    Image *image;
    ChannelType channel;
    unsigned long channel_depth;

    image = rm_check_frozen(self);

    VALUE_TO_ENUM(channel_arg, channel, ChannelType);
    channel_depth = NUM2ULONG(depth);

    (void) SetImageChannelDepth(image, channel, channel_depth);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Call SeparateImages.
 *
 * Ruby usage:
 *   - @verbatim separate @endverbatim
 *   - @verbatim separate(channel) @endverbatim
 *   - @verbatim separate(channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new ImageList
 */
VALUE
Image_separate(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_images;
    ChannelType channels = 0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // All arguments are ChannelType enums
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    GetExceptionInfo(&exception);
    new_images = SeparateImages(image, channels, &exception);
    rm_check_exception(&exception, new_images, DestroyOnError);
    DestroyExceptionInfo(&exception);
    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/**
 * Call SepiaToneImage.
 *
 * Ruby usage:
 *   - @verbatim Image#sepiatone @endverbatim
 *   - @verbatim Image#sepiatone(threshold) @endverbatim
 *
 * Notes:
 *   - Default threshold is QuantumRange
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_sepiatone(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double threshold = (double) QuantumRange;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 1:
            threshold = NUM2DBL(argv[0]);
            break;
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }

    GetExceptionInfo(&exception);
    new_image = SepiaToneImage(image, threshold, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call SegmentImage.
 *
 * Ruby usage:
 *   - @verbatim Image#segment @endverbatim
 *   - @verbatim Image#segment(colorspace) @endverbatim
 *   - @verbatim Image#segment(colorspace,cluster_threshold) @endverbatim
 *   - @verbatim Image#segment(colorspace,cluster_threshold,smoothing_threshold) @endverbatim
 *   - @verbatim Image#segment(colorspace,cluster_threshold,smoothing_threshold,verbose) @endverbatim
 *
 * Notes:
 *   - Default colorspace is RGBColorspace
 *   - Default cluster_threshold is 1.0
 *   - Default smoothing_threshold is 1.5
 *   - Default verbose is false
 *   - The default values are the same as Magick++
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_segment(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    int colorspace              = RGBColorspace;    // These are the Magick++ defaults
    unsigned int verbose        = MagickFalse;
    double cluster_threshold    = 1.0;
    double smoothing_threshold  = 1.5;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 4:
            verbose = RTEST(argv[3]);
        case 3:
            smoothing_threshold = NUM2DBL(argv[2]);
        case 2:
            cluster_threshold = NUM2DBL(argv[1]);
        case 1:
            VALUE_TO_ENUM(argv[0], colorspace, ColorspaceType);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 4)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    (void) SegmentImage(new_image, colorspace, verbose, cluster_threshold, smoothing_threshold);
    rm_check_image_exception(new_image, DestroyOnError);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call SetImageOpacity.
 *
 * Ruby usage:
 *   - @verbatim Image#opacity= @endverbatim
 *
 * @param self this object
 * @param opacity_arg the opacity
 * @return self
 */
VALUE
Image_opacity_eq(VALUE self, VALUE opacity_arg)
{
    Image *image;
    Quantum opacity;

    image = rm_check_frozen(self);
    opacity = APP2QUANTUM(opacity_arg);
    (void) SetImageOpacity(image, opacity);
    rm_check_image_exception(image, RetainOnError);
    return self;
}


/**
 * Traverse the attributes and yield to the block. If no block, return a hash
 * of all the attribute keys & values.
 *
 * Ruby usage:
 *   - @verbatim Image#properties [{ |k,v| block }] @endverbatim
 *
 * Notes:
 *   - I use the word "properties" to distinguish between these "user-added"
 *     attribute strings and Image object attributes.
 *
 * @param self this object
 * @return self if block, else hash of attribute keys and values.
 */
VALUE
Image_properties(VALUE self)
{
    Image *image;
    volatile VALUE attr_hash;
    volatile VALUE ary;
    char *property;
    const char *value;

    image = rm_check_destroyed(self);

    if (rb_block_given_p())
    {
        ary = rb_ary_new2(2);

        ResetImagePropertyIterator(image);
        property = GetNextImageProperty(image);
        while (property)
        {
            value = GetImageProperty(image, property);
            (void) rb_ary_store(ary, 0, rb_str_new2(property));
            (void) rb_ary_store(ary, 1, rb_str_new2(value));
            (void) rb_yield(ary);
            property = GetNextImageProperty(image);
        }
        rm_check_image_exception(image, RetainOnError);
        return self;
    }

    // otherwise return properties hash
    else
    {
        attr_hash = rb_hash_new();
        ResetImagePropertyIterator(image);
        property = GetNextImageProperty(image);
        while (property)
        {
            value = GetImageProperty(image, property);
            (void) rb_hash_aset(attr_hash, rb_str_new2(property), rb_str_new2(value));
            property = GetNextImageProperty(image);
        }
        rm_check_image_exception(image, RetainOnError);
        return attr_hash;
    }

}


/**
 * Shine a distant light on an image to create a three-dimensional effect. You
 * control the positioning of the light with azimuth and elevation; azimuth is
 * measured in degrees off the x axis and elevation is measured in pixels above
 * the Z axis.
 *
 * Ruby usage:
 *   - @verbatim Image#shade @endverbatim
 *   - @verbatim Image#shade(shading) @endverbatim
 *   - @verbatim Image#shade(shading, azimuth) @endverbatim
 *   - @verbatim Image#shade(shading, azimuth, elevation) @endverbatim
 *
 * Notes:
 *   - Default shading is false
 *   - Default azimuth is 30
 *   - Default elevation is 30
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_shade(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double azimuth = 30.0, elevation = 30.0;
    unsigned int shading=MagickFalse;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 3:
            elevation = NUM2DBL(argv[2]);
        case 2:
            azimuth = NUM2DBL(argv[1]);
        case 1:
            shading = RTEST(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 3)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = ShadeImage(image, shading, azimuth, elevation, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call ShadowImage. X- and y-offsets are the pixel offset. Opacity is either a
 * number between 0 and 1 or a string "NN%". Sigma is the std. dev. of the
 * Gaussian, in pixels.
 *
 * Ruby usage:
 *   - @verbatim Image#shadow @endverbatim
 *   - @verbatim Image#shadow(x_offset) @endverbatim
 *   - @verbatim Image#shadow(x_offset, y_offset) @endverbatim
 *   - @verbatim Image#shadow(x_offset, y_offset, sigma) @endverbatim
 *   - @verbatim Image#shadow(x_offset, y_offset, sigma, opacity) @endverbatim
 *
 * Notes:
 *   - Default x_offset is 4
 *   - Default y_offset is 4
 *   - Default sigma is 4.0
 *   - Default opacity is 1.0
 *   - The defaults are taken from the mogrify.c source, except for opacity,
 *     which has no default.
 *   - Introduced in ImageMagick 6.1.7
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_shadow(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double opacity = 100.0;
    double sigma = 4.0;
    long x_offset = 4L;
    long y_offset = 4L;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 4:
            opacity = rm_percentage(argv[3],1.0);   // Clamp to 1.0 < x <= 100.0
            if (fabs(opacity) < 0.01)
            {
                rb_warning("shadow will be transparent - opacity %g very small", opacity);
            }
            opacity = FMIN(opacity, 1.0);
            opacity = FMAX(opacity, 0.01);
            opacity *= 100.0;
        case 3:
            sigma = NUM2DBL(argv[2]);
        case 2:
            y_offset = NUM2LONG(argv[1]);
        case 1:
            x_offset = NUM2LONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 4)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = ShadowImage(image, opacity, sigma, x_offset, y_offset, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Sharpen an image.
 *
 * Ruby usage:
 *   - @verbatim Image#sharpen @endverbatim
 *   - @verbatim Image#sharpen(radius) @endverbatim
 *   - @verbatim Image#sharpen(radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see effect_image
 */
VALUE
Image_sharpen(int argc, VALUE *argv, VALUE self)
{
    return effect_image(self, argc, argv, SharpenImage);
}


/**
 * Sharpen image on a channel.
 *
 * Ruby usage:
 *   - @verbatim Image#sharpen_channel @endverbatim
 *   - @verbatim Image#sharpen_channel(radius) @endverbatim
 *   - @verbatim Image#sharpen_channel(radius, sigma) @endverbatim
 *   - @verbatim Image#sharpen_channel(radius, sigma, channel) @endverbatim
 *   - @verbatim Image#sharpen_channel(radius, sigma, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_sharpen_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;
    ExceptionInfo exception;
    double radius = 0.0, sigma = 1.0;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    // There must be 0, 1, or 2 remaining arguments.
    switch (argc)
    {
        case 2:
            sigma = NUM2DBL(argv[1]);
            /* Fall thru */
        case 1:
            radius = NUM2DBL(argv[0]);
            /* Fall thru */
        case 0:
            break;
        default:
            raise_ChannelType_error(argv[argc-1]);
    }

    new_image = rm_clone_image(image);

    GetExceptionInfo(&exception);
    (void) SharpenImageChannel(new_image, channels, radius, sigma, &exception);

    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


/**
 * Shave pixels from the image edges, leaving a rectangle of the specified width
 * & height in the center.
 *
 * Ruby usage:
 *   - @verbatim Image#shave(width, height) @endverbatim
 *
 * @param self this object
 * @param width the width to leave
 * @param height the hight to leave
 * @return a new image
 * @see xform_image
 * @see Image_shave_bang
 */
VALUE
Image_shave(VALUE self, VALUE width, VALUE height)
{
    (void) rm_check_destroyed(self);
    return xform_image(False, self, INT2FIX(0), INT2FIX(0), width, height, ShaveImage);
}


/**
 * Shave pixels from the image edges, leaving a rectangle of the specified width
 * & height in the center.
 *
 * Ruby usage:
 *   - @verbatim Image#shave!(width, height) @endverbatim
 *
 * @param self this object
 * @param width the width to leave
 * @param height the hight to leave
 * @return self
 * @see xform_image
 * @see Image_shave
 */
VALUE
Image_shave_bang(VALUE self, VALUE width, VALUE height)
{
    (void) rm_check_frozen(self);
    return xform_image(True, self, INT2FIX(0), INT2FIX(0), width, height, ShaveImage);
}


/**
 * Call ShearImage.
 *
 * Ruby usage:
 *   - @verbatim Image#shear(x_shear, y_shear) @endverbatim
 *
 * @param self this object
 * @param x_shear the x shear (in degrees)
 * @param y_shear the y shear (in degrees)
 * @return a new image
 */
VALUE
Image_shear(VALUE self, VALUE x_shear, VALUE y_shear)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    GetExceptionInfo(&exception);
    new_image = ShearImage(image, NUM2DBL(x_shear), NUM2DBL(y_shear), &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);
    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call SigmoidalContrastImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#sigmoidal_contrast_channel @endverbatim
 *   - @verbatim Image#sigmoidal_contrast_channel(contrast) @endverbatim
 *   - @verbatim Image#sigmoidal_contrast_channel(contrast, midpoint) @endverbatim
 *   - @verbatim Image#sigmoidal_contrast_channel(contrast, midpoint, sharpen) @endverbatim
 *   - @verbatim Image#sigmoidal_contrast_channel(contrast, midpoint, sharpen, channel) @endverbatim
 *   - @verbatim Image#sigmoidal_contrast_channel(contrast, midpoint, sharpen, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default contrast is 3.0
 *   - Default midpoint is 50.0
 *   - Default sharpen is false
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_sigmoidal_contrast_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    MagickBooleanType sharpen = MagickFalse;
    double contrast = 3.0;
    double midpoint = 50.0;
    ChannelType channels;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);

    switch (argc)
    {
        case 3:
            sharpen  = (MagickBooleanType) RTEST(argv[2]);
        case 2:
            midpoint = NUM2DBL(argv[1]);
        case 1:
            contrast = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            raise_ChannelType_error(argv[argc-1]);
            break;
    }

    new_image = rm_clone_image(image);

    (void) SigmoidalContrastImageChannel(new_image, channels, sharpen, contrast, midpoint);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Compute a message digest from an image pixel stream with an implementation of
 * the NIST SHA-256 Message Digest algorithm.
 *
 * Ruby usage:
 *   - @verbatim Image#signature @endverbatim
 *
 * @param self this object
 * @return the message digest
 */
VALUE
Image_signature(VALUE self)
{
    Image *image;
    const char *signature;

    image = rm_check_destroyed(self);

    (void) SignatureImage(image);
    signature = rm_get_property(image, "signature");
    rm_check_image_exception(image, RetainOnError);
    if (!signature)
    {
        return Qnil;
    }
    return rb_str_new(signature, 64);
}


/**
 * Call SketchImage.
 *
 * Ruby usage:
 *   - @verbatim Image#sketch @endverbatim
 *   - @verbatim Image#sketch(radius) @endverbatim
 *   - @verbatim Image#sketch(radius, sigma) @endverbatim
 *   - @verbatim Image#sketch(radius, sigma, angle) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default angle is 0.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see motion_blur
 */
VALUE
Image_sketch(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return motion_blur(argc, argv, self, SketchImage);
}


/**
 * Apply a special effect to the image, similar to the effect achieved in a
 * photo darkroom by selectively exposing areas of photo sensitive paper to
 * light. Threshold ranges from 0 to QuantumRange and is a measure of the extent
 * of the solarization.
 *
 * Ruby usage:
 *   - @verbatim Image#solarize @endverbatim
 *   - @verbatim Image#solarize(threshold) @endverbatim
 *
 * Notes:
 *   - Default threshold is 50.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_solarize(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double threshold = 50.0;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:
            threshold = NUM2DBL(argv[0]);
            if (threshold < 0.0 || threshold > QuantumRange)
            {
                rb_raise(rb_eArgError, "threshold out of range, must be >= 0.0 and < QuantumRange");
            }
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    (void) SolarizeImage(new_image, threshold);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Compare two images.
 *
 * Ruby usage:
 *   - @verbatim Image#<=> @endverbatim
 *
 * @param self this image
 * @param other other image
 * @return -1, 0, 1
 */
VALUE
Image_spaceship(VALUE self, VALUE other)
{
    Image *imageA, *imageB;
    const char *sigA, *sigB;
    int res;

    imageA = rm_check_destroyed(self);

    // If the other object isn't a Image object, then they can't be equal.
    if (!rb_obj_is_kind_of(other, Class_Image))
    {
        return Qnil;
    }

    imageB = rm_check_destroyed(other);

    (void) SignatureImage(imageA);
    (void) SignatureImage(imageB);
    sigA = rm_get_property(imageA, "signature");
    sigB = rm_get_property(imageB, "signature");
    if (!sigA || !sigB)
    {
        rb_raise(Class_ImageMagickError, "can't get image signature");
    }

    res = memcmp(sigA, sigB, 64);
    res = res > 0 ? 1 : (res < 0 ? -1 :  0);    // reduce to 1, -1, 0

    return INT2FIX(res);
}


#if defined(HAVE_SPARSECOLORIMAGE)
/**
 * Count the number of channels from the specified list are in an image. Note
 * that this method also removes invalid channels based on the image.
 *
 * No Ruby usage (internal function)
 *
 * @param image the image
 * @param channels the channels
 * @return number of channels
 */
static unsigned long
count_channels(Image *image, ChannelType *channels)
{
    unsigned long ncolors = 0UL;

    if (image->colorspace != CMYKColorspace)
    {
        *channels = (ChannelType) (*channels & ~IndexChannel);  /* remove index channels from count */
    }
    if ( image->matte == MagickFalse )
    {
        *channels = (ChannelType) (*channels & ~OpacityChannel);  /* remove matte/alpha *channels from count */
    }

    if (*channels & RedChannel)
    {
        ncolors += 1;
    }
    if (*channels & GreenChannel)
    {
        ncolors += 1;
    }
    if (*channels & BlueChannel)
    {
        ncolors += 1;
    }
    if (*channels & IndexChannel)
    {
        ncolors += 1;
    }
    if (*channels & OpacityChannel)
    {
        ncolors += 1;
    }

    return ncolors;
}
#endif


/**
 * Call SparseColorInterpolate.
 *
 * Ruby usage:
 *   - @verbatim Image#sparse_color(method, x1, y1, color) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, x2, y2, color) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, x2, y2, color, ...) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, channel) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, x2, y2, color, channel) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, x2, y2, color, ..., channel) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, channel, ...) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, x2, y2, color, channel, ...) @endverbatim
 *   - @verbatim Image#sparse_color(method, x1, y1, color, x2, y2, color, ..., channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *   - As usual, 'color' can be either a color name or a pixel
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_sparse_color(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_SPARSECOLORIMAGE)
    Image *image, *new_image;
    unsigned long x, nargs, ncolors;
    SparseColorMethod method;
    int n, exp;
    double * volatile args;
    ChannelType channels;
    MagickPixelPacket pp;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    n = argc;
    channels = extract_channels(&argc, argv);
    n -= argc;  // n is now the number of channel arguments

    // After the channel arguments have been removed, and not counting the first
    // (method) argument, the number of arguments should be a multiple of 3.
    if (argc < 4 || argc % 3 != 1)
    {
        exp = argc - 1;
        exp = (argc + 2) / 3 * 3;
        exp = max(exp, 3);
        rb_raise(rb_eArgError, "wrong number of arguments (expected at least %d, got %d)", n+exp+1,  n+argc);
    }

    // Get the method from the argument list
    VALUE_TO_ENUM(argv[0], method, SparseColorMethod);
    argv += 1;
    argc -= 1;

    // A lot of the following code is based on SparseColorOption, in wand/mogrify.c
    ncolors = count_channels(image, &channels);
    nargs = (argc / 3) * (2 + ncolors);

    // Allocate args from Ruby's memory so that GC will collect it if one of
    // the type conversions below raises an exception.
    args = ALLOC_N(double, nargs);
    memset(args, 0, nargs * sizeof(double));

    x = 0;
    n = 0;
    while (n < argc)
    {
        args[x++] = NUM2DBL(argv[n++]);
        args[x++] = NUM2DBL(argv[n++]);
        Color_to_MagickPixelPacket(NULL, &pp, argv[n++]);
        if (channels & RedChannel)
        {
            args[x++] = pp.red / QuantumRange;
        }
        if (channels & GreenChannel)
        {
            args[x++] = pp.green / QuantumRange;
        }
        if (channels & BlueChannel)
        {
            args[x++] = pp.blue / QuantumRange;
        }
        if (channels & IndexChannel)
        {
            args[x++] = pp.index / QuantumRange;
        }
        if (channels & OpacityChannel)
        {
            args[x++] = pp.opacity / QuantumRange;
        }
    }

    GetExceptionInfo(&exception);
    new_image = SparseColorImage(image, channels, method, nargs, args, &exception);
    xfree(args);
    CHECK_EXCEPTION();
    rm_ensure_result(new_image);

    return rm_image_new(new_image);

#else
    self = self;
    argc = argc;
    argv = argv;
    rm_not_implemented();
    return(VALUE)0;
#endif
}


/**
 * Splice a solid color into the part of the image specified by the x, y, width,
 * and height arguments. If the color argument is specified it must be a color
 * name or Pixel.
 *
 * Ruby usage:
 *   - @verbatim Image#splice(x, y, width, height) @endverbatim
 *   - @verbatim Image#splice(x, y, width, height, color) @endverbatim
 *
 * Notes:
 *   - Default color is the background color.
 *   - Splice is the inverse of chop
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see Image_chop
 */
VALUE
Image_splice(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    PixelPacket color, old_color;
    RectangleInfo rectangle;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 4:
            // use background color
            color = image->background_color;
            break;
        case 5:
            // Convert color argument to PixelPacket
            Color_to_PixelPacket(&color, argv[4]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 4 or 5)", argc);
            break;
    }

    rectangle.x      = NUM2LONG(argv[0]);
    rectangle.y      = NUM2LONG(argv[1]);
    rectangle.width  = NUM2ULONG(argv[2]);
    rectangle.height = NUM2ULONG(argv[3]);

    GetExceptionInfo(&exception);

    // Swap in color for the duration of this call.
    old_color = image->background_color;
    image->background_color = color;
    new_image = SpliceImage(image, &rectangle, &exception);
    image->background_color = old_color;

    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Randomly displace each pixel in a block defined by "radius".
 *
 * Ruby usage:
 *   - @verbatim Image#spread @endverbatim
 *   - @verbatim Image#spread(radius) @endverbatim
 *
 * Notes:
 *   - Default radius is 3.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_spread(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double radius = 3.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 1:
            radius = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = SpreadImage(image, radius, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    rm_ensure_result(new_image);

    (void) DestroyExceptionInfo(&exception);

    return rm_image_new(new_image);
}


DEF_ATTR_ACCESSOR(Image, start_loop, bool)


/**
 * Hide a digital watermark within the image. Recover the hidden watermark later
 * to prove that the authenticity of an image.
 *
 * Ruby usage:
 *   - @verbatim Image#stegano(watermark, offset) @endverbatim
 *
 * @param self this object
 * @param watermark_image the watermark image
 * @param offset the start position within the image to hide the watermark.
 * @return a new image
 */
VALUE
Image_stegano(VALUE self, VALUE watermark_image, VALUE offset)
{
    Image *image, *new_image;
    volatile VALUE wm_image;
    Image *watermark;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    wm_image = rm_cur_image(watermark_image);
    watermark = rm_check_destroyed(wm_image);

    image->offset = NUM2LONG(offset);

    GetExceptionInfo(&exception);
    new_image = SteganoImage(image, watermark, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Combine two images and produces a single image that is the composite of a
 * left and right image of a stereo pair. Special red-green stereo glasses are
 * required to view this effect.
 *
 * Ruby usage:
 *   - @verbatim Image#stereo(offset_image) @endverbatim
 *
 * @param self this object
 * @param offset_image_arg the other image
 * @return a new image
 */
VALUE
Image_stereo(VALUE self, VALUE offset_image_arg)
{
    Image *image, *new_image;
    volatile VALUE offset_image;
    Image *offset;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    offset_image = rm_cur_image(offset_image_arg);
    offset = rm_check_destroyed(offset_image);

    GetExceptionInfo(&exception);
    new_image = StereoImage(image, offset, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Return the image's storage class (a.k.a. storage type, class type).
 *
 * Ruby usage:
 *   - @verbatim Image#class_type @endverbatim
 *
 * Notes:
 *   - Based on Magick++'s Magick::Magick::classType
 * 
 * @param self this object
 * @return the storage class
 */
VALUE
Image_class_type(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return ClassType_new(image->storage_class);
}


/**
 * Change the image's storage class.
 *
 * Ruby usage:
 *   - @verbatim Image#class_type= @endverbatim
 *
 * Notes:
 *   - Based on Magick++'s Magick::Magick::classType
 *
 * @param self this object
 * @param new_class_type the storage class
 * @return self
 */
VALUE
Image_class_type_eq(VALUE self, VALUE new_class_type)
{
    Image *image;
    ClassType class_type;
    QuantizeInfo qinfo;

    image = rm_check_frozen(self);

    VALUE_TO_ENUM(new_class_type, class_type, ClassType);

    if (image->storage_class == PseudoClass && class_type == DirectClass)
    {
        (void) SyncImage(image);
        magick_free(image->colormap);
        image->colormap = NULL;
    }
    else if (image->storage_class == DirectClass && class_type == PseudoClass)
    {
        GetQuantizeInfo(&qinfo);
        qinfo.number_colors = QuantumRange+1;
        (void) QuantizeImage(&qinfo, image);
    }

    (void) SetImageStorageClass(image, class_type);
    return self;
}


/**
 * Replace the pixels in the specified rectangle.
 *
 * Ruby usage:
 *   - @verbatim Image#store_pixels(x,y,cols,rows,new_pixels) @endverbatim
 *
 * Notes:
 *   - Calls GetImagePixels, then SyncImagePixels after replacing the pixels.
 *   - This is the complement of get_pixels. The array object returned by
 *     get_pixels is suitable for use as the "new_pixels" argument.
 *
 * @param self this object
 * @param x_arg x position of start of region
 * @param y_arg y position of start of region
 * @param cols_arg width of region
 * @param rows_arg height of region
 * @param new_pixels the replacing pixels
 * @return self
 */
VALUE
Image_store_pixels(VALUE self, VALUE x_arg, VALUE y_arg, VALUE cols_arg
                   , VALUE rows_arg, VALUE new_pixels)
{
    Image *image;
    Pixel *pixels, *pixel;
    volatile VALUE new_pixel;
    long n, size;
    long x, y;
    unsigned long cols, rows;
    unsigned int okay;

    image = rm_check_destroyed(self);

    x = NUM2LONG(x_arg);
    y = NUM2LONG(y_arg);
    cols = NUM2ULONG(cols_arg);
    rows = NUM2ULONG(rows_arg);
    if (x < 0 || y < 0 || x+cols > image->columns || y+rows > image->rows)
    {
        rb_raise(rb_eRangeError, "geometry (%lux%lu%+ld%+ld) exceeds image bounds"
                 , cols, rows, x, y);
    }

    size = (long)(cols * rows);
    rm_check_ary_len(new_pixels, size);

    okay = SetImageStorageClass(image, DirectClass);
    rm_check_image_exception(image, RetainOnError);
    if (!okay)
    {
        rb_raise(Class_ImageMagickError, "SetImageStorageClass failed. Can't store pixels.");
    }

    // Get a pointer to the pixels. Replace the values with the PixelPackets
    // from the pixels argument.
    {
#if defined(HAVE_SYNCAUTHENTICPIXELS) || defined(HAVE_GETAUTHENTICPIXELS)
        ExceptionInfo exception;
        GetExceptionInfo(&exception);
#endif

#if defined(HAVE_GETAUTHENTICPIXELS)
        pixels = GetAuthenticPixels(image, x, y, cols, rows, &exception);
        CHECK_EXCEPTION()
#else
        pixels = GetImagePixels(image, x, y, cols, rows);
        rm_check_image_exception(image, RetainOnError);
#endif

        if (pixels)
        {
            for (n = 0; n < size; n++)
            {
                new_pixel = rb_ary_entry(new_pixels, n);
                Data_Get_Struct(new_pixel, Pixel, pixel);
                pixels[n] = *pixel;
            }
#if defined(HAVE_SYNCAUTHENTICPIXELS)
            SyncAuthenticPixels(image, &exception);
            CHECK_EXCEPTION()
#else
            SyncImagePixels(image);
            rm_check_image_exception(image, RetainOnError);
#endif
        }

#if defined(HAVE_SYNCAUTHENTICPIXELS) || defined(HAVE_GETAUTHENTICPIXELS)
        DestroyExceptionInfo(&exception);
#endif
    }

    return self;
}


/**
 * Strips an image of all profiles and comments.
 *
 * Ruby usage:
 *   - @verbatim Image#strip! @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
Image_strip_bang(VALUE self)
{
    Image *image = rm_check_frozen(self);
    (void) StripImage(image);
    rm_check_image_exception(image, RetainOnError);
    return self;
}


/**
 * Swirl the pixels about the center of the image, where degrees indicates the
 * sweep of the arc through which each pixel is moved. You get a more dramatic
 * effect as the degrees move from 1 to 360.
 *
 * Ruby usage:
 *   - @verbatim Image#swirl(degrees) @endverbatim
 *
 * @param self this object
 * @param degrees the degrees
 * @return a new image
 */
VALUE
Image_swirl(VALUE self, VALUE degrees)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    GetExceptionInfo(&exception);
    new_image = SwirlImage(image, NUM2DBL(degrees), &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Synchronize image properties with the image profiles.
 *
 * Ruby usage:
 *   - @verbatim Image#sync_profiles @endverbatim
 *
 * @param self this object
 * @return true if succeeded, otherwise false
 */
VALUE
Image_sync_profiles(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    volatile VALUE okay =  SyncImageProfiles(image) ? Qtrue : Qfalse;
    rm_check_image_exception(image, RetainOnError);
    return okay;
}


/**
 * Emulates Magick++'s floodFillTexture.
 *
 * If the FloodfillMethod method is specified, flood-fills texture across pixels
 * starting at the target pixel and matching the specified color.
 *
 * If the FillToBorderMethod method is specified, flood-fills 'texture across
 * pixels starting at the target pixel and stopping at pixels matching the
 * specified color.'
 *
 * Ruby usage:
 *   - @verbatim Image#texture_flood_fill(color, texture, x, y, method) @endverbatim
 *
 * @param self this object
 * @param color_obj the color
 * @param texture_obj the texture to fill
 * @param x_obj the x position
 * @param y_obj the y position
 * @param method_obj the method to call (FloodfillMethod or FillToBorderMethod)
 * @return a new image
 */
VALUE
Image_texture_flood_fill(VALUE self, VALUE color_obj, VALUE texture_obj
                         , VALUE x_obj, VALUE y_obj, VALUE method_obj)
{
    Image *image, *new_image;
    Image *texture_image;
    PixelPacket color;
    volatile VALUE texture;
    DrawInfo *draw_info;
    long x, y;
    PaintMethod method;

    image = rm_check_destroyed(self);

    Color_to_PixelPacket(&color, color_obj);
    texture = rm_cur_image(texture_obj);
    texture_image = rm_check_destroyed(texture);

    x = NUM2LONG(x_obj);
    y = NUM2LONG(y_obj);

    if ((unsigned long)x > image->columns || (unsigned long)y > image->rows)
    {
        rb_raise(rb_eArgError, "target out of range. %ldx%ld given, image is %lux%lu"
                 , x, y, image->columns, image->rows);
    }

    VALUE_TO_ENUM(method_obj, method, PaintMethod);
    if (method != FillToBorderMethod && method != FloodfillMethod)
    {
        rb_raise(rb_eArgError, "paint method must be FloodfillMethod or "
                 "FillToBorderMethod (%d given)", (int)method);
    }

    draw_info = CloneDrawInfo(NULL, NULL);
    if (!draw_info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    draw_info->fill_pattern = rm_clone_image(texture_image);
    new_image = rm_clone_image(image);


#if defined(HAVE_FLOODFILLPAINTIMAGE)
    {
        MagickPixelPacket color_mpp;
        MagickBooleanType invert;

        GetMagickPixelPacket(new_image, &color_mpp);
        if (method == FillToBorderMethod)
        {
            invert = MagickTrue;
            color_mpp.red   = (MagickRealType) image->border_color.red;
            color_mpp.green = (MagickRealType) image->border_color.green;
            color_mpp.blue  = (MagickRealType) image->border_color.blue;
        }
        else
        {
            invert = MagickFalse;
            color_mpp.red   = (MagickRealType) color.red;
            color_mpp.green = (MagickRealType) color.green;
            color_mpp.blue  = (MagickRealType) color.blue;
        }

        (void) FloodfillPaintImage(new_image, DefaultChannels, draw_info, &color_mpp, x, y, invert);
    }

#else
    (void) ColorFloodfillImage(new_image, draw_info, color, x, y, method);
#endif

    (void) DestroyDrawInfo(draw_info);
    rm_check_image_exception(new_image, DestroyOnError);


    return rm_image_new(new_image);
}


/**
 * Change the value of individual pixels based on the intensity of each pixel
 * compared to threshold. The result is a high-contrast, two color image.
 *
 * Ruby usage:
 *   - @verbatim Image#threshold(threshold) @endverbatim
 *
 * @param self this object
 * @param threshold the threshold
 * @return a new image
 */
VALUE
Image_threshold(VALUE self, VALUE threshold)
{
    Image *image, *new_image;

    image = rm_check_destroyed(self);
    new_image = rm_clone_image(image);

    (void) BilevelImageChannel(new_image, DefaultChannels, NUM2DBL(threshold));
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Call one of the xxxxThresholdImage methods.
 *
 * No Ruby usage (internal function)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param thresholder which xxxxThresholdImage method to call
 * @return a new image
 */
static
VALUE threshold_image(int argc, VALUE *argv, VALUE self, thresholder_t thresholder)
{
    Image *image, *new_image;
    double red, green, blue, opacity;
    char ctarg[200];

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 4:
            red     = NUM2DBL(argv[0]);
            green   = NUM2DBL(argv[1]);
            blue    = NUM2DBL(argv[2]);
            opacity = NUM2DBL(argv[3]);
            sprintf(ctarg, "%f,%f,%f,%f", red, green, blue, opacity);
            break;
        case 3:
            red     = NUM2DBL(argv[0]);
            green   = NUM2DBL(argv[1]);
            blue    = NUM2DBL(argv[2]);
            sprintf(ctarg, "%f,%f,%f", red, green, blue);
            break;
        case 2:
            red     = NUM2DBL(argv[0]);
            green   = NUM2DBL(argv[1]);
            sprintf(ctarg, "%f,%f", red, green);
            break;
        case 1:
            red     = NUM2DBL(argv[0]);
            sprintf(ctarg, "%f", red);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 4)", argc);
    }

    new_image = rm_clone_image(image);

    (thresholder)(new_image, ctarg);
    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Fast resize for thumbnail images.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Uses BoxFilter, blur attribute of input image
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see Image_thumbnail
 * @see Image_thumbnail_bang
 */
static VALUE
thumbnail(int bang, int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    unsigned long columns, rows;
    double scale_arg, drows, dcols;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);

    switch (argc)
    {
        case 2:
            columns = NUM2ULONG(argv[0]);
            rows = NUM2ULONG(argv[1]);
            if (columns == 0 || rows == 0)
            {
                rb_raise(rb_eArgError, "invalid result dimension (%lu, %lu given)", columns, rows);
            }
            break;
        case 1:
            scale_arg = NUM2DBL(argv[0]);
            if (scale_arg < 0.0)
            {
                rb_raise(rb_eArgError, "invalid scale value (%g given)", scale_arg);
            }
            drows = scale_arg * image->rows + 0.5;
            dcols = scale_arg * image->columns + 0.5;
            if (drows > (double)ULONG_MAX || dcols > (double)ULONG_MAX)
            {
                rb_raise(rb_eRangeError, "resized image too big");
            }
            rows = (unsigned long) drows;
            columns = (unsigned long) dcols;
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = ThumbnailImage(image, columns, rows, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);
}


/**
 * Fast resize for thumbnail images.
 *
 * Ruby usage:
 *   - @verbatim Image#thumbnail(scale) @endverbatim
 *   - @verbatim Image#thumbnail(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see thumbnail
 * @see Image_thumbnail_bang
 */
VALUE
Image_thumbnail(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return thumbnail(False, argc, argv, self);
}


/**
 * Fast resize for thumbnail images.
 *
 * Ruby usage:
 *   - @verbatim Image#thumbnail!(scale) @endverbatim
 *   - @verbatim Image#thumbnail!(cols, rows) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see thumbnail
 * @see Image_thumbnail
 */
VALUE
Image_thumbnail_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return thumbnail(True, argc, argv, self);
}


/**
 * The ticks_per_second attribute reader.
 *
 * Ruby usage:
 *   - @verbatim Image#ticks_per_second @endverbatim
 *
 * @param self this object
 * @return ticks per second
 */
VALUE
Image_ticks_per_second(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return INT2FIX(image->ticks_per_second);
}


/**
 * The ticks_per_second attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Image#ticks_per_second= @endverbatim
 *
 * @param self this object
 * @param tps ticks per second
 * @return self
 */
VALUE
Image_ticks_per_second_eq(VALUE self, VALUE tps)
{
    Image *image = rm_check_frozen(self);
    image->ticks_per_second = NUM2ULONG(tps);
    return self;
}


/**
 * Call TintImage.
 *
 * Ruby usage:
 *   - @verbatim Image#tint(tint, red_opacity) @endverbatim
 *   - @verbatim Image#tint(tint, red_opacity, green_opacity) @endverbatim
 *   - @verbatim Image#tint(tint, red_opacity, green_opacity, blue_opacity) @endverbatim
 *   - @verbatim Image#tint(tint, red_opacity, green_opacity, blue_opacity, alpha_opacity) @endverbatim
 *
 * Notes:
 *   - Default green_opacity is red_opacity
 *   - Default blue_opacity is red_opacity
 *   - Default alpha_opacity is 1.0
 *   - Opacity values are percentages: 0.10 -> 10%.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_tint(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    Pixel *tint;
    double red_pct_opaque, green_pct_opaque, blue_pct_opaque;
    double alpha_pct_opaque = 1.0;
    char opacity[50];
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 2:
            red_pct_opaque   = NUM2DBL(argv[1]);
            green_pct_opaque = blue_pct_opaque = red_pct_opaque;
            break;
        case 3:
            red_pct_opaque   = NUM2DBL(argv[1]);
            green_pct_opaque = NUM2DBL(argv[2]);
            blue_pct_opaque  = red_pct_opaque;
            break;
        case 4:
            red_pct_opaque     = NUM2DBL(argv[1]);
            green_pct_opaque   = NUM2DBL(argv[2]);
            blue_pct_opaque    = NUM2DBL(argv[3]);
            break;
        case 5:
            red_pct_opaque     = NUM2DBL(argv[1]);
            green_pct_opaque   = NUM2DBL(argv[2]);
            blue_pct_opaque    = NUM2DBL(argv[3]);
            alpha_pct_opaque   = NUM2DBL(argv[4]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 5)", argc);
            break;
    }

    if (red_pct_opaque < 0.0 || green_pct_opaque < 0.0
        || blue_pct_opaque < 0.0 || alpha_pct_opaque < 0.0)
    {
        rb_raise(rb_eArgError, "opacity percentages must be non-negative.");
    }

#if defined(HAVE_SNPRINTF)
    snprintf(opacity, sizeof(opacity),
#else
    sprintf(opacity,
#endif
            "%g,%g,%g,%g", red_pct_opaque*100.0, green_pct_opaque*100.0
            , blue_pct_opaque*100.0, alpha_pct_opaque*100.0);

    Data_Get_Struct(argv[0], Pixel, tint);
    GetExceptionInfo(&exception);

    new_image = TintImage(image, opacity, *tint, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Return a "blob" (a String) from the image.
 *
 * Ruby usage:
 *   - @verbatim Image#to_blob @endverbatim
 *
 * Notes:
 *   - The magick member of the Image structure determines the format of the
 *     returned blob (GIG, JPEG,  PNG, etc.)
 *
 * @param self this object
 * @return the blob
 */
VALUE
Image_to_blob(VALUE self)
{
    Image *image;
    Info *info;
    const MagickInfo *magick_info;
    volatile VALUE info_obj;
    volatile VALUE blob_str;
    void *blob = NULL;
    size_t length = 2048;       // Do what Magick++ does
    ExceptionInfo exception;

    // The user can specify the depth (8 or 16, if the format supports
    // both) and the image format by setting the depth and format
    // values in the info parm block.
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    image = rm_check_destroyed(self);

    // Copy the depth and magick fields to the Image
    if (info->depth != 0)
    {
        (void) SetImageDepth(image, info->depth);
        rm_check_image_exception(image, RetainOnError);
    }

    GetExceptionInfo(&exception);
    if (*info->magick)
    {
        (void) SetImageInfo(info, MagickTrue, &exception);
        CHECK_EXCEPTION()

        if (*info->magick == '\0')
        {
            return Qnil;
        }
        strncpy(image->magick, info->magick, sizeof(info->magick)-1);
    }

    // Fix #2844 - libjpeg exits when image is 0x0
    magick_info = GetMagickInfo(image->magick, &exception);
    CHECK_EXCEPTION()

    if (magick_info)
    {
        if (  (!rm_strcasecmp(magick_info->name, "JPEG")
               || !rm_strcasecmp(magick_info->name, "JPG"))
              && (image->rows == 0 || image->columns == 0))
        {
            rb_raise(rb_eRuntimeError, "Can't convert %lux%lu %.4s image to a blob"
                     , image->columns, image->rows, magick_info->name);
        }
    }

    rm_sync_image_options(image, info);

    blob = ImageToBlob(info, image, &length, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    if (length == 0 || !blob)
    {
        return Qnil;
    }

    blob_str = rb_str_new(blob, length);

    magick_free((void*)blob);

    return blob_str;
}


/**
 * Return a color name for the color intensity specified by the Magick::Pixel
 * argument.
 *
 * Ruby usage:
 *   - @verbatim Image#to_color(pixel) @endverbatim
 *
 * Notes:
 *   - Respects depth and matte attributes
 *
 * @param self this object
 * @param pixel_arg the pixel
 * @return the color name
 */
VALUE
Image_to_color(VALUE self, VALUE pixel_arg)
{
    Image *image;
    Pixel *pixel;
    ExceptionInfo exception;
    char name[MaxTextExtent];

    image = rm_check_destroyed(self);
    Data_Get_Struct(pixel_arg, Pixel, pixel);
    GetExceptionInfo(&exception);

    // QueryColorname returns False if the color represented by the PixelPacket
    // doesn't have a "real" name, just a sequence of hex digits. We don't care
    // about that.

    name[0] = '\0';
    (void) QueryColorname(image, pixel, AllCompliance, name, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return rb_str_new2(name);

}


/**
 * Alias for Image#number_colors.
 *
 * Ruby usage:
 *   - @verbatim Image#total_colors @endverbatim
 *
 * Notes:
 *   - This used to be a direct reference to the `total_colors' field in Image
 *     but that field is not reliable.
 *
 * @param self this object
 * @return number of unique colors
 * @see Image_number_colors
 */
VALUE
Image_total_colors(VALUE self)
{
    return Image_number_colors(self);
}


/**
 * Return value from GetImageTotalInkDensity.
 *
 * Ruby usage:
 *   - @verbatim Image#total_ink_density @endverbatim
 *
 * Notes:
 *   - Raises an exception if the image is not CMYK
 *
 * @param self this object
 * @return the total ink density
 */
VALUE
Image_total_ink_density(VALUE self)
{
    Image *image;
    double density;

    image = rm_check_destroyed(self);
    density = GetImageTotalInkDensity(image);
    rm_check_image_exception(image, RetainOnError);
    return rb_float_new(density);
}


/**
 * Call TransparentPaintImage.
 *
 * Ruby usage:
 *   - @verbatim Image#transparent(color-name) @endverbatim
 *   - @verbatim Image#transparent(color-name, opacity) @endverbatim
 *   - @verbatim Image#transparent(pixel) @endverbatim
 *   - @verbatim Image#transparent(pixel, opacity) @endverbatim
 *
 * Notes:
 *   - Default opacity is Magick::TransparentOpacity.
 *   - Can use Magick::OpaqueOpacity or Magick::TransparentOpacity, or any
 *     value >= 0 && <= QuantumRange.
 *   - Use Image#fuzz= to define the tolerance level.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_transparent(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    MagickPixelPacket color;
    Quantum opacity = TransparentOpacity;
    MagickBooleanType okay;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 2:
            opacity = APP2QUANTUM(argv[1]);
        case 1:
            Color_to_MagickPixelPacket(image, &color, argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    new_image = rm_clone_image(image);

#if defined(HAVE_TRANSPARENTPAINTIMAGE)
    okay = TransparentPaintImage(new_image, &color, opacity, MagickFalse);
#else
    okay = PaintTransparentImage(new_image, &color, opacity);
#endif
    rm_check_image_exception(new_image, DestroyOnError);
    if (!okay)
    {
        // Force exception
        DestroyImage(new_image);
        rm_magick_error("TransparentPaintImage failed with no explanation", NULL);
    }

    return rm_image_new(new_image);
}


/**
 * Call TransparentPaintImageChroma.
 *
 * Ruby usage:
 *   - @verbatim Image#transparent_chroma(low, high) @endverbatim
 *   - @verbatim Image#transparent_chroma(low, high, opacity) @endverbatim
 *   - @verbatim Image#transparent_chroma(low, high, opacity, invert) @endverbatim
 *
 * Notes:
 *   - Default opacity is TransparentOpacity
 *   - Default invert is false
 *   - Available in ImageMagick >= 6.4.5-6
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_transparent_chroma(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_TRANSPARENTPAINTIMAGECHROMA)
    Image *image, *new_image;
    Quantum opacity = TransparentOpacity;
    MagickPixelPacket low, high;
    MagickBooleanType invert = MagickFalse;
    MagickBooleanType okay;

    image = rm_check_destroyed(self);

    switch (argc)
    {
        case 4:
            invert = RTEST(argv[3]);
        case 3:
            opacity = APP2QUANTUM(argv[2]);
        case 2:
            Color_to_MagickPixelPacket(image, &high, argv[1]);
            Color_to_MagickPixelPacket(image, &low, argv[0]);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2, 3 or 4)", argc);
            break;
    }

    new_image = rm_clone_image(image);

    okay = TransparentPaintImageChroma(new_image, &low, &high, opacity, invert);
    rm_check_image_exception(new_image, DestroyOnError);
    if (!okay)
    {
        // Force exception
        DestroyImage(new_image);
        rm_magick_error("TransparentPaintImageChroma failed with no explanation", NULL);
    }

    return rm_image_new(new_image);
#else
    rm_not_implemented();
    return (VALUE)0;
    argc = argc;
    argv = argv;
    self = self;
#endif
}


/**
 * Return the name of the transparent color as a String.
 *
 * Ruby usage:
 *   - @verbatim Image#transparent_color @endverbatim
 *
 * @param self this object
 * @return the name of the transparent color
 */
VALUE
Image_transparent_color(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return rm_pixelpacket_to_color_name(image, &image->transparent_color);
}


/**
 * Set the the transparent color to the specified color spec.
 *
 * Ruby usage:
 *   - @verbatim Image#transparent_color= @endverbatim
 *
 * @param self this object
 * @param color the transparent color
 * @return self
 */
VALUE
Image_transparent_color_eq(VALUE self, VALUE color)
{
    Image *image = rm_check_frozen(self);
    Color_to_PixelPacket(&image->transparent_color, color);
    return self;
}


/**
 * Call TransposeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#transpose @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see crisscross
 * @see Image_transpose_bang
 */
VALUE
Image_transpose(VALUE self)
{
    (void) rm_check_destroyed(self);
    return crisscross(False, self, TransposeImage);
}


/**
 * Call TransposeImage.
 *
 * Ruby usage:
 *   - @verbatim Image#transpose! @endverbatim
 *
 * @param self this object
 * @return self
 * @see crisscross
 * @see Image_transpose
 */
VALUE
Image_transpose_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return crisscross(True, self, TransposeImage);
}


/**
 * Call TransverseImage.
 *
 * Ruby usage:
 *   - @verbatim Image#transverse @endverbatim
 *
 * @param self this object
 * @return a new image
 * @see crisscross
 * @see Image_transverse_bang
 */
VALUE
Image_transverse(VALUE self)
{
    (void) rm_check_destroyed(self);
    return crisscross(False, self, TransverseImage);
}

/**
 * Call TransverseImage.
 *
 * Ruby usage:
 *   - @verbatim Image#transverse! @endverbatim
 *
 * @param self this object
 * @return self
 * @see crisscross
 * @see Image_transverse_bang
 */
VALUE
Image_transverse_bang(VALUE self)
{
    (void) rm_check_frozen(self);
    return crisscross(True, self, TransverseImage);
}


/**
 * Convenient front-end to CropImage.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Respects fuzz attribute.
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see Image_trim
 * @see Image_trim_bang
 */
static VALUE
trimmer(int bang, int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;
    int reset_page = 0;

    switch (argc)
    {
        case 1:
            reset_page = RTEST(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (expecting 0 or 1, got %d)", argc);
            break;
    }

    Data_Get_Struct(self, Image, image);

    GetExceptionInfo(&exception);
    new_image = TrimImage(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (reset_page)
    {
        ResetImagePage(new_image, "0x0+0+0");
    }

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);
}


/**
 * Convenient front-end to CropImage.
 *
 * Ruby usage:
 *   - @verbatim Image#trim @endverbatim
 *   - @verbatim Image#trim(reset_page) @endverbatim
 *
 * Notes:
 *   - Default reset_page is false
 *   - Respects fuzz attribute.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see trimmer
 * @see Image_trim_bang
 */
VALUE
Image_trim(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_destroyed(self);
    return trimmer(False, argc, argv, self);
}


/**
 * Convenient front-end to CropImage.
 *
 * Ruby usage:
 *   - @verbatim Image#trim! @endverbatim
 *   - @verbatim Image#trim!(reset_page) @endverbatim
 *
 * Notes:
 *   - Default reset_page is false
 *   - Respects fuzz attribute.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see trimmer
 * @see Image_trim
 */
VALUE
Image_trim_bang(int argc, VALUE *argv, VALUE self)
{
    (void) rm_check_frozen(self);
    return trimmer(True, argc, argv, self);
}


/**
 * Get the image gravity attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#gravity @endverbatim
 *
 * @param self this object
 * @return the image gravity
 */
VALUE Image_gravity(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return GravityType_new(image->gravity);
}


/**
 * Set the image gravity attribute.
 *
 * Ruby usage:
 *   - @verbatim Image#gravity= @endverbatim
 *
 * @param self this object
 * @param gravity the image gravity
 * @return the image gravity
 */
VALUE Image_gravity_eq(VALUE self, VALUE gravity)
{
    Image *image = rm_check_frozen(self);
    VALUE_TO_ENUM(gravity, image->gravity, GravityType);
    return gravity;
}


/**
 * Call GetImageType to get the image type.
 *
 * Ruby usage:
 *   - @verbatim Image#image_type @endverbatim
 *
 * @param self this object
 * @return the image type
 */
VALUE Image_image_type(VALUE self)
{
    Image *image;
    ImageType type;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);
    type = GetImageType(image, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return ImageType_new(type);
}


/**
 * Call SetImageType to set the image type.
 *
 * Ruby usage:
 *   - @verbatim Image#image_type= @endverbatim
 *
 * @param self this object
 * @param image_type the image type
 * @return the image type
 */
VALUE Image_image_type_eq(VALUE self, VALUE image_type)
{
    Image *image;
    ImageType type;

    image = rm_check_frozen(self);
    VALUE_TO_ENUM(image_type, type, ImageType);
    SetImageType(image, type);
    return image_type;
}


/**
 * Call RemoveImageArtifact.
 *
 * Ruby usage:
 *   - @verbatim Image#undefine(artifact) @endverbatim
 *
 * Notes:
 *   - Normally a script should never call this method.
 *
 * @param self this object
 * @param artifact the artifact
 * @return self
 * @see Image_define
 */
VALUE
Image_undefine(VALUE self, VALUE artifact)
{
#if defined(HAVE_REMOVEIMAGEARTIFACT)
    Image *image;
    char *key;
    long key_l;

    image = rm_check_frozen(self);
    key = rm_str2cstr(artifact, &key_l);
    (void) RemoveImageArtifact(image, key);
    return self;
#else
    rm_not_implemented();
    artifact = artifact;
    self = self;
    return(VALUE)0;
#endif
}


/**
 * Call UniqueImageColors.
 *
 * Ruby usage:
 *   - @verbatim Image#unique_colors @endverbatim
 *
 * @param self this object
 * @return a new image
 */
VALUE
Image_unique_colors(VALUE self)
{
    Image *image, *new_image;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    GetExceptionInfo(&exception);

    new_image = UniqueImageColors(image, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Get the resolution type field.
 *
 * Ruby usage:
 *   - @verbatim Image#units @endverbatim
 *
 * @param self this object
 * @return the resolution type
 */
VALUE
Image_units(VALUE self)
{
    Image *image = rm_check_destroyed(self);
    return ResolutionType_new(image->units);
}


/**
 * Set the resolution type field.
 *
 * Ruby usage:
 *   - @verbatim Image#units= @endverbatim
 *
 * @param self this object
 * @param restype the resolution type
 * @return self
 */
VALUE
Image_units_eq(VALUE self, VALUE restype)
{
    ResolutionType units;
    Image *image = rm_check_frozen(self);

    VALUE_TO_ENUM(restype, units, ResolutionType);

    if (image->units != units)
    {
        switch (image->units)
        {
            case PixelsPerInchResolution:
                if (units == PixelsPerCentimeterResolution)
                {
                    image->x_resolution /= 2.54;
                    image->y_resolution /= 2.54;
                }
                break;

            case PixelsPerCentimeterResolution:
                if (units == PixelsPerInchResolution)
                {
                    image->x_resolution *= 2.54;
                    image->y_resolution *= 2.54;
                }
                break;

            default:
                // UndefinedResolution
                image->x_resolution = 0.0;
                image->y_resolution = 0.0;
                break;
        }

        image->units = units;
    }

    return self;
}


/**
 * Sharpen an image. "amount" is the percentage of the difference between the
 * original and the blur image that is added back into the original. "threshold"
 * is the threshold in pixels needed to apply the diffence amount.
 *
 * No Ruby usage (internal function)
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param radious the radious
 * @param sigma the sigma
 * @param amount the amount
 * @param threshold the threshold
 * @see Image_unsharp_mask
 */
static void
unsharp_mask_args(int argc, VALUE *argv, double *radius, double *sigma
                  , double *amount, double *threshold)
{
    switch (argc)
    {
        case 4:
            *threshold = NUM2DBL(argv[3]);
            if (*threshold < 0.0)
            {
                rb_raise(rb_eArgError, "threshold must be >= 0.0");
            }
        case 3:
            *amount = NUM2DBL(argv[2]);
            if (*amount <= 0.0)
            {
                rb_raise(rb_eArgError, "amount must be > 0.0");
            }
        case 2:
            *sigma = NUM2DBL(argv[1]);
            if (*sigma == 0.0)
            {
                rb_raise(rb_eArgError, "sigma must be != 0.0");
            }
        case 1:
            *radius = NUM2DBL(argv[0]);
            if (*radius < 0.0)
            {
                rb_raise(rb_eArgError, "radius must be >= 0.0");
            }
        case 0:
            break;

            // This case can't occur if we're called from Image_unsharp_mask_channel
            // because it has already raised an exception for the the argc > 4 case.
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 4)", argc);
    }
}


/**
 * Sharpen an image. "amount" is the percentage of the difference between the
 * original and the blur image that is added back into the original. "threshold"
 * is the threshold in pixels needed to apply the diffence amount.
 *
 * Ruby usage:
 *   - @verbatim Image#unsharp_mask @endverbatim
 *   - @verbatim Image#unsharp_mask(radius) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma, amount) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma, amount, threshold) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default amount is 1.0
 *   - Default threshold is 0.05
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see unsharp_mask_args
 */
VALUE
Image_unsharp_mask(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double radius = 0.0, sigma = 1.0, amount = 1.0, threshold = 0.05;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    unsharp_mask_args(argc, argv, &radius, &sigma, &amount, &threshold);

    GetExceptionInfo(&exception);
    new_image = UnsharpMaskImage(image, radius, sigma, amount, threshold, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call UnsharpMaskImageChannel.
 *
 * Ruby usage:
 *   - @verbatim Image#unsharp_mask @endverbatim
 *   - @verbatim Image#unsharp_mask(radius) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma, amount) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma, amount, threshold) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma, amount, threshold, channel) @endverbatim
 *   - @verbatim Image#unsharp_mask(radius, sigma, amount, threshold, channel, ...) @endverbatim
 *
 * Notes:
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - Default amount is 1.0
 *   - Default threshold is 0.05
 *   - Default channel is AllChannels
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see unsharp_mask_args
 */
VALUE
Image_unsharp_mask_channel(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    ChannelType channels;
    double radius = 0.0, sigma = 1.0, amount = 1.0, threshold = 0.05;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    channels = extract_channels(&argc, argv);
    if (argc > 4)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    unsharp_mask_args(argc, argv, &radius, &sigma, &amount, &threshold);

    GetExceptionInfo(&exception);
    new_image = UnsharpMaskImageChannel(image, channels, radius, sigma, amount
                                        , threshold, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Soften the edges of an image.
 *
 * Ruby usage:
 *   - @verbatim Image#vignette @endverbatim
 *   - @verbatim Image#vignette(horz_radius) @endverbatim
 *   - @verbatim Image#vignette(horz_radius, vert_radius) @endverbatim
 *   - @verbatim Image#vignette(horz_radius, vert_radius, radius) @endverbatim
 *   - @verbatim Image#vignette(horz_radius, vert_radius, radius, sigma) @endverbatim
 *
 * Notes:
 *   - Default horz_radius is image-columns*0.1+0.5
 *   - Default vert_radius is image-rows*0.1+0.5
 *   - Default radius is 0.0
 *   - Default sigma is 1.0
 *   - The outer edges of the image are replaced by the background color.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_vignette(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    long horz_radius, vert_radius;
    double radius = 0.0, sigma = 10.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);

    horz_radius = (long)(image->columns * 0.10 + 0.5);
    vert_radius = (long)(image->rows * 0.10 + 0.5);

    switch (argc)
    {
        case 4:
            sigma = NUM2DBL(argv[3]);
        case 3:
            radius = NUM2DBL(argv[2]);
        case 2:
            vert_radius = NUM2INT(argv[1]);
        case 1:
            horz_radius = NUM2INT(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 4)", argc);
            break;
    }

    GetExceptionInfo(&exception);

    new_image = VignetteImage(image, radius, sigma, horz_radius, vert_radius, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Get the VirtualPixelMethod for the image.
 *
 * Ruby usage:
 *   - @verbatim Image#virtual_pixel_method @endverbatim
 *
 * @param self this object
 * @return the VirtualPixelMethod
 */
VALUE
Image_virtual_pixel_method(VALUE self)
{
    Image *image;
    VirtualPixelMethod vpm;

    image = rm_check_destroyed(self);
    vpm = GetImageVirtualPixelMethod(image);
    rm_check_image_exception(image, RetainOnError);
    return VirtualPixelMethod_new(vpm);
}


/**
 * Set the virtual pixel method for the image.
 *
 * Ruby usage:
 *   - @verbatim Image#virtual_pixel_method= @endverbatim
 *
 * @param self this object
 * @param method the VirtualPixelMethod
 * @return self
 */
VALUE
Image_virtual_pixel_method_eq(VALUE self, VALUE method)
{
    Image *image;
    VirtualPixelMethod vpm;

    image = rm_check_frozen(self);
    VALUE_TO_ENUM(method, vpm, VirtualPixelMethod);
    (void) SetImageVirtualPixelMethod(image, vpm);
    rm_check_image_exception(image, RetainOnError);
    return self;
}


/**
 * Add a watermark to an image.
 *
 * Ruby usage:
 *   - @verbatim Image#watermark(mark) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness, saturation) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness, saturation, gravity) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness, saturation, gravity, x_off) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness, saturation, gravity, x_off, y_off) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness, saturation, x_off) @endverbatim
 *   - @verbatim Image#watermark(mark, brightness, saturation, x_off, y_off) @endverbatim
 *
 * Notes:
 *   - Default brightness is 100%
 *   - Default saturation is 100%
 *   - Default x_off is 0
 *   - Default y_off is 0
 *   - x_off and y_off can be negative, which means measure from the
 *     right/bottom of the target image.
 *
 */
VALUE
Image_watermark(int argc, VALUE *argv, VALUE self)
{
    Image *image, *overlay, *new_image;
    double src_percent = 100.0, dst_percent = 100.0;
    long x_offset = 0L, y_offset = 0L;
    char geometry[20];
    volatile VALUE ovly;

    image = rm_check_destroyed(self);

    if (argc < 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
    }

    ovly = rm_cur_image(argv[0]);
    overlay = rm_check_destroyed(ovly);

    if (argc > 3)
    {
        get_composite_offsets(argc-3, &argv[3], image, overlay, &x_offset, &y_offset);
        // There must be 3 arguments left
        argc = 3;
    }

    switch (argc)
    {
        case 3:
            dst_percent = rm_percentage(argv[2],1.0) * 100.0;
        case 2:
            src_percent = rm_percentage(argv[1],1.0) * 100.0;
        case 1:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 to 6)", argc);
            break;
    }

    blend_geometry(geometry, sizeof(geometry), src_percent, dst_percent);
    (void) CloneString(&overlay->geometry, geometry);
#if defined(HAVE_SETIMAGEARTIFACT)
    (void) SetImageArtifact(overlay,"compose:args", geometry);
#endif

    new_image = rm_clone_image(image);
    (void) CompositeImage(new_image, ModulateCompositeOp, overlay, x_offset, y_offset);

    rm_check_image_exception(new_image, DestroyOnError);

    return rm_image_new(new_image);
}


/**
 * Create a "ripple" effect in the image by shifting the pixels vertically along
 * a sine wave whose amplitude and wavelength is specified by the given
 * parameters.
 *
 * Ruby usage:
 *   - @verbatim Image#wave @endverbatim
 *   - @verbatim Image#wave(amplitude) @endverbatim
 *   - @verbatim Image#wave(amplitude, wavelength) @endverbatim
 *
 * Notes:
 *   - Default amplitude is 25.0
 *   - Default wavelength is 150.0
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 */
VALUE
Image_wave(int argc, VALUE *argv, VALUE self)
{
    Image *image, *new_image;
    double amplitude = 25.0, wavelength = 150.0;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 2:
            wavelength = NUM2DBL(argv[1]);
        case 1:
            amplitude = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
            break;
    }

    GetExceptionInfo(&exception);
    new_image = WaveImage(image, amplitude, wavelength, &exception);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Construct a "wet floor" reflection.
 *
 * Ruby usage:
 *   - @verbatim Image#wet_floor @endverbatim
 *   - @verbatim Image#wet_floor(initial) @endverbatim
 *   - @verbatim Image#wet_floor(initial, rate) @endverbatim
 *
 * Notes:
 *   - Default initial is 0.5
 *   - Default rate is 1.0
 *   - `initial' is a number between 0 and 1, inclusive, that represents the
 *     initial level of transparency. Smaller numbers are less transparent than
 *     larger numbers. 0 is fully opaque. 1.0 is fully transparent.
 *   - `rate' is the rate at which the initial level of transparency changes to
 *     complete transparency. Larger values cause the change to occur more
 *     rapidly. The resulting reflection will be shorter. Smaller values cause
 *     the change to occur less rapidly. The resulting reflection will be
 *     taller. If the rate is exactly 0 then the amount of transparency doesn't
 *     change at all.
 * 
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see http://en.wikipedia.org/wiki/Wet_floor_effect
 */
VALUE
Image_wet_floor(int argc, VALUE *argv, VALUE self)
{
    Image *image, *reflection, *flip_image;
    const PixelPacket *p;
    PixelPacket *q;
    RectangleInfo geometry;
    long x, y, max_rows;
    double initial = 0.5;
    double rate = 1.0;
    double opacity, step;
    const char *func;
    ExceptionInfo exception;

    image = rm_check_destroyed(self);
    switch (argc)
    {
        case 2:
            rate = NUM2DBL(argv[1]);
        case 1:
            initial = NUM2DBL(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
            break;
    }


    if (initial < 0.0 || initial > 1.0)
    {
        rb_raise(rb_eArgError, "Initial transparency must be in the range 0.0-1.0 (%g)", initial);
    }
    if (rate < 0.0)
    {
        rb_raise(rb_eArgError, "Transparency change rate must be >= 0.0 (%g)", rate);
    }

    initial *= TransparentOpacity;

    // The number of rows in which to transition from the initial level of
    // transparency to complete transparency. rate == 0.0 -> no change.
    if (rate > 0.0)
    {
        max_rows = (long)((double)image->rows) / (3.0 * rate);
        max_rows = (long)min((unsigned long)max_rows, image->rows);
        step =  (TransparentOpacity - initial) / max_rows;
    }
    else
    {
        max_rows = (long)image->rows;
        step = 0.0;
    }


    GetExceptionInfo(&exception);
    flip_image = FlipImage(image, &exception);
    CHECK_EXCEPTION();


    geometry.x = 0;
    geometry.y = 0;
    geometry.width = image->columns;
    geometry.height = max_rows;
    reflection = CropImage(flip_image, &geometry, &exception);
    DestroyImage(flip_image);
    CHECK_EXCEPTION();


    (void) SetImageStorageClass(reflection, DirectClass);
    rm_check_image_exception(reflection, DestroyOnError);


    reflection->matte = MagickTrue;
    opacity = initial;

    for (y = 0; y < max_rows; y++)
    {
        if (opacity > TransparentOpacity)
        {
            opacity = TransparentOpacity;
        }


#if defined(HAVE_GETVIRTUALPIXELS)
        p = GetVirtualPixels(reflection, 0, y, image->columns, 1, &exception);
#else
        p = AcquireImagePixels(reflection, 0, y, image->columns, 1, &exception);
#endif
        rm_check_exception(&exception, reflection, DestroyOnError);
        if (!p)
        {
            func = "AcquireImagePixels";
            goto error;
        }

#if defined(HAVE_QUEUEAUTHENTICPIXELS)
        q = QueueAuthenticPixels(reflection, 0, y, image->columns, 1, &exception);
#else
        q = SetImagePixels(reflection, 0, y, image->columns, 1);
#endif
        rm_check_exception(&exception, reflection, DestroyOnError);
        if (!q)
        {
            func = "SetImagePixels";
            goto error;
        }

        for (x = 0; x < (long) image->columns; x++)
        {
            q[x] = p[x];
            // Never make a pixel *less* transparent than it already is.
            q[x].opacity = max(q[x].opacity, (Quantum)opacity);
        }


#if defined(HAVE_SYNCAUTHENTICPIXELS)
        SyncAuthenticPixels(reflection, &exception);
        rm_check_exception(&exception, reflection, DestroyOnError);
#else
        SyncImagePixels(reflection);
        rm_check_image_exception(reflection, DestroyOnError);
#endif

        opacity += step;
    }


    (void) DestroyExceptionInfo(&exception);
    return rm_image_new(reflection);

    error:
    (void) DestroyExceptionInfo(&exception);
    (void) DestroyImage(reflection);
    rb_raise(rb_eRuntimeError, "%s failed on row %lu", func, y);
    return(VALUE)0;
}


/**
 * Call WhiteThresholdImage.
 *
 * Ruby usage:
 *   - @verbatim Image#white_threshold(red_channel) @endverbatim
 *   - @verbatim Image#white_threshold(red_channel, green_channel) @endverbatim
 *   - @verbatim Image#white_threshold(red_channel, green_channel, blue_channel) @endverbatim
 *   - @verbatim Image#white_threshold(red_channel, green_channel, blue_channel, opacity_channel) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new image
 * @see threshold_image
 * @see Image_black_threshold
 */
VALUE
Image_white_threshold(int argc, VALUE *argv, VALUE self)
{
    return threshold_image(argc, argv, self, WhiteThresholdImage);
}


/**
 * Copy the filename to the Info and to the Image. Add format prefix if
 * necessary. This complicated code is necessary to handle filenames like the
 * kind Tempfile.new produces, which have an "extension" in the form ".n", which
 * confuses SetMagickInfo. So we don't use SetMagickInfo any longer.
 *
 * No Ruby usage (internal function)
 *
 * @param info the Info
 * @param file the file
 */
void add_format_prefix(Info *info, VALUE file)
{
    char *filename;
    long filename_l;
    const MagickInfo *magick_info, *magick_info2;
    ExceptionInfo exception;
    char magic[MaxTextExtent];
    size_t magic_l;
    size_t prefix_l;
    char *p;

    // Convert arg to string. If an exception occurs raise an error condition.
    file = rb_rescue(rb_String, file, file_arg_rescue, file);

    filename = rm_str2cstr(file, &filename_l);

    if (*info->magick == '\0')
    {
        memset(info->filename, 0, sizeof(info->filename));
        memcpy(info->filename, filename, (size_t)min(filename_l, MaxTextExtent-1));
        return;
    }

    // If the filename starts with a prefix, and it's a valid image format
    // prefix, then check for a conflict. If it's not a valid format prefix,
    // ignore it.
    p = memchr(filename, ':', (size_t)filename_l);
    if (p)
    {
        memset(magic, '\0', sizeof(magic));
        magic_l = p - filename;
        memcpy(magic, filename, magic_l);

        GetExceptionInfo(&exception);
        magick_info = GetMagickInfo(magic, &exception);
        CHECK_EXCEPTION();
        DestroyExceptionInfo(&exception);

        if (magick_info && magick_info->module)
        {
            // We have to compare the module names because some formats have
            // more than one name. JPG and JPEG, for example.
            GetExceptionInfo(&exception);
            magick_info2 = GetMagickInfo(info->magick, &exception);
            CHECK_EXCEPTION();
            DestroyExceptionInfo(&exception);

            if (magick_info2->module && strcmp(magick_info->module, magick_info2->module) != 0)
            {
                rb_raise(rb_eRuntimeError
                         , "filename prefix `%s' conflicts with output format `%s'"
                         , magick_info->name, info->magick);
            }

            // The filename prefix already matches the specified format.
            // Just copy the filename as-is.
            memset(info->filename, 0, sizeof(info->filename));
            filename_l = min((size_t)filename_l, sizeof(info->filename));
            memcpy(info->filename, filename, (size_t)filename_l);
            return;
        }
    }

    // The filename doesn't start with a format prefix. Add the format from
    // the image info as the filename prefix.

    memset(info->filename, 0, sizeof(info->filename));
    prefix_l = min(sizeof(info->filename)-1, strlen(info->magick));
    memcpy(info->filename, info->magick, prefix_l);
    info->filename[prefix_l++] = ':';

    filename_l = min(sizeof(info->filename) - prefix_l - 1, (size_t)filename_l);
    memcpy(info->filename+prefix_l, filename, (size_t)filename_l);
    info->filename[prefix_l+filename_l] = '\0';

    return;
}


/**
 * Write the image to the file.
 *
 * Ruby usage:
 *   - @verbatim Image#write(filename) @endverbatim
 *
 * @param self this object
 * @param file the filename
 * @return self
 */
VALUE
Image_write(VALUE self, VALUE file)
{
    Image *image;
    Info *info;
    volatile VALUE info_obj;

    image = rm_check_destroyed(self);

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    if (TYPE(file) == T_FILE)
    {
        OpenFile *fptr;

        // Ensure file is open - raise error if not
        GetOpenFile(file, fptr);
        rb_io_check_writable(fptr);
        SetImageInfoFile(info, GetWriteFile(fptr));
        memset(image->filename, 0, sizeof(image->filename));
    }
    else
    {
        add_format_prefix(info, file);
        strcpy(image->filename, info->filename);
        SetImageInfoFile(info, NULL);
    }

    rm_sync_image_options(image, info);

    info->adjoin = MagickFalse;
    (void) WriteImage(info, image);
    rm_check_image_exception(image, RetainOnError);

    return self;
}


DEF_ATTR_ACCESSOR(Image, x_resolution, dbl)

DEF_ATTR_ACCESSOR(Image, y_resolution, dbl)


/**
 * Determine if the argument list is x, y, width, height
 * or
 * gravity, width, height
 * or
 * gravity, x, y, width, height
 *
 * If the 2nd or 3rd, compute new x, y values.
 *
 * The argument list can have a trailing true, false, or nil argument. If
 * present and true, after cropping reset the page fields in the image.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Call xform_image to do the cropping.
 *
 * @param bang whether the bang (!) version of the method was called
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self if bang, otherwise a new image
 * @see xform_image
 */
static VALUE
cropper(int bang, int argc, VALUE *argv, VALUE self)
{
    volatile VALUE x, y, width, height;
    unsigned long nx = 0, ny = 0;
    unsigned long columns, rows;
    int reset_page = 0;
    GravityType gravity;
    Image *image;
    VALUE cropped;

    // Check for a "reset page" trailing argument.
    if (argc >= 1)
    {
        switch (TYPE(argv[argc-1]))
        {
            case T_TRUE:
                reset_page = 1;
                // fall thru
            case T_FALSE:
            case T_NIL:
                argc -= 1;
            default:
                break;
        }
    }

    switch (argc)
    {
        case 5:
            Data_Get_Struct(self, Image, image);

            VALUE_TO_ENUM(argv[0], gravity, GravityType);

            x      = argv[1];
            y      = argv[2];
            width  = argv[3];
            height = argv[4];

            nx      = NUM2ULONG(x);
            ny      = NUM2ULONG(y);
            columns = NUM2ULONG(width);
            rows    = NUM2ULONG(height);

            switch (gravity)
            {
                case NorthEastGravity:
                case EastGravity:
                case SouthEastGravity:
                    nx = image->columns - columns - nx;
                    break;
                case NorthGravity:
                case SouthGravity:
                case CenterGravity:
                case StaticGravity:
                    nx += image->columns/2 - columns/2;
                    break;
                default:
                    break;
            }
            switch (gravity)
            {
                case SouthWestGravity:
                case SouthGravity:
                case SouthEastGravity:
                    ny = image->rows - rows - ny;
                    break;
                case EastGravity:
                case WestGravity:
                case CenterGravity:
                case StaticGravity:
                    ny += image->rows/2 - rows/2;
                    break;
                case NorthEastGravity:
                case NorthGravity:
                default:
                    break;
            }

            x = ULONG2NUM(nx);
            y = ULONG2NUM(ny);
            break;
        case 4:
            x      = argv[0];
            y      = argv[1];
            width  = argv[2];
            height = argv[3];
            break;
        case 3:

            // Convert the width & height arguments to unsigned longs.
            // Compute the x & y offsets from the gravity and then
            // convert them to VALUEs.
            VALUE_TO_ENUM(argv[0], gravity, GravityType);
            width   = argv[1];
            height  = argv[2];
            columns = NUM2ULONG(width);
            rows    = NUM2ULONG(height);

            Data_Get_Struct(self, Image, image);

            switch (gravity)
            {
                case ForgetGravity:
                case NorthWestGravity:
                    nx = 0;
                    ny = 0;
                    break;
                case NorthGravity:
                    nx = (image->columns - columns) / 2;
                    ny = 0;
                    break;
                case NorthEastGravity:
                    nx = image->columns - columns;
                    ny = 0;
                    break;
                case WestGravity:
                    nx = 0;
                    ny = (image->rows - rows) / 2;
                    break;
                case EastGravity:
                    nx = image->columns - columns;
                    ny = (image->rows - rows) / 2;
                    break;
                case SouthWestGravity:
                    nx = 0;
                    ny = image->rows - rows;
                    break;
                case SouthGravity:
                    nx = (image->columns - columns) / 2;
                    ny = image->rows - rows;
                    break;
                case SouthEastGravity:
                    nx = image->columns - columns;
                    ny = image->rows - rows;
                    break;
                case StaticGravity:
                case CenterGravity:
                    nx = (image->columns - columns) / 2;
                    ny = (image->rows - rows) / 2;
                    break;
            }

            x = ULONG2NUM(nx);
            y = ULONG2NUM(ny);
            break;
        default:
            if (reset_page)
            {
                rb_raise(rb_eArgError, "wrong number of arguments (%d for 4, 5, or 6)", argc);
            }
            else
            {
                rb_raise(rb_eArgError, "wrong number of arguments (%d for 3, 4, or 5)", argc);
            }
            break;
    }

    cropped = xform_image(bang, self, x, y, width, height, CropImage);
    if (reset_page)
    {
        Data_Get_Struct(cropped, Image, image);
        ResetImagePage(image, "0x0+0+0");
    }
    return cropped;
}


/**
 * Call one of the image transformation functions.
 *
 * No Ruby usage (internal function)
 *
 * @param bang whether the bang (!) version of the method was called
 * @param self this object
 * @param x x position of start of region
 * @param y y position of start of region
 * @param width width of region
 * @param height height of region
 * @param xformer the transformation function
 * @return self if bang, otherwise a new image
 */
static VALUE
xform_image(int bang, VALUE self, VALUE x, VALUE y, VALUE width, VALUE height, xformer_t xformer)
{
    Image *image, *new_image;
    RectangleInfo rect;
    ExceptionInfo exception;

    Data_Get_Struct(self, Image, image);
    rect.x      = NUM2LONG(x);
    rect.y      = NUM2LONG(y);
    rect.width  = NUM2ULONG(width);
    rect.height = NUM2ULONG(height);

    GetExceptionInfo(&exception);

    new_image = (xformer)(image, &rect, &exception);

    // An exception can occur in either the old or the new images
    rm_check_image_exception(image, RetainOnError);
    rm_check_exception(&exception, new_image, DestroyOnError);

    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    if (bang)
    {
        UPDATE_DATA_PTR(self, new_image);
        (void) rm_image_destroy(image);
        return self;
    }

    return rm_image_new(new_image);

}


/**
 * Remove all the ChannelType arguments from the end of the argument list.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Returns DefaultChannels if no channel arguments were found.
 *   - Returns the number of remaining arguments.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @return A ChannelType value suitable for passing into an xMagick function.
 */
ChannelType extract_channels(int *argc, VALUE *argv)
{
    volatile VALUE arg;
    ChannelType channels, ch_arg;

    channels = 0;
    while (*argc > 0)
    {
        arg = argv[(*argc)-1];

        // Stop when you find a non-ChannelType argument
        if (CLASS_OF(arg) != Class_ChannelType)
        {
            break;
        }
        VALUE_TO_ENUM(arg, ch_arg, ChannelType);
        channels |= ch_arg;
        *argc -= 1;
    }

    if (channels == 0)
    {
        channels = DefaultChannels;
    }

    return channels;
}


/**
 * Raise TypeError when an non-ChannelType object is unexpectedly encountered.
 *
 * No Ruby usage (internal function)
 *
 * @param arg the argument
 */
void
raise_ChannelType_error(VALUE arg)
{
    rb_raise(rb_eTypeError, "argument must be a ChannelType value (%s given)"
             , rb_class2name(CLASS_OF(arg)));
}



/**
 * If Magick.trace_proc is not nil, build an argument list and call the proc.
 *
 * No Ruby usage (internal function)
 *
 * @param image the image
 * @param which which operation the proc is being called for
 */
static void call_trace_proc(Image *image, const char *which)
{
    volatile VALUE trace;
    VALUE trace_args[4];

    if (rb_ivar_defined(Module_Magick, rm_ID_trace_proc) == Qtrue)
    {
        trace = rb_ivar_get(Module_Magick, rm_ID_trace_proc);
        if (!NIL_P(trace))
        {
            // Maybe the stack won't get extended until we need the space.
            char buffer[MaxTextExtent];
            int n;

            trace_args[0] = ID2SYM(rb_intern(which));

            build_inspect_string(image, buffer, sizeof(buffer));
            trace_args[1] = rb_str_new2(buffer);

            n = sprintf(buffer, "%p", (void *)image);
            buffer[n] = '\0';
            trace_args[2] = rb_str_new2(buffer+2);      // don't use leading 0x
            trace_args[3] = ID2SYM(THIS_FUNC());
            (void) rb_funcall2(trace, rm_ID_call, 4, (VALUE *)trace_args);
        }
    }

}


/**
 * Trace image creation
 *
 * No Ruby usage (internal function)
 *
 * @param image the image
 * @see call_trace_proc
 */
void rm_trace_creation(Image *image)
{
    call_trace_proc(image, "c");
}



/**
 * Destroy an image. Called from GC when all references to the image have gone
 * out of scope.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - A NULL Image pointer indicates that the image has already been destroyed
 *     by Image#destroy!
 *
 * @param img the image
 */
void rm_image_destroy(void *img)
{
    Image *image = (Image *)img;

    if (img != NULL)
    {
        call_trace_proc(image, "d");
        (void) DestroyImage(image);
    }
}


