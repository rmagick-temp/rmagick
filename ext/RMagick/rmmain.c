/* $Id: rmmain.c,v 1.38 2003/12/25 21:15:37 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2003 by Timothy P. Hunter
| Name:     rmmain.c
| Author:   Tim Hunter
| Purpose:  Contains all module, class, method declarations.
|           Defines all constants
|           Contains Magick module methods.
\============================================================================*/

#define MAIN                        // Define external variables
#include "rmagick.h"

/*----------------------------------------------------------------------------\
| External declarations
\----------------------------------------------------------------------------*/
void Init_RMagick(void);

static void version_constants(void);


#define MAGICK_MONITOR_CVAR "@@__rmagick_monitor__"
static VALUE Magick_Monitor;

/*
  Method:   Magick::colors [ { |colorinfo| } ]
  Purpose:  If called with the optional block, iterates over the colors,
            otherwise returns an array of Magick::Color objects
*/
VALUE
Magick_colors(VALUE class)
{
#if defined(HAVE_GETCOLORINFOARRAY)
    ColorInfo **color_ary;
    ExceptionInfo exception;
    volatile VALUE ary, el;
    int x;

    GetExceptionInfo(&exception);

    color_ary = GetColorInfoArray(&exception);
    HANDLE_ERROR

    ary = rb_ary_new();

    x = 0;
    while (color_ary[x])
    {
        rb_ary_push(ary, Color_from_ColorInfo(color_ary[x]));
        x += 1;
    }

    magick_free(color_ary);

#else
    const ColorInfo *color_list;
    ColorInfo *color;
    ExceptionInfo exception;
    volatile VALUE ary, el;

    GetExceptionInfo(&exception);

    color_list = GetColorInfo("*", &exception);
    HANDLE_ERROR

    // IM may change the order of the colors list in mid-iteration,
    // so the only way we can guarantee a single pass thru the list
    // is to copy the elements into an array without returning to
    // IM. So, we always create a Ruby array and either return it
    // or iterate over it.
    ary = rb_ary_new();
    for (color = (ColorInfo *)color_list; color; color = color->next)
    {
        rb_ary_push(ary, Color_from_ColorInfo(color));
    }
#endif

    // If block, iterate over colors
    if (rb_block_given_p())
    {
        while ((el = rb_ary_shift(ary)) != Qnil)
        {
            rb_yield(el);
        }
        return class;
    }
    else
    {
        return ary;
    }
}

/*
  Method:   Magick::fonts [ { |fontinfo| } ]
  Purpose:  If called with the optional block, iterates over the fonts,
            otherwise returns an array of Magick::Font objects
*/
VALUE
Magick_fonts(VALUE class)
{
    const TypeInfo *type_list;
    TypeInfo *type, *next;
    ExceptionInfo exception;
    volatile VALUE ary;

    GetExceptionInfo(&exception);

    type_list = GetTypeInfo("*", &exception);
    HANDLE_ERROR

    // If block, iterate over fonts
    if (rb_block_given_p())
    {
        for (type = (TypeInfo *)type_list; type; type = next)
        {
            next = type->next;  // Protect against recursive calls to GetTypeInfo
            if (! type->stealth)
            {
                rb_yield(Font_from_TypeInfo(type));
            }
        }

        return class;
    }
    else
    {
        ary = rb_ary_new();
        for (type = (TypeInfo *)type_list; type; type = type->next)
        {
            rb_ary_push(ary, Font_from_TypeInfo(type));
        }
        return ary;
    }
}


/*
  Method:   Magick.init_formats
  Purpose:  Build the @@formats hash

            The hash keys are image formats. The hash values
            specify the format "mode string", i.e. a description of what
            ImageMagick can do with that format. The mode string is in the
            form "BRWA", where
                "B" is "*" if the format has native blob support, or " " otherwise.
                "R" is "r" if ImageMagick can read that format, or "-" otherwise.
                "W" is "w" if ImageMagick can write that format, or "-" otherwise.
                "A" is "+" if the format supports multi-image files, or "-" otherwise.
  Notes:    Only called once.
*/
VALUE
Magick_init_formats(VALUE class)
{
    MagickInfo *m;
    volatile VALUE formats;
    ExceptionInfo exception;
    char mode[5] = {0};

    class = class;      // defeat "never referenced" message from icc

    formats = rb_hash_new();

    GetExceptionInfo(&exception);
#if defined(HAVE_GETMAGICKINFOARRAY)
    m = (MagickInfo *)GetMagickInfoArray(&exception);
#else
    m = (MagickInfo *)GetMagickInfo("*", &exception);
#endif
    HANDLE_ERROR

    for ( ; m != NULL; m = m->next)
    {
        mode[0] = m->blob_support ? '*': ' ';
        mode[1] = m->decoder ? 'r' : '-';
        mode[2] = m->encoder ? 'w' : '-';
        mode[3] = m->encoder && m->adjoin ? '+' : '-';
        rb_hash_aset(formats, rb_str_new2(m->name), rb_str_new2(mode));
    }

#if defined(HAVE_GETMAGICKINFOARRAY)
    magick_free(m);
#endif

    return formats;
}

/*
    This is the exit known to ImageMagick. Retrieve the monitor
    proc and call it, passing the 3 exit arguments.
*/
static unsigned int
monitor_handler(
    const char *text,
    const magick_int64_t quantum,
    const magick_uint64_t span,
    ExceptionInfo *exception)
{
    volatile VALUE monitor;
    volatile VALUE args[3];

    exception = exception;  // defeat "never referenced" message from icc

    if (rb_cvar_defined(Module_Magick, Magick_Monitor))
    {
        args[0] = rb_str_new2(text);
        // Convert these possibly-64-bit types to 32-bit types that
        // Ruby can handle.
        args[1] = INT2NUM((long) quantum);
        args[2] = UINT2NUM((unsigned long) span);

        monitor = rb_cvar_get(Module_Magick, Magick_Monitor);
        (void) rb_funcall2((VALUE)monitor, call_ID, 3, (VALUE *)args);
    }

    return True;
}

/*
    Method:     Magick.set_monitor(&monitor)
    Purpose:    Establish MagickMonitor exit
    Notes:      use nil argument to turn off monitoring
*/
static VALUE
Magick_set_monitor(VALUE class, VALUE monitor)
{

    // 1st time: establish ID, define @@__MAGICK_MONITOR__
    // class variable, stow monitor VALUE in it.
    if (!Magick_Monitor)
    {
        Magick_Monitor = rb_intern(MAGICK_MONITOR_CVAR);
        rb_define_class_variable(Module_Magick, MAGICK_MONITOR_CVAR, monitor);
        call_ID = rb_intern("call");
    }

    // If nil, turn off monitoring.
    if (monitor == Qnil)
    {
        (void) SetMonitorHandler(NULL);
    }
    else
    // Otherwise, store monitor in @@__MAGICK_MONITOR__
    {
        // 1.8.0 deletes rb_cvar_declare and adds another
        // parm to rb_cvar_set - if rb_cvar_declare is
        // available, use the 3-parm version of rb_cvar_set.
        RUBY18(rb_cvar_set(Module_Magick, Magick_Monitor, monitor, 0);)
        RUBY16(rb_cvar_set(Module_Magick, Magick_Monitor, monitor);)
        (void) SetMonitorHandler(&monitor_handler);
    }

    return class;
}

/*
    Method      Magick.set_cache_threshold(megabytes)
    Purpose:    sets the amount of free memory allocated for the
                pixel cache.  Once this threshold is exceeded, all
                subsequent pixels cache operations are to/from disk.
    Notes:      singleton method
                Pre-5.5.1 this method called the SetCacheThreshold
                function, which is deprecated in 5.5.1.
*/
static VALUE
Magick_set_cache_threshold(VALUE class, VALUE threshold)
{
    unsigned long thrshld = NUM2ULONG(threshold);
    SetMagickResourceLimit(MemoryResource,thrshld);
    SetMagickResourceLimit(MapResource,2*thrshld);
    return class;
}

/*
    Method:     Magick.set_log_event_mask(event,...) -> Magick
    Notes:      "event" is one of "all", "annotate", "blob", "cache",
                "coder", "configure", "deprecate", "locale", "none",
                "render", "transform", "user", "x11". Multiple events
                can be specified. Event names may be capitalized.
*/
static VALUE
Magick_set_log_event_mask(int argc, VALUE *argv, VALUE class)
{
    int x;

    if (argc == 0)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (at least 1 required)");
    }
    for (x = 0; x < argc; x++)
    {
        (void) SetLogEventMask(STRING_PTR(argv[x]));
    }
    return class;
}

/*
    Method:     Magick.set_log_format(format) -> Magick
    Notes:      Format is a string containing one or more of:
                %t  - current time
                %r  - elapsed time
                %u  - user time
                %p  - pid
                %m  - module (source file name)
                %f  - function name
                %l  - line number
                %d  - event domain (one of the events listed above)
                %e  - event name
                Plus other characters, including \n, etc.
*/
static VALUE
Magick_set_log_format(VALUE class, VALUE format)
{
#ifdef HAVE_SETLOGFORMAT
    SetLogFormat(STRING_PTR(format));
    return class;
#else
    not_implemented("set_log_format");
    return (VALUE) 0;
#endif
}



/*
  External:     Init_RMagick
  Purpose:      define the classes and constants
  Arguments:    void
  Returns:      void
*/

void
Init_RMagick(void)
{

    InitializeMagick("RMagick");

    Module_Magick = rb_define_module("Magick");

    /*-----------------------------------------------------------------------*/
    /* Module Magick methods                                                 */
    /*-----------------------------------------------------------------------*/

    rb_define_module_function(Module_Magick, "colors", Magick_colors, 0);
    rb_define_module_function(Module_Magick, "fonts", Magick_fonts, 0);
    rb_define_module_function(Module_Magick, "init_formats", Magick_init_formats, 0);
    rb_define_module_function(Module_Magick, "set_monitor", Magick_set_monitor, 1);
    rb_define_module_function(Module_Magick, "set_cache_threshold", Magick_set_cache_threshold, 1);
    rb_define_module_function(Module_Magick, "set_log_event_mask", Magick_set_log_event_mask, -1);
    rb_define_module_function(Module_Magick, "set_log_format", Magick_set_log_format, 1);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::Image methods                                           */
    /*-----------------------------------------------------------------------*/

    Class_Image = rb_define_class_under(Module_Magick, "Image", rb_cObject);

    // Define an alias for Object#display before we override it
    rb_define_alias(Class_Image, "__display__", "display");

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_Image, Image_alloc);
    rb_define_method(Class_Image, "initialize", Image_initialize, -1);
#else
    rb_define_singleton_method(Class_Image, "new", Image_new, -1);
    rb_define_method(Class_Image, "initialize", Image_initialize, 4);
#endif

    rb_define_singleton_method(Class_Image, "constitute", Image_constitute, 4);
    rb_define_singleton_method(Class_Image, "_load", Image__load, 1);
    rb_define_singleton_method(Class_Image, "capture", Image_capture, -1);
    rb_define_singleton_method(Class_Image, "ping", Image_ping, 1);
    rb_define_singleton_method(Class_Image, "read", Image_read, 1);
    rb_define_singleton_method(Class_Image, "from_blob", Image_from_blob, 1);

    DCL_ATTR_ACCESSOR(Image, background_color)
    DCL_ATTR_READER(Image, base_columns)
    DCL_ATTR_READER(Image, base_filename)
    DCL_ATTR_READER(Image, base_rows)
    DCL_ATTR_ACCESSOR(Image, blur)
    DCL_ATTR_ACCESSOR(Image, border_color)
    DCL_ATTR_READER(Image, bounding_box)
    DCL_ATTR_ACCESSOR(Image, chromaticity)
    DCL_ATTR_WRITER(Image, clip_mask)
    DCL_ATTR_ACCESSOR(Image, color_profile)
    DCL_ATTR_READER(Image, colors)
    DCL_ATTR_ACCESSOR(Image, colorspace)
    DCL_ATTR_READER(Image, columns)
    DCL_ATTR_ACCESSOR(Image, compose)
    DCL_ATTR_ACCESSOR(Image, compression)
    DCL_ATTR_ACCESSOR(Image, delay)
    DCL_ATTR_ACCESSOR(Image, density)
    DCL_ATTR_READER(Image, depth)
    DCL_ATTR_READER(Image, directory)
    DCL_ATTR_ACCESSOR(Image, dispose)
    DCL_ATTR_ACCESSOR(Image, extract_info)
    DCL_ATTR_READER(Image, filename)
    DCL_ATTR_READER(Image, filesize)
    DCL_ATTR_ACCESSOR(Image, filter)
    DCL_ATTR_ACCESSOR(Image, format)
    DCL_ATTR_ACCESSOR(Image, fuzz)
    DCL_ATTR_ACCESSOR(Image, gamma)
    DCL_ATTR_ACCESSOR(Image, geometry)
    DCL_ATTR_ACCESSOR(Image, image_type)
    DCL_ATTR_ACCESSOR(Image, interlace)
    DCL_ATTR_ACCESSOR(Image, iptc_profile)
    DCL_ATTR_ACCESSOR(Image, iterations)        // do not document! Only used by Image#iterations=
    DCL_ATTR_ACCESSOR(Image, matte)
    DCL_ATTR_ACCESSOR(Image, matte_color)
    DCL_ATTR_READER(Image, mean_error_per_pixel)
    DCL_ATTR_READER(Image, mime_type)
    DCL_ATTR_ACCESSOR(Image, montage)
    DCL_ATTR_READER(Image, normalized_mean_error)
    DCL_ATTR_READER(Image, normalized_maximum_error)
    DCL_ATTR_READER(Image, number_colors)
    DCL_ATTR_ACCESSOR(Image, offset)
    DCL_ATTR_WRITER(Image, opacity)
    DCL_ATTR_ACCESSOR(Image, page)
    DCL_ATTR_READER(Image, quantum_depth)
    DCL_ATTR_ACCESSOR(Image, rendering_intent)
    DCL_ATTR_READER(Image, rows)
    DCL_ATTR_READER(Image, scene)
    DCL_ATTR_ACCESSOR(Image, start_loop)
    DCL_ATTR_ACCESSOR(Image, class_type)
    DCL_ATTR_ACCESSOR(Image, tile_info)
    DCL_ATTR_READER(Image, total_colors)
    DCL_ATTR_ACCESSOR(Image, units)
    DCL_ATTR_ACCESSOR(Image, x_resolution)
    DCL_ATTR_ACCESSOR(Image, y_resolution)

    rb_define_method(Class_Image, "adaptive_threshold", Image_adaptive_threshold, -1);
    rb_define_method(Class_Image, "add_noise", Image_add_noise, 1);
    rb_define_method(Class_Image, "affine_transform", Image_affine_transform, 1);
    rb_define_method(Class_Image, "[]", Image_aref, 1);
    rb_define_method(Class_Image, "[]=", Image_aset, 2);
    rb_define_method(Class_Image, "properties", Image_properties, 0);
    rb_define_method(Class_Image, "black_threshold", Image_black_threshold, -1);
    rb_define_method(Class_Image, "blur_image", Image_blur_image, -1);
    rb_define_method(Class_Image, "border", Image_border, 3);
    rb_define_method(Class_Image, "border!", Image_border_bang, 3);
    rb_define_method(Class_Image, "change_geometry", Image_change_geometry, 1);
    rb_define_method(Class_Image, "change_geometry!", Image_change_geometry, 1);
    rb_define_method(Class_Image, "changed?", Image_changed_q, 0);
    rb_define_method(Class_Image, "channel", Image_channel, 1);
    rb_define_method(Class_Image, "channel_compare", Image_channel_compare, -1);
    rb_define_method(Class_Image, "channel_depth", Image_channel_depth, -1);
    rb_define_method(Class_Image, "channel_extrema", Image_channel_extrema, -1);
    rb_define_method(Class_Image, "channel_threshold", Image_channel_threshold, -1);
    rb_define_method(Class_Image, "charcoal", Image_charcoal, -1);
    rb_define_method(Class_Image, "chop", Image_chop, 4);
    rb_define_method(Class_Image, "color_flood_fill", Image_color_flood_fill, 5);
    rb_define_method(Class_Image, "color_histogram", Image_color_histogram, 0);
    rb_define_method(Class_Image, "colorize", Image_colorize, -1);
    rb_define_method(Class_Image, "colormap", Image_colormap, -1);
    rb_define_method(Class_Image, "composite", Image_composite, -1);
    rb_define_method(Class_Image, "composite_affine", Image_composite_affine, 2);
    rb_define_method(Class_Image, "compress_colormap!", Image_compress_colormap_bang, 0);
    rb_define_method(Class_Image, "contrast", Image_contrast, -1);
    rb_define_method(Class_Image, "convolve", Image_convolve, 2);
    rb_define_method(Class_Image, "copy", Image_copy, 0);
    rb_define_method(Class_Image, "crop", Image_crop, -1);
    rb_define_method(Class_Image, "crop!", Image_crop_bang, -1);
    rb_define_method(Class_Image, "cycle_colormap", Image_cycle_colormap, 1);
    rb_define_method(Class_Image, "despeckle", Image_despeckle, 0);
    rb_define_method(Class_Image, "difference", Image_difference, 1);
    rb_define_method(Class_Image, "dispatch", Image_dispatch, -1);
    rb_define_method(Class_Image, "display", Image_display, 0);
    rb_define_method(Class_Image, "_dump", Image__dump, 1);
    rb_define_method(Class_Image, "each_profile", Image_each_profile, 0);
    rb_define_method(Class_Image, "edge", Image_edge, -1);
    rb_define_method(Class_Image, "emboss", Image_emboss, -1);
    rb_define_method(Class_Image, "enhance", Image_enhance, 0);
    rb_define_method(Class_Image, "equalize", Image_equalize, 0);
    rb_define_method(Class_Image, "erase!", Image_erase_bang, 0);
    rb_define_method(Class_Image, "export_pixels", Image_export_pixels, 5);
    rb_define_method(Class_Image, "flip", Image_flip, 0);
    rb_define_method(Class_Image, "flip!", Image_flip_bang, 0);
    rb_define_method(Class_Image, "flop", Image_flop, 0);
    rb_define_method(Class_Image, "flop!", Image_flop_bang, 0);
    rb_define_method(Class_Image, "frame", Image_frame, -1);
    rb_define_method(Class_Image, "gamma_correct", Image_gamma_correct, -1);
    rb_define_method(Class_Image, "gaussian_blur", Image_gaussian_blur, -1);
    rb_define_method(Class_Image, "get_pixels", Image_get_pixels, 4);
    rb_define_method(Class_Image, "gray?", Image_gray_q, 0);
    rb_define_method(Class_Image, "grey?", Image_gray_q, 0);
    rb_define_method(Class_Image, "implode", Image_implode, -1);
    rb_define_method(Class_Image, "import_pixels", Image_import_pixels, 6);
    rb_define_method(Class_Image, "inspect", Image_inspect, 0);
    rb_define_method(Class_Image, "level", Image_level, -1);
    rb_define_method(Class_Image, "level_channel", Image_level_channel, -1);
    rb_define_method(Class_Image, "magnify", Image_magnify, 0);
    rb_define_method(Class_Image, "magnify!", Image_magnify_bang, 0);
    rb_define_method(Class_Image, "map", Image_map, -1);
    rb_define_method(Class_Image, "matte_flood_fill", Image_matte_flood_fill, 5);
    rb_define_method(Class_Image, "median_filter", Image_median_filter, -1);
    rb_define_method(Class_Image, "minify", Image_minify, 0);
    rb_define_method(Class_Image, "minify!", Image_minify_bang, 0);
    rb_define_method(Class_Image, "modulate", Image_modulate, -1);
    rb_define_method(Class_Image, "monochrome?", Image_monochrome_q, 0);
    rb_define_method(Class_Image, "motion_blur", Image_motion_blur, 3);
    rb_define_method(Class_Image, "negate", Image_negate, -1);
    rb_define_method(Class_Image, "normalize", Image_normalize, 0);
    rb_define_method(Class_Image, "oil_paint", Image_oil_paint, -1);
    rb_define_method(Class_Image, "opaque", Image_opaque, 2);
    rb_define_method(Class_Image, "opaque?", Image_opaque_q, 0);
    rb_define_method(Class_Image, "ordered_dither", Image_ordered_dither, 0);
    rb_define_method(Class_Image, "palette?", Image_palette_q, 0);
    rb_define_method(Class_Image, "pixel_color", Image_pixel_color, -1);
//  rb_define_method(Class_Image, "plasma", Image_plasma, 6);
    rb_define_method(Class_Image, "preview", Image_preview, 1);
    rb_define_method(Class_Image, "profile!", Image_profile_bang, 2);
    rb_define_method(Class_Image, "quantize", Image_quantize, -1);
    rb_define_method(Class_Image, "raise", Image_raise, -1);
    rb_define_method(Class_Image, "random_channel_threshold", Image_random_channel_threshold, 2);
    rb_define_method(Class_Image, "reduce_noise", Image_reduce_noise, 1);
    rb_define_method(Class_Image, "resize", Image_resize, -1);
    rb_define_method(Class_Image, "resize!", Image_resize_bang, -1);
    rb_define_method(Class_Image, "roll", Image_roll, 2);
    rb_define_method(Class_Image, "rotate", Image_rotate, 1);
    rb_define_method(Class_Image, "rotate!", Image_rotate_bang, 1);
    rb_define_method(Class_Image, "sample", Image_sample, -1);
    rb_define_method(Class_Image, "sample!", Image_sample_bang, -1);
    rb_define_method(Class_Image, "scale", Image_scale, -1);
    rb_define_method(Class_Image, "scale!", Image_scale_bang, -1);
    rb_define_method(Class_Image, "segment", Image_segment, -1);
    rb_define_method(Class_Image, "shade", Image_shade, -1);
    rb_define_method(Class_Image, "sharpen", Image_sharpen, -1);
    rb_define_method(Class_Image, "shave", Image_shave, 2);
    rb_define_method(Class_Image, "shave!", Image_shave_bang, 2);
    rb_define_method(Class_Image, "shear", Image_shear, 2);
    rb_define_method(Class_Image, "signature", Image_signature, 0);
    rb_define_method(Class_Image, "solarize", Image_solarize, -1);
    rb_define_method(Class_Image, "<=>", Image_spaceship, 1);
    rb_define_method(Class_Image, "spread", Image_spread, -1);
    rb_define_method(Class_Image, "stegano", Image_stegano, 2);
    rb_define_method(Class_Image, "stereo", Image_stereo, 1);
    rb_define_method(Class_Image, "strip!", Image_strip_bang, 0);
    rb_define_method(Class_Image, "store_pixels", Image_store_pixels, 5);
    rb_define_method(Class_Image, "swirl", Image_swirl, 1);
    rb_define_method(Class_Image, "texture_flood_fill", Image_texture_flood_fill, 5);
    rb_define_method(Class_Image, "threshold", Image_threshold, 1);
    rb_define_method(Class_Image, "thumbnail", Image_thumbnail, -1);
    rb_define_method(Class_Image, "thumbnail!", Image_thumbnail_bang, -1);
    rb_define_method(Class_Image, "tint", Image_tint, -1);
    rb_define_method(Class_Image, "to_color", Image_to_color, 1);
    rb_define_method(Class_Image, "to_blob", Image_to_blob, 0);
    rb_define_method(Class_Image, "transparent", Image_transparent, -1);
    rb_define_method(Class_Image, "unsharp_mask", Image_unsharp_mask, 4);
    rb_define_method(Class_Image, "wave", Image_wave, -1);
    rb_define_method(Class_Image, "white_threshold", Image_white_threshold, -1);
    rb_define_method(Class_Image, "write", Image_write, 1);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::ImageList methods (see also RMagick.rb)                 */
    /*-----------------------------------------------------------------------*/

    Class_ImageList = rb_define_class_under(Module_Magick, "ImageList", rb_cArray);

    // Define an alias for Object#display before we override it
    rb_define_alias(Class_ImageList, "__display__", "display");

    // Define an alias for Array's "map" method.
    rb_define_alias(Class_ImageList, "__map__", "map");

    rb_define_method(Class_ImageList, "animate", ImageList_animate, -1);
    rb_define_method(Class_ImageList, "append", ImageList_append, 1);
    rb_define_method(Class_ImageList, "average", ImageList_average, 0);
    rb_define_method(Class_ImageList, "coalesce", ImageList_coalesce, 0);
    rb_define_method(Class_ImageList, "deconstruct", ImageList_deconstruct, 0);
    rb_define_method(Class_ImageList, "display", ImageList_display, 0);
    rb_define_method(Class_ImageList, "flatten_images", ImageList_flatten_images, 0);
    rb_define_method(Class_ImageList, "map", ImageList_map, 2);
    rb_define_method(Class_ImageList, "montage", ImageList_montage, 0);
    rb_define_method(Class_ImageList, "morph", ImageList_morph, 1);
    rb_define_method(Class_ImageList, "mosaic", ImageList_mosaic, 0);
    rb_define_method(Class_ImageList, "quantize", ImageList_quantize, -1);
    rb_define_method(Class_ImageList, "to_blob", ImageList_to_blob, 0);
    rb_define_method(Class_ImageList, "write", ImageList_write, 1);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::Draw methods                                            */
    /*-----------------------------------------------------------------------*/

    Class_Draw = rb_define_class_under(Module_Magick, "Draw", rb_cObject);
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_Draw, Draw_alloc);
#else
    rb_define_singleton_method(Class_Draw, "new", Draw_new, 0);
#endif

    DCL_ATTR_WRITER(Draw, affine)
    DCL_ATTR_WRITER(Draw, align)
    DCL_ATTR_WRITER(Draw, decorate)
    DCL_ATTR_WRITER(Draw, density)
    DCL_ATTR_WRITER(Draw, encoding)
    DCL_ATTR_WRITER(Draw, fill)
    DCL_ATTR_WRITER(Draw, font)
    DCL_ATTR_WRITER(Draw, font_family)
    DCL_ATTR_WRITER(Draw, font_stretch)
    DCL_ATTR_WRITER(Draw, font_style)
    DCL_ATTR_WRITER(Draw, font_weight)
    DCL_ATTR_WRITER(Draw, gravity)
    DCL_ATTR_WRITER(Draw, pointsize)
    DCL_ATTR_WRITER(Draw, rotation)
    DCL_ATTR_WRITER(Draw, stroke)
    DCL_ATTR_WRITER(Draw, text_antialias)
    DCL_ATTR_WRITER(Draw, undercolor)

    rb_define_method(Class_Draw, "annotate", Draw_annotate, 6);
    rb_define_method(Class_Draw, "composite", Draw_composite, -1);
    rb_define_method(Class_Draw, "draw", Draw_draw, 1);
    rb_define_method(Class_Draw, "get_type_metrics", Draw_get_type_metrics, -1);
    rb_define_method(Class_Draw, "initialize", Draw_initialize, 0);
    rb_define_method(Class_Draw, "inspect", Draw_inspect, 0);
    rb_define_method(Class_Draw, "primitive", Draw_primitive, 1);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::ImageList::Montage methods                              */
    /*-----------------------------------------------------------------------*/

    Class_Montage = rb_define_class_under(Class_ImageList, "Montage", rb_cObject);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_Montage, Montage_alloc);
#else
    rb_define_singleton_method(Class_Montage, "new", Montage_new, 0);
#endif

    rb_define_method(Class_Montage, "initialize", Montage_initialize, 0);

    // These accessors supply optional arguments for Magick::ImageList::Montage.new
    DCL_ATTR_WRITER(Montage, background_color)
    DCL_ATTR_WRITER(Montage, border_color)
    DCL_ATTR_WRITER(Montage, border_width)
    DCL_ATTR_WRITER(Montage, compose)
    DCL_ATTR_WRITER(Montage, filename)
    DCL_ATTR_WRITER(Montage, fill)
    DCL_ATTR_WRITER(Montage, font)
    DCL_ATTR_WRITER(Montage, frame)
    DCL_ATTR_WRITER(Montage, geometry)
    DCL_ATTR_WRITER(Montage, gravity)
    DCL_ATTR_WRITER(Montage, matte_color)
    DCL_ATTR_WRITER(Montage, pointsize)
    DCL_ATTR_WRITER(Montage, shadow)
    DCL_ATTR_WRITER(Montage, stroke)
    DCL_ATTR_WRITER(Montage, texture)
    DCL_ATTR_WRITER(Montage, tile)
    DCL_ATTR_WRITER(Montage, title)

    /*-----------------------------------------------------------------------*/
    /* Class Magick::Image::Info                                             */
    /*-----------------------------------------------------------------------*/

    Class_Info = rb_define_class_under(Class_Image, "Info", rb_cObject);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_Info, Info_alloc);
#else
    rb_define_singleton_method(Class_Info, "new", Info_new, 0);
#endif

    rb_define_method(Class_Info, "initialize", Info_initialize, 0);

    DCL_ATTR_ACCESSOR(Info, antialias)
    DCL_ATTR_ACCESSOR(Info, background_color)
    DCL_ATTR_ACCESSOR(Info, border_color)
    DCL_ATTR_ACCESSOR(Info, colorspace)
    DCL_ATTR_ACCESSOR(Info, compression)
    DCL_ATTR_ACCESSOR(Info, density)
    DCL_ATTR_ACCESSOR(Info, depth)
    DCL_ATTR_ACCESSOR(Info, dither)
    DCL_ATTR_ACCESSOR(Info, extract)    // new in 5.5.6, replaces tile
    DCL_ATTR_ACCESSOR(Info, filename)
    DCL_ATTR_ACCESSOR(Info, font)
    DCL_ATTR_ACCESSOR(Info, format)
    DCL_ATTR_ACCESSOR(Info, fuzz)
    DCL_ATTR_ACCESSOR(Info, group)
    DCL_ATTR_ACCESSOR(Info, interlace)
    DCL_ATTR_ACCESSOR(Info, matte_color)
    DCL_ATTR_ACCESSOR(Info, monochrome)
    DCL_ATTR_ACCESSOR(Info, number_scenes)  // new in 5.5.6, replaces subrange
    DCL_ATTR_ACCESSOR(Info, page)
    DCL_ATTR_ACCESSOR(Info, quality)
    DCL_ATTR_ACCESSOR(Info, scene)      // new in 5.5.6, replaces subimage
    DCL_ATTR_ACCESSOR(Info, server_name)
    DCL_ATTR_ACCESSOR(Info, size)
    DCL_ATTR_ACCESSOR(Info, subimage)   // deprecated >=5.5.6, replaced by scene
    DCL_ATTR_ACCESSOR(Info, subrange)   // deprecated >=5.5.6, replaced by number_scenes
    DCL_ATTR_ACCESSOR(Info, tile)       // deprecated >=5.5.6, replaced by extract and scenes
    DCL_ATTR_ACCESSOR(Info, image_type)
    DCL_ATTR_ACCESSOR(Info, units)
    DCL_ATTR_ACCESSOR(Info, view)

    /*-----------------------------------------------------------------------*/
    /* Magick::******Fill classes and methods                                */
    /*-----------------------------------------------------------------------*/

    // class Magick::GradientFill
    Class_GradientFill = rb_define_class_under(Module_Magick, "GradientFill", rb_cObject);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_GradientFill, GradientFill_alloc);
#else
    rb_define_singleton_method(Class_GradientFill, "new", GradientFill_new, 6);
#endif

    rb_define_method(Class_GradientFill, "initialize", GradientFill_initialize, 6);
    rb_define_method(Class_GradientFill, "fill", GradientFill_fill, 1);

    // class Magick::TextureFill
    Class_TextureFill = rb_define_class_under(Module_Magick, "TextureFill", rb_cObject);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_TextureFill, TextureFill_alloc);
#else
    rb_define_singleton_method(Class_TextureFill, "new", TextureFill_new, 1);
#endif

    rb_define_method(Class_TextureFill, "initialize", TextureFill_initialize, 1);
    rb_define_method(Class_TextureFill, "fill", TextureFill_fill, 1);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::ImageMagickError < StandardError                        */
    /*-----------------------------------------------------------------------*/

    Class_ImageMagickError = rb_define_class_under(Module_Magick, "ImageMagickError", rb_eStandardError);
    rb_define_method(Class_ImageMagickError, "initialize", ImageMagickError_initialize, 2);
    RUBY16(rb_enable_super(Class_ImageMagickError, "initialize");)
    rb_define_attr(Class_ImageMagickError, MAGICK_LOC, True, False);


    // Miscellaneous constants
    rb_define_const(Module_Magick, "MaxRGB", INT2FIX(MaxRGB));
    rb_define_const(Module_Magick, "QuantumDepth", INT2FIX(QuantumDepth));

    version_constants();

    // Opacity constants
    DEF_CONST(OpaqueOpacity);
    DEF_CONST(TransparentOpacity);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::Enum                                                    */
    /*-----------------------------------------------------------------------*/

    // includes Comparable
    Class_Enum = rb_define_class_under(Module_Magick, "Enum", rb_cObject);
    rb_include_module(Class_Enum, rb_mComparable);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_Enum, Enum_alloc);
#else
    rb_define_singleton_method(Class_Enum, "new", Enum_new, 2);
#endif

    rb_define_method(Class_Enum, "initialize", Enum_initialize, 2);
    rb_define_method(Class_Enum, "to_s", Enum_to_s, 0);
    rb_define_method(Class_Enum, "to_i", Enum_to_i, 0);
    rb_define_method(Class_Enum, "<=>", Enum_spaceship, 1);
    rb_define_method(Class_Enum, "===", Enum_case_eq, 1);

    // AlignType constants
    DEF_ENUM(AlignType);
    ENUM_VAL(AlignType, UndefinedAlign); ENUM_VAL(AlignType, LeftAlign);
    ENUM_VAL(AlignType, CenterAlign);    ENUM_VAL(AlignType, RightAlign);

    // AnchorType constants (for Draw#text_anchor - these are not defined by ImageMagick)
    DEF_ENUM(AnchorType);
    rb_define_const(Module_Magick, "StartAnchor",
            rm_enum_new(Class_AnchorType, ID2SYM(rb_intern("StartAnchor")), INT2FIX(1)));
    rb_define_const(Module_Magick, "MiddleAnchor",
            rm_enum_new(Class_AnchorType, ID2SYM(rb_intern("MiddleAnchor")), INT2FIX(2)));
    rb_define_const(Module_Magick, "EndAnchor",
            rm_enum_new(Class_AnchorType, ID2SYM(rb_intern("EndAnchor")), INT2FIX(3)));

    // ChannelType constants
    DEF_ENUM(ChannelType);
    ENUM_VAL(ChannelType, UndefinedChannel);    ENUM_VAL(ChannelType, RedChannel);
    ENUM_VAL(ChannelType, CyanChannel);         ENUM_VAL(ChannelType, GreenChannel);
    ENUM_VAL(ChannelType, MagentaChannel);      ENUM_VAL(ChannelType, BlueChannel);
    ENUM_VAL(ChannelType, YellowChannel);       ENUM_VAL(ChannelType, OpacityChannel);
    ENUM_VAL(ChannelType, BlackChannel);        ENUM_VAL(ChannelType, MatteChannel);
#if defined(HAVE_INDEXCHANNEL)
    ENUM_VAL(ChannelType, IndexChannel);    // 5.5.8
#endif

    // ClassType constants
    DEF_ENUM(ClassType);
    ENUM_VAL(ClassType, UndefinedClass);      ENUM_VAL(ClassType, PseudoClass);
    ENUM_VAL(ClassType, DirectClass);

    // ColorspaceType constants
    DEF_ENUM(ColorspaceType);
    ENUM_VAL(ColorspaceType, UndefinedColorspace);
    ENUM_VAL(ColorspaceType, RGBColorspace);
    ENUM_VAL(ColorspaceType, GRAYColorspace);
    ENUM_VAL(ColorspaceType, TransparentColorspace);
    ENUM_VAL(ColorspaceType, OHTAColorspace);
    ENUM_VAL(ColorspaceType, XYZColorspace);
    ENUM_VAL(ColorspaceType, YCbCrColorspace);
    ENUM_VAL(ColorspaceType, YCCColorspace);
    ENUM_VAL(ColorspaceType, YIQColorspace);
    ENUM_VAL(ColorspaceType, YPbPrColorspace);
    ENUM_VAL(ColorspaceType, YUVColorspace);
    ENUM_VAL(ColorspaceType, CMYKColorspace);
    rb_define_const(Module_Magick, "SRGBColorspace"
                  , rm_enum_new(Class_ColorspaceType
                  , ID2SYM(rb_intern("SRGBColorspace"))
                  , INT2FIX(sRGBColorspace)));
#if defined(HAVE_HSLCOLORSPACE)
    ENUM_VAL(ColorspaceType, HSLColorspace);       // 5.5.7
#endif
#if defined(HAVE_HWBCOLORSPACE)
    ENUM_VAL(ColorspaceType, HWBColorspace);       // 5.5.7
#endif

    // ComplianceType constants are defined as enums but used as bit flags
    DEF_ENUM(ComplianceType);
    ENUM_VAL(ComplianceType, UndefinedCompliance);
    // AllCompliance is 0xffff, not too useful for us!
    rb_define_const(Module_Magick, "AllCompliance"
                  , rm_enum_new(Class_AnchorType
                  , ID2SYM(rb_intern("AllCompliance"))
                  , INT2FIX(SVGCompliance|X11Compliance|XPMCompliance)));

#if defined(HAVE_NOCOMPLIANCE)
    ENUM_VAL(ComplianceType, NoCompliance);
#endif
    ENUM_VAL(ComplianceType, SVGCompliance);
    ENUM_VAL(ComplianceType, X11Compliance);
    ENUM_VAL(ComplianceType, XPMCompliance);

    // CompositeOperator constants
    DEF_ENUM(CompositeOperator);
    ENUM_VAL(CompositeOperator, UndefinedCompositeOp);
    ENUM_VAL(CompositeOperator, OverCompositeOp);
    ENUM_VAL(CompositeOperator, InCompositeOp);
    ENUM_VAL(CompositeOperator, OutCompositeOp);
    ENUM_VAL(CompositeOperator, AtopCompositeOp);
    ENUM_VAL(CompositeOperator, XorCompositeOp);
    ENUM_VAL(CompositeOperator, PlusCompositeOp);
    ENUM_VAL(CompositeOperator, MinusCompositeOp);
    ENUM_VAL(CompositeOperator, AddCompositeOp);
    ENUM_VAL(CompositeOperator, SubtractCompositeOp);
    ENUM_VAL(CompositeOperator, DifferenceCompositeOp);
    ENUM_VAL(CompositeOperator, MultiplyCompositeOp);
    ENUM_VAL(CompositeOperator, BumpmapCompositeOp);
    ENUM_VAL(CompositeOperator, CopyCompositeOp);
    ENUM_VAL(CompositeOperator, CopyRedCompositeOp);
    ENUM_VAL(CompositeOperator, CopyGreenCompositeOp);
    ENUM_VAL(CompositeOperator, CopyBlueCompositeOp);
    ENUM_VAL(CompositeOperator, CopyOpacityCompositeOp);
    ENUM_VAL(CompositeOperator, ClearCompositeOp);
    ENUM_VAL(CompositeOperator, DissolveCompositeOp);
    ENUM_VAL(CompositeOperator, DisplaceCompositeOp);
    ENUM_VAL(CompositeOperator, ModulateCompositeOp);
    ENUM_VAL(CompositeOperator, ThresholdCompositeOp);
    ENUM_VAL(CompositeOperator, NoCompositeOp);
    ENUM_VAL(CompositeOperator, DarkenCompositeOp);
    ENUM_VAL(CompositeOperator, LightenCompositeOp);
    ENUM_VAL(CompositeOperator, HueCompositeOp);
    ENUM_VAL(CompositeOperator, SaturateCompositeOp);
    ENUM_VAL(CompositeOperator, ColorizeCompositeOp);
    ENUM_VAL(CompositeOperator, LuminizeCompositeOp);
    ENUM_VAL(CompositeOperator, ScreenCompositeOp);
    ENUM_VAL(CompositeOperator, OverlayCompositeOp);

#if defined(HAVE_COPYCYANCOMPOSITEOP)
                                        // CYMK added 5.5.7
    ENUM_VAL(CompositeOperator, CopyCyanCompositeOp);
    ENUM_VAL(CompositeOperator, CopyMagentaCompositeOp);
    ENUM_VAL(CompositeOperator, CopyYellowCompositeOp);
    ENUM_VAL(CompositeOperator, CopyBlackCompositeOp);
#endif

#if defined(HAVE_ANNOTATECOMPOSITEOP)
                                        // Added 5.5.8
    ENUM_VAL(CompositeOperator, AnnotateCompositeOp);
    ENUM_VAL(CompositeOperator, ReplaceCompositeOp);    // synonym for CopyCompositeOp
#endif

    // CompressionType constants
    DEF_ENUM(CompressionType);
    ENUM_VAL(CompressionType, UndefinedCompression);
    ENUM_VAL(CompressionType, NoCompression);
    ENUM_VAL(CompressionType, BZipCompression);
    ENUM_VAL(CompressionType, FaxCompression);
    ENUM_VAL(CompressionType, Group4Compression);
    ENUM_VAL(CompressionType, JPEGCompression);
    ENUM_VAL(CompressionType, LosslessJPEGCompression);
    ENUM_VAL(CompressionType, LZWCompression);
    ENUM_VAL(CompressionType, RunlengthEncodedCompression);
    ENUM_VAL(CompressionType, ZipCompression);

    // DecorationType constants
    DEF_ENUM(DecorationType);
    ENUM_VAL(DecorationType, NoDecoration);
    ENUM_VAL(DecorationType, UnderlineDecoration);
    ENUM_VAL(DecorationType, OverlineDecoration);
    ENUM_VAL(DecorationType, LineThroughDecoration);

#if defined(HAVE_DISPOSETYPE)
    // DisposeType constants (5.5.1)
    DEF_ENUM(DisposeType);
    ENUM_VAL(DisposeType, UndefinedDispose);
    ENUM_VAL(DisposeType, BackgroundDispose);
    ENUM_VAL(DisposeType, NoneDispose);
    ENUM_VAL(DisposeType, PreviousDispose);
#endif

    // FilterTypes constants
    DEF_ENUM(FilterTypes);
    ENUM_VAL(FilterTypes, UndefinedFilter);
    ENUM_VAL(FilterTypes, PointFilter);
    ENUM_VAL(FilterTypes, BoxFilter);
    ENUM_VAL(FilterTypes, TriangleFilter);
    ENUM_VAL(FilterTypes, HermiteFilter);
    ENUM_VAL(FilterTypes, HanningFilter);
    ENUM_VAL(FilterTypes, HammingFilter);
    ENUM_VAL(FilterTypes, BlackmanFilter);
    ENUM_VAL(FilterTypes, GaussianFilter);
    ENUM_VAL(FilterTypes, QuadraticFilter);
    ENUM_VAL(FilterTypes, CubicFilter);
    ENUM_VAL(FilterTypes, CatromFilter);
    ENUM_VAL(FilterTypes, MitchellFilter);
    ENUM_VAL(FilterTypes, LanczosFilter);
    ENUM_VAL(FilterTypes, BesselFilter);
    ENUM_VAL(FilterTypes, SincFilter);

    // GravityType constants
    DEF_ENUM(GravityType);
    ENUM_VAL(GravityType, ForgetGravity);
    ENUM_VAL(GravityType, NorthWestGravity);
    ENUM_VAL(GravityType, NorthGravity);
    ENUM_VAL(GravityType, NorthEastGravity);
    ENUM_VAL(GravityType, WestGravity);
    ENUM_VAL(GravityType, CenterGravity);
    ENUM_VAL(GravityType, EastGravity);
    ENUM_VAL(GravityType, SouthWestGravity);
    ENUM_VAL(GravityType, SouthGravity);
    ENUM_VAL(GravityType, SouthEastGravity);
    ENUM_VAL(GravityType, StaticGravity);

    // ImageType constants
    DEF_ENUM(ImageType);
    ENUM_VAL(ImageType, UndefinedType);
    ENUM_VAL(ImageType, BilevelType);
    ENUM_VAL(ImageType, GrayscaleType);
    ENUM_VAL(ImageType, GrayscaleMatteType);
    ENUM_VAL(ImageType, PaletteType);
    ENUM_VAL(ImageType, PaletteMatteType);
    ENUM_VAL(ImageType, TrueColorType);
    ENUM_VAL(ImageType, TrueColorMatteType);
    ENUM_VAL(ImageType, ColorSeparationType);
    ENUM_VAL(ImageType, ColorSeparationMatteType);
    ENUM_VAL(ImageType, OptimizeType);

    // InterlaceType constants
    DEF_ENUM(InterlaceType);
    ENUM_VAL(InterlaceType, UndefinedInterlace);
    ENUM_VAL(InterlaceType, NoInterlace);
    ENUM_VAL(InterlaceType, LineInterlace);
    ENUM_VAL(InterlaceType, PlaneInterlace);
    ENUM_VAL(InterlaceType, PartitionInterlace);

#if defined(HAVE_COMPAREIMAGECHANNELS)
    DEF_ENUM(MetricType);
    ENUM_VAL(MetricType, UndefinedMetric);
    ENUM_VAL(MetricType, MeanAbsoluteErrorMetric);
    ENUM_VAL(MetricType, MeanSquaredErrorMetric);
    ENUM_VAL(MetricType, PeakAbsoluteErrorMetric);
    ENUM_VAL(MetricType, PeakSignalToNoiseRatioMetric);
    ENUM_VAL(MetricType, RootMeanSquaredErrorMetric);
#endif

    // NoiseType constants
    DEF_ENUM(NoiseType);
    ENUM_VAL(NoiseType, UniformNoise);
    ENUM_VAL(NoiseType, GaussianNoise);
    ENUM_VAL(NoiseType, MultiplicativeGaussianNoise);
    ENUM_VAL(NoiseType, ImpulseNoise);
    ENUM_VAL(NoiseType, LaplacianNoise);
    ENUM_VAL(NoiseType, PoissonNoise);

    // Paint method constants
    DEF_ENUM(PaintMethod);
    ENUM_VAL(PaintMethod, PointMethod);
    ENUM_VAL(PaintMethod, ReplaceMethod);
    ENUM_VAL(PaintMethod, FloodfillMethod);
    ENUM_VAL(PaintMethod, FillToBorderMethod);
    ENUM_VAL(PaintMethod, ResetMethod);

    // PreviewType
    DEF_ENUM(PreviewType);
    ENUM_VAL(PreviewType, UndefinedPreview);
    ENUM_VAL(PreviewType, RotatePreview);
    ENUM_VAL(PreviewType, ShearPreview);
    ENUM_VAL(PreviewType, RollPreview);
    ENUM_VAL(PreviewType, HuePreview);
    ENUM_VAL(PreviewType, SaturationPreview);
    ENUM_VAL(PreviewType, BrightnessPreview);
    ENUM_VAL(PreviewType, GammaPreview);
    ENUM_VAL(PreviewType, SpiffPreview);
    ENUM_VAL(PreviewType, DullPreview);
    ENUM_VAL(PreviewType, GrayscalePreview);
    ENUM_VAL(PreviewType, QuantizePreview);
    ENUM_VAL(PreviewType, DespecklePreview);
    ENUM_VAL(PreviewType, ReduceNoisePreview);
    ENUM_VAL(PreviewType, AddNoisePreview);
    ENUM_VAL(PreviewType, SharpenPreview);
    ENUM_VAL(PreviewType, BlurPreview);
    ENUM_VAL(PreviewType, ThresholdPreview);
    ENUM_VAL(PreviewType, EdgeDetectPreview);
    ENUM_VAL(PreviewType, SpreadPreview);
    ENUM_VAL(PreviewType, SolarizePreview);
    ENUM_VAL(PreviewType, ShadePreview);
    ENUM_VAL(PreviewType, RaisePreview);
    ENUM_VAL(PreviewType, SegmentPreview);
    ENUM_VAL(PreviewType, SwirlPreview);
    ENUM_VAL(PreviewType, ImplodePreview);
    ENUM_VAL(PreviewType, WavePreview);
    ENUM_VAL(PreviewType, OilPaintPreview);
    ENUM_VAL(PreviewType, CharcoalDrawingPreview);
    ENUM_VAL(PreviewType, JPEGPreview);

    // RenderingIntent
    DEF_ENUM(RenderingIntent);
    ENUM_VAL(RenderingIntent, UndefinedIntent);
    ENUM_VAL(RenderingIntent, SaturationIntent);
    ENUM_VAL(RenderingIntent, PerceptualIntent);
    ENUM_VAL(RenderingIntent, AbsoluteIntent);
    ENUM_VAL(RenderingIntent, RelativeIntent);

    // ResolutionType constants
    DEF_ENUM(ResolutionType);
    ENUM_VAL(ResolutionType, UndefinedResolution);
    ENUM_VAL(ResolutionType, PixelsPerInchResolution);
    ENUM_VAL(ResolutionType, PixelsPerCentimeterResolution);

    // StretchType constants
    DEF_ENUM(StretchType);
    ENUM_VAL(StretchType, NormalStretch);
    ENUM_VAL(StretchType, UltraCondensedStretch);
    ENUM_VAL(StretchType, ExtraCondensedStretch);
    ENUM_VAL(StretchType, CondensedStretch);
    ENUM_VAL(StretchType, SemiCondensedStretch);
    ENUM_VAL(StretchType, SemiExpandedStretch);
    ENUM_VAL(StretchType, ExpandedStretch);
    ENUM_VAL(StretchType, ExtraExpandedStretch);
    ENUM_VAL(StretchType, UltraExpandedStretch);
    ENUM_VAL(StretchType, AnyStretch);

    // StyleType constants
    DEF_ENUM(StyleType);
    ENUM_VAL(StyleType, NormalStyle);
    ENUM_VAL(StyleType, ItalicStyle);
    ENUM_VAL(StyleType, ObliqueStyle);
    ENUM_VAL(StyleType, AnyStyle);

    // WeightType constants
    DEF_ENUM(WeightType);
    ENUM_VAL(WeightType, AnyWeight);
    ENUM_VAL(WeightType, NormalWeight);
    ENUM_VAL(WeightType, BoldWeight);
    ENUM_VAL(WeightType, BolderWeight);
    ENUM_VAL(WeightType, LighterWeight);

    /*-----------------------------------------------------------------------*/
    /* Struct classes                                                        */
    /*-----------------------------------------------------------------------*/

    // Pass NULL as the structure name to keep them from polluting the Struct
    // namespace. The only way to use these classes is via the Magick:: namespace.

    // Magick::AffineMatrix
    Class_AffineMatrix = rb_struct_define(NULL, "sx", "rx", "ry", "sy", "tx", "ty", 0);
    rb_define_const(Module_Magick, "AffineMatrix", Class_AffineMatrix);

    // Magick::Pixel has 3 constructors: "new" "from_color", and "from_HSL".
    Class_Pixel = rb_struct_define(NULL, "red", "green", "blue", "opacity", 0);
    rb_define_singleton_method(Class_Pixel, "from_color", Pixel_from_color, 1);
    rb_define_singleton_method(Class_Pixel, "from_HSL", Pixel_from_HSL, 1);
    rb_define_method(Class_Pixel, "fcmp", Pixel_fcmp, -1);
    rb_define_method(Class_Pixel, "intensity", Pixel_intensity, 0);
    rb_define_method(Class_Pixel, "to_color", Pixel_to_color, -1);
    rb_define_method(Class_Pixel, "to_HSL", Pixel_to_HSL, 0);
    rb_define_method(Class_Pixel, "to_s", Pixel_to_s, 0);
    rb_define_const(Module_Magick, "Pixel", Class_Pixel);

    // Magick::Primary
    Class_Primary = rb_struct_define(NULL, "x", "y", "z", 0);
    rb_define_method(Class_Primary, "to_s", PrimaryInfo_to_s, 0);
    rb_define_const(Module_Magick, "Primary", Class_Primary);

    // Magick::Chromaticity
    Class_Chromaticity = rb_struct_define(NULL
                                            , "red_primary"
                                            , "green_primary"
                                            , "blue_primary"
                                            , "white_point"
                                            , 0);
    rb_define_method(Class_Chromaticity, "to_s", ChromaticityInfo_to_s, 0);
    rb_define_const(Module_Magick, "Chromaticity", Class_Chromaticity);

    // Magick::Color
    Class_Color = rb_struct_define(NULL, "name", "compliance", "color", 0);
    rb_define_method(Class_Color, "to_s", Color_to_s, 0);
    rb_define_const(Module_Magick, "Color", Class_Color);

    // Magick::Point
    Class_Point = rb_struct_define(NULL, "x", "y", 0);
    rb_define_const(Module_Magick, "Point", Class_Point);

    // Magick::Rectangle
    Class_Rectangle = rb_struct_define(NULL, "width", "height", "x", "y", 0);
    rb_define_method(Class_Rectangle, "to_s", RectangleInfo_to_s, 0);
    rb_define_const(Module_Magick, "Rectangle", Class_Rectangle);

    // Magick::Segment
    Class_Segment = rb_struct_define(NULL, "x1", "y1", "x2", "y2", 0);
    rb_define_method(Class_Segment, "to_s", SegmentInfo_to_s, 0);
    rb_define_const(Module_Magick, "Segment", Class_Segment);

    // Magick::Font
    Class_Font = rb_struct_define(NULL, "name", "description",
                                      "family", "style", "stretch", "weight",
                                      "encoding", "foundry", "format", 0);
    rb_define_method(Class_Font, "to_s", Font_to_s, 0);
    rb_define_const(Module_Magick, "Font", Class_Font);

    // Magick::TypeMetric
    Class_TypeMetric = rb_struct_define(NULL, "pixels_per_em", "ascent", "descent",
                                        "width", "height", "max_advance", "bounds",
                                        "underline_position", "underline_thickness", 0);
    rb_define_method(Class_TypeMetric, "to_s", TypeMetric_to_s, 0);
    rb_define_const(Module_Magick, "TypeMetric", Class_TypeMetric);


    /*-----------------------------------------------------------------------*/
    /* Create IDs for frequently used methods                                */
    /*-----------------------------------------------------------------------*/

    new_ID = rb_intern("new");
    push_ID = rb_intern("push");
    length_ID = rb_intern("length");
    cur_image_ID = rb_intern("cur_image");
    values_ID = rb_intern("values");
    to_s_ID = rb_intern("to_s");
    _dummy_img__ID = rb_intern("_dummy_img_");
}

/*
    Static:     version_constants
    Purpose:    create Version, Magick_version, and Version_long constants.
*/
static void version_constants(void)
{
    const char *mgk_version;
    char long_version[300];

    mgk_version = GetMagickVersion(NULL);
    rb_define_const(Module_Magick, "Magick_version", rb_str_new2(mgk_version));

    rb_define_const(Module_Magick, "Version", rb_str_new2(PACKAGE_STRING));
    sprintf(long_version,
        "This is %s ($Date: 2003/12/25 21:15:37 $) Copyright (C) 2003 by Timothy P. Hunter\n"
        "Built with %s\n"
        "Built for %s\n"
        "Web page: http://rmagick.rubyforge.org\n"
        "Email: rmagick@rubyforge.org\n",
        PACKAGE_STRING, mgk_version, RUBY_VERSION_STRING);
    rb_define_const(Module_Magick, "Long_version", rb_str_new2(long_version));

}
