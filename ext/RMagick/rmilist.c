/* $Id: rmilist.c,v 1.63 2007/12/24 19:08:08 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2007 by Timothy P. Hunter
| Name:     rmilist.c
| Author:   Tim Hunter
| Purpose:  ImageList class method definitions for RMagick
\============================================================================*/

#include "rmagick.h"


/*
    Method:     ImageList#animate(<delay>)
    Purpose:    repeatedly display the images in the images array to an XWindow
                screen. The "delay" argument is the number of 1/100ths of a
                second (0 to 65535) to delay between images.
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

    // Convert the images array to an images sequence.
    images = rm_images_from_imagelist(self);

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


    // Create a new Info object to use with this call
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    (void) AnimateImages(info, images);
    rm_check_image_exception(images, RetainOnError);
    rm_split(images);

    return self;
}

/*
    Method:     ImageList#append(stack)
    Purpose:    Append all the images by calling ImageAppend
    Returns:    an Frame object for the result
*/
VALUE
ImageList_append(VALUE self, VALUE stack_arg)
{
    Image *images, *new_image;
    unsigned int stack;
    ExceptionInfo exception;

    // Convert the image array to an image sequence.
    images = rm_images_from_imagelist(self);

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

/*
    Method:     ImageList#average
    Purpose:    Average all images together by calling AverageImages
    Returns:    an Frame object for the averaged image
*/
VALUE
ImageList_average(VALUE self)
{
    Image *images, *new_image;
    ExceptionInfo exception;

    // Convert the images array to an images sequence.
    images = rm_images_from_imagelist(self);

    GetExceptionInfo(&exception);
    new_image = AverageImages(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}

/*
    Method:     ImageList#coalesce
    Purpose:    call CoalesceImages
    Returns:    a new Image with the coalesced image sequence
                stored in the images array
    Notes:      respects the delay, matte, and start_loop fields
                in each image.
*/
VALUE
ImageList_coalesce(VALUE self)
{
    Image *images, *new_images;
    ExceptionInfo exception;

    // Convert the image array to an image sequence.
    images = rm_images_from_imagelist(self);

    GetExceptionInfo(&exception);
    new_images = CoalesceImages(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/*
    Method:     ImageList#composite_layers
    Purpose:    Equivalent to convert's -layers composite option
    Notes:      see mogrify.c
*/
VALUE
ImageList_composite_layers(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_COMPOSITELAYERS)
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
    dest = rm_images_from_imagelist(self);
    new_images = rm_clone_imagelist(dest);
    rm_split(dest);

    source = rm_images_from_imagelist(source_images);

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

#else

    self = self;
    source_images = source_images;
    rm_not_implemented();
    return (VALUE)0;

#endif
}


/*
    Method:     ImageList#deconstruct
    Purpose:    compares each image with the next in a sequence and returns
                the maximum bounding region of any pixel differences it
                discovers.
    Returns:    a new imagelist
*/
VALUE
ImageList_deconstruct(VALUE self)
{
    Image *new_images, *images;
    ExceptionInfo exception;

    images = rm_images_from_imagelist(self);
    GetExceptionInfo(&exception);
    new_images = DeconstructImages(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}

/*
    Method:     ImageList#display
    Purpose:    Display all the images to an X window screen.
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
    images = rm_images_from_imagelist(self);

    (void) DisplayImages(info, images);
    rm_split(images);
    rm_check_image_exception(images, RetainOnError);

    return self;
}

/*
    Method:     ImageList#flatten_images
    Purpose:    merge all the images into a single image
    Returns:    the new image
    Notes:      Can't use "flatten" because that's an Array method
*/
VALUE
ImageList_flatten_images(VALUE self)
{
    Image *images, *new_image;
    ExceptionInfo exception;

    images = rm_images_from_imagelist(self);
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


/*
    Method:     ImageList#fx(expression[, channel...])
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

    images = rm_images_from_imagelist(self);
    GetExceptionInfo(&exception);
    new_image = FxImageChannel(images, channels, expression, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_image, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_image);

    return rm_image_new(new_image);
}



/*
    Method:     ImageList#map(reference, dither=false)
    Purpose:    Call MapImages
    Returns:    a new ImageList with mapped images. @scene is set to self.scene
*/
VALUE
ImageList_map(int argc, VALUE *argv, VALUE self)
{
    Image *images, *new_images = NULL;
    Image *map;
    unsigned int dither = False;
    volatile VALUE scene, new_imagelist, t;
    ExceptionInfo exception;

    switch (argc)
    {
        case 2:
            dither = RTEST(argv[1]);
        case 1:
            t = ImageList_cur_image(argv[0]);
            map = rm_check_destroyed(t);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }


    if (rm_imagelist_length(self) == 0L)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

    // Convert image array to image sequence, clone image sequence.
    GetExceptionInfo(&exception);

    images = rm_images_from_imagelist(self);
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
    (void) rm_imagelist_scene_eq(new_imagelist, scene);

    return new_imagelist;
}

/*
    Method:     ImageList#montage <{parm block}>
    Purpose:    Call MontageImages
    Notes:      Creates Montage object, yields to block if present
                in Montage object's scope.
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

    images = rm_images_from_imagelist(self);

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

/*
    Method:     ImageList#morph(number_images)
    Purpose:    requires a minimum of two images. The first image is
                transformed into the second by a number of intervening images
                as specified by "number_images".
    Returns:    a new Image with the images array set to the morph sequence.
                @scenes = 0
*/
VALUE
ImageList_morph(VALUE self, VALUE nimages)
{
    Image *images, *new_images;
    ExceptionInfo exception;
    long number_images;

    if (rm_imagelist_length(self) < 1L)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

    // Use a signed long so we can test for a negative argument.
    number_images = NUM2LONG(nimages);
    if (number_images <= 0)
    {
        rb_raise(rb_eArgError, "number of intervening images must be > 0");
    }

    GetExceptionInfo(&exception);
    images = rm_images_from_imagelist(self);
    new_images = MorphImages(images, (unsigned long)number_images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}

/*
    Method:     ImageList#mosaic
    Purpose:    merge all the images into a single image
    Returns:    the new image
*/
VALUE
ImageList_mosaic(VALUE self)
{
    Image *images, *new_image;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    images = rm_images_from_imagelist(self);

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


/*
    Method:     ImageList#optimize_layers
    Purpose:    Equivalent to -layers option in 6.2.6
    Returns:    a new imagelist
*/
VALUE
ImageList_optimize_layers(VALUE self, VALUE method)
{
    Image *images, *new_images, *new_images2;
    MagickLayerMethod mthd;
    ExceptionInfo exception;

    new_images2 = NULL;     // defeat "unused variable" message

    GetExceptionInfo(&exception);
    VALUE_TO_ENUM(method, mthd, MagickLayerMethod);
    images = rm_images_from_imagelist(self);

    switch (mthd)
    {
        case CoalesceLayer:
            new_images = CoalesceImages(images, &exception);
            break;
        case DisposeLayer:
            new_images = DisposeImages(images, &exception);
            break;
#if defined(HAVE_ENUM_OPTIMIZETRANSLAYER)
        case OptimizeTransLayer:
            new_images = rm_clone_imagelist(images);
            OptimizeImageTransparency(new_images, &exception);
            break;
#endif
#if defined(HAVE_ENUM_REMOVEDUPSLAYER)
        case RemoveDupsLayer:
            new_images = rm_clone_imagelist(images);
            RemoveDuplicateLayers(&new_images, &exception);
            break;
#endif
#if defined(HAVE_ENUM_REMOVEZEROLAYER)
        case RemoveZeroLayer:
            new_images = rm_clone_imagelist(images);
            RemoveZeroDelayLayers(&new_images, &exception);
            break;
#endif
#if defined(HAVE_ENUM_COMPOSITELAYER)
        case CompositeLayer:
            rb_raise(rb_eNotImpError, "Magick::CompositeLayer is not yet supported.");
            break;
#endif
#if defined(HAVE_ENUM_OPTIMIZEIMAGELAYER)
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
            (void) MapImages(new_images, NULL, 0);
            break;
#else
        case OptimizeLayer:
            new_images = OptimizeImageLayers(images, &exception);
            break;
#endif
        case OptimizePlusLayer:
            new_images = OptimizePlusImageLayers(images, &exception);
            break;
        case CompareAnyLayer:
        case CompareClearLayer:
        case CompareOverlayLayer:
            new_images = CompareImageLayers(images, mthd, &exception);
            break;
        default:
            rb_raise(rb_eArgError, "undefined layer method");
            break;
    }

    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);

    return rm_imagelist_from_images(new_images);
}


/*
    External:   rm_imagelist_new
    Purpose:    create a new ImageList object with no images
    Notes:      this simply calls ImageList.new() in RMagick.rb
*/
VALUE
rm_imagelist_new()
{
    return rb_funcall(Class_ImageList, rm_ID_new, 0);
}



/*
     Extern:   rm_imagelist_from_images
     Purpose:  Construct a new imagelist object from a list of images
     Notes:    Sets @scene to 0.
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

    new_imagelist = rm_imagelist_new();

    while (images)
    {
        image = RemoveFirstImageFromList(&images);
        rm_imagelist_push(new_imagelist, rm_image_new(image));
    }

    (void) rb_iv_set(new_imagelist, "@scene", INT2FIX(0));
    return new_imagelist;
}


/*
    Extern:     rm_images_from_imagelist
    Purpose:    Convert an array of Image *s to an ImageMagick scene
                sequence (i.e. a doubly-linked list of Images)
    Returns:    a pointer to the head of the scene sequence list
*/
Image *
rm_images_from_imagelist(VALUE imagelist)
{
    long x, len;
    Image *head = NULL;
    volatile VALUE images, t;

    len = rm_imagelist_length(imagelist);
    if (len == 0)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

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


/*
 *   Extern:   rm_imagelist_scene_eq(imagelist, scene)
 *   Purpose:  @scene attribute writer
*/
VALUE
rm_imagelist_scene_eq(VALUE imagelist, VALUE scene)
{
    rb_check_frozen(imagelist);
    (void) rb_iv_set(imagelist, "@scene", scene);
    return scene;
}

/*
    External:   rm_imagelist_length
    Purpose:    return the # of images in an imagelist
*/
long
rm_imagelist_length(VALUE imagelist)
{
    volatile VALUE images = rb_iv_get(imagelist, "@images");
    return RARRAY(images)->len;
}

/*
    External:   rm_imagelist_push
    Purpose:    push an image onto the end of the imagelist
*/
void
rm_imagelist_push(VALUE imagelist, VALUE image)
{
    rb_check_frozen(imagelist);
    (void) rb_funcall(imagelist, rm_ID_push, 1, image);
}


/*
 *  Extern:     rm_clone_imagelist
 *  Purpose:    clone a list of images, handle errors
 */
Image *rm_clone_imagelist(Image *images)
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


/*
    Method:     ImageList#quantize(<number_colors<, colorspace<, dither<, tree_depth<, measure_error>>>>>)
                     defaults: 256, Magick::RGBColorspace, true, 0, false
    Purpose:    call QuantizeImages
    Returns:    a new ImageList with quantized images. 'scene' is set to the same
                value as self.scene
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
            quantize_info.dither = (MagickBooleanType) RTEST(argv[2]);
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

    if (rm_imagelist_length(self) == 0L)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

    // Convert image array to image sequence, clone image sequence.
    GetExceptionInfo(&exception);
    images = rm_images_from_imagelist(self);
    new_images = CloneImageList(images, &exception);
    rm_split(images);
    rm_check_exception(&exception, new_images, DestroyOnError);
    (void) DestroyExceptionInfo(&exception);

    rm_ensure_result(new_images);


    (void) QuantizeImages(&quantize_info, new_images);
    rm_check_exception(&exception, new_images, DestroyOnError);

    // Create new ImageList object, convert mapped image sequence to images,
    // append to images array.
    new_imagelist = rm_imagelist_new();
    while ((new_image = ShiftImageList(&new_images)))
    {
        rm_imagelist_push(new_imagelist, rm_image_new(new_image));
    }

    // Set @scene in new ImageList object to same value as in self.
    scene = rb_iv_get(self, "@scene");
    (void) rb_iv_set(new_imagelist, "@scene", scene);

    return new_imagelist;
}

/*
    Method:     ImageList#to_blob
    Purpose:    returns the imagelist as a blob (a String)
    Notes:      runs an info parm block if present - the user can
                specify the image format and depth
*/
VALUE
ImageList_to_blob(VALUE self)
{
    Image *images;
    Info *info;
    volatile VALUE info_obj;
    volatile VALUE blob_str;
    void *blob = NULL;
    size_t length = 0;
    ExceptionInfo exception;

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    // Convert the images array to an images sequence.
    images = rm_images_from_imagelist(self);

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

    // Unconditionally request multi-images support. The worst that
    // can happen is that there's only one image or the format
    // doesn't support multi-image files.
    info->adjoin = MagickTrue;
    blob = ImagesToBlob(info, images, &length, &exception);
    if (exception.severity != UndefinedException)
    {
        magick_free((void*)blob);
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


/*
 *  Static:     file_arg_rescue
 *  Purpose:    called when `arg_to_str' raised an exception
*/
static VALUE file_arg_rescue(VALUE arg)
{
    rb_raise(rb_eTypeError, "argument must be path name or open file (%s given)",
             rb_class2name(CLASS_OF(arg)));
}


/*
  Method:   ImageList#write(file)
  Purpose:  Write all the images to the specified file. If the file format
            supports multi-image files, and the @images array contains more
            than one image, then the images will be written as a single
            multi-image file. Otherwise each image will be written to a
            separate file. Returns self.
*/
VALUE
ImageList_write(VALUE self, VALUE file)
{
    Image *images, *img;
    Info *info;
    const MagickInfo *m;
    volatile VALUE info_obj;
    char *filename;
    long filenameL;
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
        // Convert arg to string. Catch exceptions.
        file = rb_rescue(rb_String, file, file_arg_rescue, file);

        // Copy the filename to the Info and to the Image.
        filename = rb_str2cstr(file, &filenameL);
        filenameL = min(filenameL, MaxTextExtent-1);
        memcpy(info->filename, filename, (size_t)filenameL);
        info->filename[filenameL] = '\0';
        SetImageInfoFile(info, NULL);
    }

    // Convert the images array to an images sequence.
    images = rm_images_from_imagelist(self);

    // Copy the filename into each images. Set a scene number to be used if
    // writing multiple files. (Ref: ImageMagick's utilities/convert.c
    for (scene = 0, img = images; img; img = GetNextImageInList(img))
    {
        img->scene = scene++;
        strcpy(img->filename, info->filename);
    }

    GetExceptionInfo(&exception);
    (void) SetImageInfo(info, MagickTrue, &exception);
    rm_check_exception(&exception, images, RetainOnError);
    (void) DestroyExceptionInfo(&exception);

    // Find out if the format supports multi-images files.
    GetExceptionInfo(&exception);
    m = GetMagickInfo(info->magick, &exception);
    rm_check_exception(&exception, images, RetainOnError);
    (void) DestroyExceptionInfo(&exception);

    // Tell WriteImage if we want a multi-images file.
    if (rm_imagelist_length(self) > 1L && m->adjoin)
    {
        info->adjoin = MagickTrue;
    }

    for (img = images; img; img = GetNextImageInList(img))
    {
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
