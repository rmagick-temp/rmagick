/* $Id: rmilist.c,v 1.17 2004/12/02 23:19:01 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2004 by Timothy P. Hunter
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

    // Convert the images array to an images sequence.
    images = rm_toseq(self);

    if (argc > 1)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 or 1)", argc);
    }
    if (argc == 1)
    {
        Image *img;
        unsigned int delay;

        delay = NUM2UINT(argv[0]);
        for (img = images; img; img = GET_NEXT_IMAGE(img))
        {
            img->delay = delay;
        }
    }


    // Create a new Info object to use with this call
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    (void) AnimateImages(info, images);
    rm_handle_all_errors(images);
    rm_unseq(images);

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
    Image *images, *result;
    unsigned int stack;
    ExceptionInfo exception;

    // Convert the image array to an image sequence.
    images = rm_toseq(self);

    // If stack == true, stack rectangular images top-to-bottom,
    // otherwise left-to-right.
    stack = RTEST(stack_arg);

    GetExceptionInfo(&exception);
    result = AppendImages(images, stack, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    return rm_image_new(result);
}

/*
    Method:     ImageList#average
    Purpose:    Average all images together by calling AverageImages
    Returns:    an Frame object for the averaged image
*/
VALUE
ImageList_average(VALUE self)
{
    Image *images, *result;
    ExceptionInfo exception;

    // Convert the images array to an images sequence.
    images = rm_toseq(self);

    GetExceptionInfo(&exception);
    result = AverageImages(images, &exception);
    rm_handle_all_errors(images);
    rm_unseq(images);

    return rm_image_new(result);
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
    Image *images, *results;
    ExceptionInfo exception;

    // Convert the image array to an image sequence.
    images = rm_toseq(self);

    GetExceptionInfo(&exception);
    results = CoalesceImages(images, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    return rm_imagelist_from_images(results);
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

    images = rm_toseq(self);
    GetExceptionInfo(&exception);
    new_images = DeconstructImages(images, &exception);
    HANDLE_ERROR
    rm_unseq(images);

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
    unsigned int ok;

    // Convert the images array to an images sequence.
    images = rm_toseq(self);

    // Create a new Info object to use with this call
    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    ok = DisplayImages(info, images);
    if (!ok)
    {
        rm_handle_all_errors(images);
    }
    rm_unseq(images);

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

    images = rm_toseq(self);
    GetExceptionInfo(&exception);
    new_image = FlattenImages(images, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    return rm_image_new(new_image);
}


/*
    Method:     ImageList#map
    Purpose:    Call MapImages
    Returns:    a new ImageList with mapped images. @scene is set to self.scene
*/
VALUE
ImageList_map(VALUE self, VALUE map_image, VALUE dither_arg)
{
    Image *images, *clone_images = NULL;
    Image *map;
    unsigned int dither;
    volatile VALUE image, scene, new_imagelist;
    ExceptionInfo exception;

    image = ImageList_cur_image(map_image);
    Data_Get_Struct(image, Image, map);

    if (rm_imagelist_length(self) == 0)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

    // Convert image array to image sequence, clone image sequence.
    images = rm_toseq(self);
    GetExceptionInfo(&exception);
    clone_images = CloneImageList(images, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    // Call ImageMagick
    dither = !(dither_arg == Qfalse || dither_arg == Qnil);
    (void) MapImages(clone_images, map, dither);
    HANDLE_IMG_ERROR(clone_images)

    // Set @scene in new ImageList object to same value as in self.
    new_imagelist = rm_imagelist_from_images(clone_images);
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
    Image *montage_seq, *image_list;
    ExceptionInfo exception;

    // Create a new instance of the Magick::Montage class
    montage_obj = rm_montage_new();
    if (rb_block_given_p())
    {
        // Run the block in the instance's context, allowing the app to modify the
        // object's attributes.
        rb_obj_instance_eval(0, NULL, montage_obj);
    }

    Data_Get_Struct(montage_obj, Montage, montage);

    image_list = rm_toseq(self);

    // If app specified a non-default composition operator, use it for all images.
    if (montage->compose != UndefinedCompositeOp)
    {
        Image *i;
        for (i = image_list; i; i = GET_NEXT_IMAGE(i))
        {
            i->compose = montage->compose;
        }
    }

    GetExceptionInfo(&exception);

    // MontageImage can return more than one image.
    montage_seq = MontageImages(image_list, montage->info, &exception);
    HANDLE_ERROR
    rm_unseq(image_list);

    return rm_imagelist_from_images(montage_seq);
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
    unsigned long number_images;

    if (rm_imagelist_length(self) < 1)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }
    number_images = NUM2ULONG(nimages);
    if (number_images <= 0)
    {
        rb_raise(rb_eArgError, "number of intervening images must be > 0");
    }

    images = rm_toseq(self);
    GetExceptionInfo(&exception);
    new_images = MorphImages(images, number_images, &exception);
    HANDLE_ERROR

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

    images = rm_toseq(self);
    GetExceptionInfo(&exception);
    new_image = MosaicImages(images, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    return rm_image_new(new_image);
}

/*
    External:   rm_imagelist_new
    Purpose:    create a new ImageList object with no images
    Notes:      this simply calls ImageList.new() in RMagick.rb
*/
VALUE
rm_imagelist_new()
{
    return rb_funcall(Class_ImageList, ID_new, 0);
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
#if defined(HAVE_REMOVEFIRSTIMAGEFROMLIST)
     Image *image;

     new_imagelist = rm_imagelist_new();

     while (images)
     {
          image = RemoveFirstImageFromList(&images);
          rm_imagelist_push(new_imagelist, rm_image_new(image));
     }
#else
     Image *image, *next;

     new_imagelist = rm_imagelist_new();

     for (image = images; image; image = next)
     {
          next = GET_NEXT_IMAGE(image);
          image->previous = image->next = NULL;
          rm_imagelist_push(new_imagelist, rm_image_new(image));
     }
#endif

     rb_iv_set(new_imagelist, "@scene", INT2FIX(0));
     return new_imagelist;
}


/*
 *   Extern:   rm_imagelist_scene_eq(imagelist, scene)
 *   Purpose:  @scene attribute writer
*/
VALUE
rm_imagelist_scene_eq(VALUE imagelist, VALUE scene)
{
     rb_iv_set(imagelist, "@scene", scene);
     return scene;
}

/*
    External:   rm_imagelist_length
    Purpose:    return the # of images in an imagelist
*/
int
rm_imagelist_length(VALUE imagelist)
{
    volatile VALUE len;

    len = rb_funcall(imagelist, ID_length, 0);
    return FIX2INT(len);
}

/*
    External:   rm_imagelist_push
    Purpose:    push an image onto the end of the imagelist
*/
void
rm_imagelist_push(VALUE imagelist, VALUE image)
{
    rm_check_frozen(imagelist);
    (void) rb_funcall(imagelist, ID_push, 1, image);
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
            quantize_info.measure_error = RTEST(argv[4]);
        case 4:
            quantize_info.tree_depth = NUM2INT(argv[3]);
        case 3:
            quantize_info.dither = RTEST(argv[2]);
        case 2:
            VALUE_TO_ENUM(argv[1], quantize_info.colorspace, ColorspaceType);
        case 1:
            quantize_info.number_colors = NUM2INT(argv[0]);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 5)", argc);
            break;
    }

    if (rm_imagelist_length(self) == 0)
    {
        rb_raise(rb_eArgError, "no images in this image list");
    }

    // Convert image array to image sequence, clone image sequence.
    GetExceptionInfo(&exception);
    images = rm_toseq(self);
    new_images = CloneImageList(images, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    QuantizeImages(&quantize_info, new_images);

    // Create new ImageList object, convert mapped image sequence to images,
    // append to images array.
    new_imagelist = rm_imagelist_new();
    while ((new_image = ShiftImageList(&new_images)))
    {
        rm_imagelist_push(new_imagelist, rm_image_new(new_image));
    }

    // Set @scene in new ImageList object to same value as in self.
    scene = rb_iv_get(self, "@scene");
    rb_iv_set(new_imagelist, "@scene", scene);

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
    void *blob = NULL;
    size_t length = 0;
    ExceptionInfo exception;

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);

    // Convert the images array to an images sequence.
    images = rm_toseq(self);

    GetExceptionInfo(&exception);
    (void) SetImageInfo(info, True, &exception);
    HANDLE_ERROR
    if (*info->magick != '\0')
    {
        Image *img;
        for (img = images; img; img = GET_NEXT_IMAGE(img))
        {
            strncpy(img->magick, info->magick, sizeof(info->magick)-1);
        }
    }

    // Unconditionally request multi-images support. The worst that
    // can happen is that there's only one image or the format
    // doesn't support multi-image files.
    info->adjoin = True;
    GetExceptionInfo(&exception);
    blob = ImageToBlob(info, images, &length, &exception);
    HANDLE_ERROR
    rm_unseq(images);

    return (blob && length) ? rb_str_new(blob, length) : Qnil;
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
    int scene;
    ExceptionInfo exception;

    info_obj = rm_info_new();
    Data_Get_Struct(info_obj, Info, info);


    // Convert the images array to an images sequence.
    images = rm_toseq(self);

    if (TYPE(file) == T_FILE)
    {
        OpenFile *fptr;

        // Ensure file is open - raise error if not
        GetOpenFile(file, fptr);
        info->file = GetReadFile(fptr);
    }
    else
    {
        // Convert arg to string. Catch exceptions.
        file = rb_rescue(rm_obj_to_s, file, file_arg_rescue, file);

        // Copy the filename to the Info and to the Image.
        filename = STRING_PTR_LEN(file, filenameL);
        filenameL = min(filenameL, MaxTextExtent-1);
        memcpy(info->filename, filename, (size_t)filenameL);
        info->filename[filenameL] = '\0';
        info->file = NULL;
    }

    // Copy the filename into each images. Set a scene number to be used if
    // writing multiple files. (Ref: ImageMagick's utilities/convert.c
    for (scene = 0, img = images; img; img = GET_NEXT_IMAGE(img))
    {
        img->scene = scene++;
        strcpy(img->filename, info->filename);
    }

    GetExceptionInfo(&exception);
    (void) SetImageInfo(info, True, &exception);

    // Find out if the format supports multi-images files.
    m = GetMagickInfo(info->magick, &exception);
    HANDLE_ERROR

    // Tell WriteImage if we want a multi-images file.
    if (rm_imagelist_length(self) > 1 && m->adjoin)
    {
        info->adjoin = True;
    }

    for (img = images; img; img = GET_NEXT_IMAGE(img))
    {
        (void) WriteImage(info, img);
        rm_handle_all_errors(images);
        if (info->adjoin)
        {
            break;
        }
    }

    rm_unseq(images);
    return self;
}
