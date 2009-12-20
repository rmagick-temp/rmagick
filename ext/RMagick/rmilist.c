/**************************************************************************//**
 * ImageList class method definitions for RMagick.
 *
 * Copyright &copy; 2002 - 2009 by Timothy P. Hunter
 *
 * Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or
 *
 * @file     rmilist.c
 * @version  $Id: rmilist.c,v 1.94 2009/12/20 02:33:33 baror Exp $
 * @author   Tim Hunter
 ******************************************************************************/

#include "rmagick.h"

static Image *clone_imagelist(Image *);
static Image *images_from_imagelist(VALUE);
static long imagelist_length(VALUE);
static long check_imagelist_length(VALUE);
static VALUE imagelist_scene_eq(VALUE, VALUE);
static void imagelist_push(VALUE, VALUE);
static VALUE ImageList_new(void);





/**
 * Repeatedly display the images in the images array to an XWindow screen. The
 * "delay" argument is the number of 1/100ths of a second (0 to 65535) to delay
 * between images.
 *
 * Ruby usage:
 *   - @verbatim ImageList#animate @endverbatim
 *   - @verbatim ImageList#animate(delay) @endverbatim
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */

VALUE
ImageList_animate(int argc, VALUE *argv, VALUE self)
{
    Image *images;
    Info *info;
    volatile VALUE info_obj;

    if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }

    // Create a new Info object to use with this call
    info_obj = rm_info_new();

    // Convert the images array to an images sequence.
    images = images_from_imagelist(self);

    if (argc == 1)
    {
        Image *img;
        unsigned int delay;

        delay = NUM2UINT(argv[0]);
        for (img = images; img; img = GetNextImageInList(img))
        {
            img->delay = delay;
        }
    }

    Data_Get_Struct(info_obj, Info, info);
    (void) AnimateImages(info, images);
    rm_check_image_exception(images, RetainOnError);
    rm_split(images);

    return self;
}


/**
 * Append all the images by calling ImageAppend.
 *
 * Ruby usage:
 *   - @verbatim ImageList#append(stack) @endverbatim
 *
 * @param self this object
 * @param stack_arg the stack of images
 * @return a Frame object for the result
 */
VALUE
ImageList_append(VALUE self, VALUE stack_arg)
{
    Image *images, *new_image;
    unsigned int stack;
    ExceptionInfo exception;

    // Convert the image array to an image sequence.
    images = images_from_imagelist(self);

    // If stack == true, stack rectangular images top-to-bottom,
    // otherwise left-to-right.
    stack = RTEST(stack_arg);

    GetExceptionInfo(&exception);
    new_image = AppendImages(images, stack, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Average all images together by calling AverageImages.
 *
 * Ruby usage:
 *   - @verbatim ImageList#average @endverbatim
 *
 * @param self this object
 * @return a Frame object for the averaged image
 */
VALUE
ImageList_average(VALUE self)
{
    Image *images, *new_image;
    ExceptionInfo exception;

    // Convert the images array to an images sequence.
    images = images_from_imagelist(self);

    GetExceptionInfo(&exception);
    new_image = AverageImages(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call CoalesceImages.
 *
 * Ruby usage:
 *   - @verbatim ImageList#coalesce @endverbatim
 *
 * Notes:
 *   - Respects the delay, matte, and start_loop fields in each image.
 *
 * @param self this object
 * @return a new Image with the coalesced image sequence return stored in the
 * images array
 */
VALUE
ImageList_coalesce(VALUE self)
{
    Image *images, *new_images;
    ExceptionInfo exception;

    // Convert the image array to an image sequence.
    images = images_from_imagelist(self);

    GetExceptionInfo(&exception);
    new_images = CoalesceImages(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/**
 * Equivalent to convert's -layers composite option.
 *
 * Ruby usage:
 *   - @verbatim ImageList#composite_layers(images) @endverbatim
 *   - @verbatim ImageList#composite_layers(images,operator) @endverbatim
 *
 * Notes:
 *   - Default operator is OverCompositeOp
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new imagelist
 * @see mogrify.c in ImageMagick
 */
VALUE
ImageList_composite_layers(int argc, VALUE *argv, VALUE self)
{
    volatile VALUE source_images;
    Image *dest, *source, *new_images;
    RectangleInfo geometry;
    CompositeOperator operator = OverCompositeOp;
    ExceptionInfo exception;

    switch (argc)
    {
        case 2:
            VALUE_TO_ENUM(argv[1], operator, CompositeOperator);
        case 1:
            source_images = argv[0];
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (expected 1 or 2, got %d)", argc);
            break;
    }

    // Convert ImageLists to image sequences.
    dest = images_from_imagelist(self);
    new_images = clone_imagelist(dest);
    rm_split(dest);

    source = images_from_imagelist(source_images);

    SetGeometry(new_images,&geometry);
    (void) ParseAbsoluteGeometry(new_images->geometry, &geometry);

    geometry.width  = source->page.width != 0 ? source->page.width : source->columns;
    geometry.height = source->page.height != 0 ? source->page.height : source->rows;
    GravityAdjustGeometry(new_images->page.width  != 0 ? new_images->page.width  : new_images->columns
                        , new_images->page.height != 0 ? new_images->page.height : new_images->rows
                        , new_images->gravity, &geometry);

    GetExceptionInfo(&exception);
    CompositeLayers(new_images, operator, source, geometry.x, geometry.y, &exception);
    rm_split(source);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    return rm_imagelist_from_images(new_images);
}


/**
 * Compare each image with the next in a sequence and returns the maximum
 * bounding region of any pixel differences it discovers.
 *
 * Ruby usage:
 *   - @verbatim ImageList#deconstruct @endverbatim
 *
 * @param self this object
 * @return a new imagelist
 */
VALUE
ImageList_deconstruct(VALUE self)
{
    Image *new_images, *images;
    ExceptionInfo exception;

    images = images_from_imagelist(self);
    GetExceptionInfo(&exception);
    new_images = DeconstructImages(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/**
 * Display all the images to an X window screen.
 *
 * Ruby usage:
 *   - @verbatim ImageList#display @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
ImageList_display(VALUE self)
{
    Image *images;
    Info *info;
    volatile VALUE info_obj;

    // Create a new Info object to use with this call
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    // Convert the images array to an images sequence.
    images = images_from_imagelist(self);

    (void) DisplayImages(info, images);
    rm_split(images);
    rm_check_image_exception(images, RetainOnError);

    return self;
}


/**
 * Merge all the images into a single image.
 *
 * Ruby usage:
 *   - @verbatim ImageList#flatten_images @endverbatim
 *
 * Notes:
 *   - Can't use "flatten" because that's an Array method
 *
 * @param self this object
 * @return the new image
 */
VALUE
ImageList_flatten_images(VALUE self)
{
    Image *images, *new_image;
    ExceptionInfo exception;

    images = images_from_imagelist(self);
    GetExceptionInfo(&exception);

#if defined(HAVE_ENUM_FLATTENLAYER)
    new_image = MergeImageLayers(images, FlattenLayer, &exception);
#else
    new_image = FlattenImages(images, &exception);
#endif

    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Apply fx on the images.
 *
 * Ruby usage:
 *   - @verbatim ImageList#fx(expression) @endverbatim
 *   - @verbatim ImageList#fx(expression, channel) @endverbatim
 *   - @verbatim ImageList#fx(expression, channel, ...) @endverbatim
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
ImageList_fx(int argc, VALUE *argv, VALUE self)
{
    Image *images, *new_image;
    char *expression;
    ChannelType channels;
    ExceptionInfo exception;


    channels = extract_channels(&argc, argv);

    // There must be exactly 1 remaining argument.
    if (argc == 0)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (0 for 1 or more)");
    }
    else if (argc > 1)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    expression = StringValuePtr(argv[0]);

    images = images_from_imagelist(self);
    GetExceptionInfo(&exception);
    new_image = FxImageChannel(images, channels, expression, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Call MapImages.
 *
 * Ruby usage:
 *   - @verbatim ImageList#map(reference) @endverbatim
 *   - @verbatim ImageList#map(reference, dither) @endverbatim
 *
 * Notes:
 *   - Default dither is false
 *   - Sets \@scene to self.scene
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new ImageList with mapped images.
 */
VALUE
ImageList_map(int argc, VALUE *argv, VALUE self)
{
    Image *images, *new_images = NULL;
    Image *map;
    unsigned int dither = MagickFalse;
    volatile VALUE scene, new_imagelist, t;
    ExceptionInfo exception;

#if defined(HAVE_REMAPIMAGES)
    rb_warning("ImageList#map is deprecated. Use ImageList#remap instead.");
#endif

    switch (argc)
    {
        case 2:
            dither = RTEST(argv[1]);
        case 1:
            t = rm_cur_image(argv[0]);
            map = rm_check_destroyed(t);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }


    // Convert image array to image sequence, clone image sequence.
    GetExceptionInfo(&exception);

    images = images_from_imagelist(self);
    new_images = CloneImageList(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    // Call ImageMagick
    (void) MapImages(new_images, map, dither);
    rm_check_image_exception(new_images, DestroyOnError);

    // Set @scene in new ImageList object to same value as in self.
    new_imagelist = rm_imagelist_from_images(new_images);
    scene = rb_iv_get(self, "@scene");
    (void) imagelist_scene_eq(new_imagelist, scene);

    return new_imagelist;
}


/**
 * Call MontageImages.
 *
 * Ruby usage:
 *   - @verbatim ImageList#montage <{parm block}> @endverbatim
 *
 * Notes:
 *   - Creates Montage object, yields to block if present in Montage object's
 *     scope.
 *
 * @param self this object
 * @return a new image list
 */
VALUE
ImageList_montage(VALUE self)
{
    volatile VALUE montage_obj;
    Montage *montage;
    Image *new_images, *images;
    ExceptionInfo exception;

    // Create a new instance of the Magick::Montage class
    montage_obj = rm_montage_new();
    if (rb_block_given_p())
    {
        // Run the block in the instance's context, allowing the app to modify the
        // object's attributes.
        (void) rb_obj_instance_eval(0, NULL, montage_obj);
    }

    Data_Get_Struct(montage_obj, Montage, montage);

    images = images_from_imagelist(self);

    // If app specified a non-default composition operator, use it for all images.
    if (montage->compose != UndefinedCompositeOp)
    {
        Image *i;
        for (i = images; i; i = GetNextImageInList(i))
        {
            i->compose = montage->compose;
        }
    }

    GetExceptionInfo(&exception);

    // MontageImage can return more than one image.
    new_images = MontageImages(images, montage->info, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/**
 * Requires a minimum of two images. The first image is transformed into the
 * second by a number of intervening images as specified by "number_images".
 *
 * Ruby usage:
 *   - @verbatim ImageList#morph(number_images) @endverbatim
 *
 * Notes:
 *   - Sets \@scenes to 0
 *
 * @param self this object
 * @param nimages the number of images
 * @return a new image list with the images array set to the morph sequence.
 */
VALUE
ImageList_morph(VALUE self, VALUE nimages)
{
    Image *images, *new_images;
    ExceptionInfo exception;
    long number_images;


    // Use a signed long so we can test for a negative argument.
    number_images = NUM2LONG(nimages);
    if (number_images <= 0)
    {
        rb_raise(rb_eArgError, "number of intervening images must be > 0");
    }

    GetExceptionInfo(&exception);
    images = images_from_imagelist(self);
    new_images = MorphImages(images, (unsigned long)number_images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/**
 * Merge all the images into a single image.
 *
 * Ruby usage:
 *   - @verbatim ImageList#mosaic @endverbatim
 *
 * @param self this object
 * @return the new image
 */
VALUE
ImageList_mosaic(VALUE self)
{
    Image *images, *new_image;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    images = images_from_imagelist(self);

#if defined(HAVE_ENUM_MOSAICLAYER)
    new_image = MergeImageLayers(images, MosaicLayer, &exception);
#else
    new_image = MosaicImages(images, &exception);
#endif

    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}


/**
 * Equivalent to -layers option in ImageMagick 6.2.6.
 *
 * Ruby usage:
 *   - @verbatim ImageList#optimize_layers(method) @endverbatim
 *
 * @param self this object
 * @param method the method to use
 * @return a new imagelist
 */
VALUE
ImageList_optimize_layers(VALUE self, VALUE method)
{
    Image *images, *new_images, *new_images2;
    LAYERMETHODTYPE mthd;
    ExceptionInfo exception;
    QuantizeInfo quantize_info;

    new_images2 = NULL;     // defeat "unused variable" message

    GetExceptionInfo(&exception);
#if defined(HAVE_TYPE_IMAGELAYERMETHOD)
    VALUE_TO_ENUM(method, mthd, ImageLayerMethod);
#else
    VALUE_TO_ENUM(method, mthd, MagickLayerMethod);
#endif
    images = images_from_imagelist(self);

    switch (mthd)
    {
        case CoalesceLayer:
            new_images = CoalesceImages(images, &exception);
            break;
        case DisposeLayer:
            new_images = DisposeImages(images, &exception);
            break;
        case OptimizeTransLayer:
            new_images = clone_imagelist(images);
            OptimizeImageTransparency(new_images, &exception);
            break;
        case RemoveDupsLayer:
            new_images = clone_imagelist(images);
            RemoveDuplicateLayers(&new_images, &exception);
            break;
        case RemoveZeroLayer:
            new_images = clone_imagelist(images);
            RemoveZeroDelayLayers(&new_images, &exception);
            break;
        case CompositeLayer:
            rm_split(images);
            rb_raise(rb_eNotImpError, "Magick::CompositeLayer is not supported. Use the composite_layers method instead.");
            break;
            // In 6.3.4-ish, OptimizeImageLayer replaced OptimizeLayer
        case OptimizeImageLayer:
            new_images = OptimizeImageLayers(images, &exception);
            break;
            // and OptimizeLayer became a "General Purpose, GIF Animation Optimizer" (ref. mogrify.c)
        case OptimizeLayer:
            new_images = CoalesceImages(images, &exception);
            rm_split(images);
            rm_check_exception(&exception, new_images, DestroyOnError);
            new_images2 = OptimizeImageLayers(new_images, &exception);
            DestroyImageList(new_images);
            rm_check_exception(&exception, new_images2, DestroyOnError);
            new_images = new_images2;
            OptimizeImageTransparency(new_images, &exception);
            rm_check_exception(&exception, new_images, DestroyOnError);
            // mogrify supports -dither here. We don't.
#if defined(HAVE_REMAPIMAGE)
            GetQuantizeInfo(&quantize_info);
            (void) RemapImages(&quantize_info, new_images, NULL);
#else
            (void) MapImages(new_images, NULL, 0);
#endif
            break;
        case OptimizePlusLayer:
            new_images = OptimizePlusImageLayers(images, &exception);
            break;
        case CompareAnyLayer:
        case CompareClearLayer:
        case CompareOverlayLayer:
            new_images = CompareImageLayers(images, mthd, &exception);
            break;
#if defined(HAVE_ENUM_MOSAICLAYER)
        case MosaicLayer:
            new_images = MergeImageLayers(images, mthd, &exception);
            break;
#endif
#if defined(HAVE_ENUM_FLATTENLAYER)
        case FlattenLayer:
            new_images = MergeImageLayers(images, mthd, &exception);
            break;
#endif
#if defined(HAVE_ENUM_MERGELAYER)
        case MergeLayer:
            new_images = MergeImageLayers(images, mthd, &exception);
            break;
#endif
#if defined(HAVE_ENUM_TRIMBOUNDSLAYER)
        case TrimBoundsLayer:
            new_images = MergeImageLayers(images, mthd, &exception);
            break;
#endif
        default:
            rm_split(images);
            rb_raise(rb_eArgError, "undefined layer method");
            break;
    }

    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/**
 * Create a new ImageList object with no images.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - this simply calls ImageList.new() in RMagick.rb
 *
 * @return a new imagelist
 */
static VALUE
ImageList_new(void)
{
    return rb_funcall(Class_ImageList, rm_ID_new, 0);
}


/**
 * Construct a new imagelist object from a list of images.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Sets \@scene to 0.
 *
 * @param images the images
 * @return a new imagelist
 * @see images_from_imagelist
 */
VALUE
rm_imagelist_from_images(Image *images)
{
    volatile VALUE new_imagelist;
    Image *image;

    if (!images)
    {
        rb_bug("rm_imagelist_from_images called with NULL argument");
    }

    new_imagelist = ImageList_new();

    while (images)
    {
        image = RemoveFirstImageFromList(&images);
        imagelist_push(new_imagelist, rm_image_new(image));
    }

    (void) rb_iv_set(new_imagelist, "@scene", INT2FIX(0));
    return new_imagelist;
}


/**
 * Convert an array of Image *s to an ImageMagick scene sequence (i.e. a
 * doubly-linked list of Images).
 *
 * No Ruby usage (internal function)
 *
 * @param imagelist the imagelist
 * @return a pointer to the head of the scene sequence list
 * @see rm_imagelist_from_images
 */
static Image *
images_from_imagelist(VALUE imagelist)
{
    long x, len;
    Image *head = NULL;
    volatile VALUE images, t;

    len = check_imagelist_length(imagelist);

    images = rb_iv_get(imagelist, "@images");
    for (x = 0; x < len; x++)
    {
        Image *image;

        t = rb_ary_entry(images, x);
        image = rm_check_destroyed(t);
        AppendImageToList(&head, image);
    }

    return head;
}


/**
 * \@scene attribute writer.
 *
 * No Ruby usage (internal function)
 *
 * @param imagelist the imagelist
 * @param scene the scene
 * @return the scene
 */
static VALUE
imagelist_scene_eq(VALUE imagelist, VALUE scene)
{
    rb_check_frozen(imagelist);
    (void) rb_iv_set(imagelist, "@scene", scene);
    return scene;
}


/**
 * return the # of images in an imagelist.
 *
 * No Ruby usage (internal function)
 *
 * @param imagelist the imagelist
 * @return the number of images
 */
static long
imagelist_length(VALUE imagelist)
{
    volatile VALUE images = rb_iv_get(imagelist, "@images");
    return RARRAY_LEN(images);
}


/**
 * Raise exception if imagelist is emtpy.
 *
 * No Ruby usage (internal function)
 *
 * @param imagelist the imagelist
 * @return the number of images
 * @throw ArgError
 */
static long
check_imagelist_length(VALUE imagelist)
{
    long len = imagelist_length(imagelist);

    if (len == 0)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

    return len;
}


/**
 * Push an image onto the end of the imagelist.
 *
 * No Ruby usage (internal function)
 *
 * @param imagelist the imagelist
 * @param image the image
 */
static void
imagelist_push(VALUE imagelist, VALUE image)
{
    rb_check_frozen(imagelist);
    (void) rb_funcall(imagelist, rm_ID_push, 1, image);
}


/**
 * Clone a list of images, handle errors.
 *
 * No Ruby usage (internal function)
 *
 * @param images the images
 * @return a new array of images
 */
static Image *
clone_imagelist(Image *images)
{
    Image *new_imagelist = NULL, *image, *clone;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);

    image = GetFirstImageInList(images);
    while (image)
    {
        clone = CloneImage(image, 0, 0, MagickTrue, &exception);
        rm_check_exception(&exception, new_imagelist, DestroyOnError);
        AppendImageToList(&new_imagelist, clone);
        image = GetNextImageInList(image);
    }

    (void) DestroyExceptionInfo(&exception);
    return new_imagelist;
}


/**
 * Call QuantizeImages.
 *
 * Ruby usage:
 *   - @verbatim ImageList#quantize @endverbatim
 *   - @verbatim ImageList#quantize(number_colors) @endverbatim
 *   - @verbatim ImageList#quantize(number_colors, colorspace) @endverbatim
 *   - @verbatim ImageList#quantize(number_colors, colorspace, dither) @endverbatim
 *   - @verbatim ImageList#quantize(number_colors, colorspace, dither, tree_depth) @endverbatim
 *   - @verbatim ImageList#quantize(number_colors, colorspace, dither, tree_depth, measure_error) @endverbatim
 *
 * Notes:
 *   - Default number_colors is 256
 *   - Default coorspace is Magick::RGBColorsapce
 *   - Default dither is true
 *   - Default tree_depth is 0
 *   - Default measure_error is false
 *   - Sets \@scene to the same value as self.scene
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return a new ImageList with quantized images
 */
VALUE
ImageList_quantize(int argc, VALUE *argv, VALUE self)
{
    Image *images, *new_images;
    Image *new_image;
    QuantizeInfo quantize_info;
    ExceptionInfo exception;
    volatile VALUE new_imagelist, scene;

    GetQuantizeInfo(&quantize_info);

    switch (argc)
    {
        case 5:
            quantize_info.measure_error = (MagickBooleanType) RTEST(argv[4]);
        case 4:
            quantize_info.tree_depth = (unsigned long)NUM2INT(argv[3]);
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
            quantize_info.number_colors = NUM2ULONG(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 5)", argc);
            break;
    }


    // Convert image array to image sequence, clone image sequence.
    GetExceptionInfo(&exception);
    images = images_from_imagelist(self);
    new_images = CloneImageList(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);


    (void) QuantizeImages(&quantize_info, new_images);
    rm_check_exception(&exception, new_images, DestroyOnError);

    // Create new ImageList object, convert mapped image sequence to images,
    // append to images array.
    new_imagelist = ImageList_new();
    while ((new_image = RemoveFirstImageFromList(&new_images)))
    {
        imagelist_push(new_imagelist, rm_image_new(new_image));
    }

    // Set @scene in new ImageList object to same value as in self.
    scene = rb_iv_get(self, "@scene");
    (void) rb_iv_set(new_imagelist, "@scene", scene);

    return new_imagelist;
}


/**
 * Call RemapImages.
 *
 * Ruby usage:
 *   - @verbatim ImageList#remap @endverbatim
 *   - @verbatim ImageList#remap(remap_image) @endverbatim
 *   - @verbatim ImageList#remap(remap_image, dither_method) @endverbatim
 *
 * Notes:
 *   - Default remap_image is nil
 *   - Default dither_method is RiemersmaDitherMethod
 *   - Modifies images in-place.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @see Image_remap
 */
VALUE
ImageList_remap(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_REMAPIMAGES) || defined(HAVE_AFFINITYIMAGES)
    Image *images, *remap_image = NULL;
    QuantizeInfo quantize_info;


    if (argc > 0 && argv[0] != Qnil)
    {
        volatile VALUE t = rm_cur_image(argv[0]);
        remap_image = rm_check_destroyed(t);
    }

    GetQuantizeInfo(&quantize_info);

    if (argc > 1)
    {
        VALUE_TO_ENUM(argv[1], quantize_info.dither_method, DitherMethod);
        quantize_info.dither = MagickTrue;
    }
    if (argc > 2)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
    }

    images = images_from_imagelist(self);

#if defined(HAVE_REMAPIMAGE)
    (void) RemapImages(&quantize_info, images, remap_image);
#else
    (void) AffinityImages(&quantize_info, images, remap_image);
#endif
    rm_check_image_exception(images, RetainOnError);
    rm_split(images);

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
 * Return the imagelist as a blob (a String).
 *
 * Ruby usage:
 *   - @verbatim ImageList#to_blob @endverbatim
 *
 * Notes:
 *   - Runs an info parm block if present - the user can specify the image
 *     format and depth
 *
 * @param self this object
 * @return the blob
 */
VALUE
ImageList_to_blob(VALUE self)
{
    Image *images, *image;
    Info *info;
    volatile VALUE info_obj;
    volatile VALUE blob_str;
    void *blob = NULL;
    size_t length = 0;
    ExceptionInfo exception;

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    // Convert the images array to an images sequence.
    images = images_from_imagelist(self);

    GetExceptionInfo(&exception);
    (void) SetImageInfo(info, MagickTrue, &exception);
    rm_check_exception(&exception, images, RetainOnError);

    if (*info->magick != '\0')
    {
        Image *img;
        for (img = images; img; img = GetNextImageInList(img))
        {
            strncpy(img->magick, info->magick, sizeof(info->magick)-1);
        }
    }

    for (image = images; image; image = GetNextImageInList(image))
    {
        rm_sync_image_options(image, info);
    }

    // Unconditionally request multi-images support. The worst that
    // can happen is that there's only one image or the format
    // doesn't support multi-image files.
    info->adjoin = MagickTrue;
    blob = ImagesToBlob(info, images, &length, &exception);
    if (blob && exception.severity >= ErrorException)
    {
        magick_free((void*)blob);
        blob = NULL;
        length = 0;
    }
    rm_split(images);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);


    if (length == 0 || !blob)
    {
        return Qnil;
    }

    blob_str = rb_str_new(blob, (long)length);
    magick_free((void*)blob);

    return blob_str;
}


/**
 * Write all the images to the specified file. If the file format supports
 * multi-image files, and the 'images' array contains more than one image, then
 * the images will be written as a single multi-image file. Otherwise each image
 * will be written to a separate file.
 *
 * Ruby usage:
 *   - @verbatim ImageList#write(file) @endverbatim
 *
 * @param self this object
 * @param file the file
 * @return self
 */
VALUE
ImageList_write(VALUE self, VALUE file)
{
    Image *images, *img;
    Info *info;
    const MagickInfo *m;
    volatile VALUE info_obj;
    unsigned long scene;
    ExceptionInfo exception;

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);


    if (TYPE(file) == T_FILE)
    {
        OpenFile *fptr;

        // Ensure file is open - raise error if not
        GetOpenFile(file, fptr);
        SetImageInfoFile(info, GetReadFile(fptr));
    }
    else
    {
        add_format_prefix(info, file);
        SetImageInfoFile(info, NULL);
    }

    // Convert the images array to an images sequence.
    images = images_from_imagelist(self);

    // Copy the filename into each image. Set a scene number to be used if
    // writing multiple files. (Ref: ImageMagick's utilities/convert.c
    for (scene = 0, img = images; img; img = GetNextImageInList(img))
    {
        img->scene = scene++;
        strcpy(img->filename, info->filename);
    }

    // Find out if the format supports multi-images files.
    GetExceptionInfo(&exception);
    (void) SetImageInfo(info, MagickTrue, &exception);
    rm_check_exception(&exception, images, RetainOnError);

    m = GetMagickInfo(info->magick, &exception);
    rm_check_exception(&exception, images, RetainOnError);
    (void) DestroyExceptionInfo(&exception);

    // Tell WriteImage if we want a multi-images file.
    if (imagelist_length(self) > 1L && m->adjoin)
    {
        info->adjoin = MagickTrue;
    }

    for (img = images; img; img = GetNextImageInList(img))
    {
        rm_sync_image_options(img, info);
        (void) WriteImage(info, img);
        // images will be split before raising an exception
        rm_check_image_exception(images, RetainOnError);
        if (info->adjoin)
        {
            break;
        }
    }

    rm_split(images);
    return self;
}
