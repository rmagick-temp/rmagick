/* $Id: rmmain.c,v 1.101 2005/12/31 14:35:59 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2005 by Timothy P. Hunter
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
    Method:     Magick::colors [ { |colorinfo| } ]
    Purpose:    If called with the optional block, iterates over the colors,
                otherwise returns an array of Magick::Color objects
    Notes:      There are 3 implementations

*/
VALUE
Magick_colors(VALUE class)
{
#if defined(HAVE_GETCOLORINFOARRAY) // GraphicsMagick
    ColorInfo **color_ary;
    ExceptionInfo exception;
    volatile VALUE ary;
    int x;

    GetExceptionInfo(&exception);

    color_ary = GetColorInfoArray(&exception);
    HANDLE_ERROR

    if (rb_block_given_p())
    {
        x = 0;
        while(color_ary[x])
        {
            rb_yield(Color_from_ColorInfo(color_ary[x]));
            x += 1;
        }
        magick_free(color_ary);
        return class;
    }
    else
    {
        ary = rb_ary_new();

        x = 0;
        while (color_ary[x])
        {
            rb_ary_push(ary, Color_from_ColorInfo(color_ary[x]));
            x += 1;
        }
        magick_free(color_ary);
        return ary;
    }

#elif defined(HAVE_GETCOLORINFOLIST)    // ImageMagick 6.0.0

    const ColorInfo **color_info_list;
    unsigned long number_colors, x;
    volatile VALUE ary;

#if defined(HAVE_OLD_GETCOLORINFOLIST)
    color_info_list = GetColorInfoList("*", &number_colors);

#else
    // IM 6.1.3 added an exception parm to GetColorInfoList
    ExceptionInfo exception;

    GetExceptionInfo(&exception);

    color_info_list = GetColorInfoList("*", &number_colors, &exception);
    HANDLE_ERROR
#endif


    if (rb_block_given_p())
    {
        for(x = 0; x < number_colors; x++)
        {
            rb_yield(Color_from_ColorInfo(color_info_list[x]));
        }
        magick_free(color_info_list);
        return class;
    }
    else
    {
        ary = rb_ary_new2((long) number_colors);
        for(x = 0; x < number_colors; x++)
        {
            rb_ary_push(ary, Color_from_ColorInfo(color_info_list[x]));
        }

        magick_free(color_info_list);
        return ary;
    }

#else   // ImageMagick < 6.0.0

    const ColorInfo *color_list;
    ColorInfo *color;
    ExceptionInfo exception;
    volatile VALUE ary, el;

    GetExceptionInfo(&exception);

    color_list = GetColorInfo("*", &exception);
    DestroyExceptionInfo(&exception);

    // The order of the colors list can change in mid-iteration,
    // so the only way we can guarantee a single pass thru the list
    // is to copy the elements into an array without returning to
    // IM. So, we always create a Ruby array and either return it
    // or iterate over it.
    ary = rb_ary_new();
    for (color = (ColorInfo *)color_list; color; color = color->next)
    {
        rb_ary_push(ary, Color_from_ColorInfo(color));
    }

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

#endif
}

/*
  Method:   Magick::fonts [ { |fontinfo| } ]
  Purpose:  If called with the optional block, iterates over the fonts,
            otherwise returns an array of Magick::Font objects
*/
VALUE
Magick_fonts(VALUE class)
{
#if defined(HAVE_GETTYPEINFOLIST)   // ImageMagick 6.0.0

    const TypeInfo **type_info;
    unsigned long number_types, x;
    volatile VALUE ary;

#if defined(HAVE_OLD_GETTYPEINFOLIST)
    type_info = GetTypeInfoList("*", &number_types);

#else
    // IM 6.1.3 added an exception argument to GetTypeInfoList
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    type_info = GetTypeInfoList("*", &number_types, &exception);
    HANDLE_ERROR
#endif

    if (rb_block_given_p())
    {
        for(x = 0; x < number_types; x++)
        {
            rb_yield(Font_from_TypeInfo((TypeInfo *)type_info[x]));
        }
        magick_free(type_info);
        return class;
    }
    else
    {
        ary = rb_ary_new2(number_types);
        for(x = 0; x < number_types; x++)
        {
            rb_ary_push(ary, Font_from_TypeInfo((TypeInfo *)type_info[x]));
        }
        magick_free(type_info);
        return ary;
    }

#else

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

#endif
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
            There are 3 implementations.
*/

static VALUE MagickInfo_to_format(MagickInfo *magick_info)
{
    char mode[4];

    mode[0] = magick_info->blob_support ? '*': ' ';
    mode[1] = magick_info->decoder ? 'r' : '-';
    mode[2] = magick_info->encoder ? 'w' : '-';
    mode[3] = magick_info->encoder && magick_info->adjoin ? '+' : '-';

    return rb_str_new(mode, sizeof(mode));
}

VALUE
Magick_init_formats(VALUE class)
{
#if defined(HAVE_GETMAGICKINFOARRAY)    // GraphicsMagick

    MagickInfo *magick_info, *m;
    volatile VALUE formats;
    ExceptionInfo exception;

    class = class;      // defeat "never referenced" message from icc

    formats = rb_hash_new();

    GetExceptionInfo(&exception);
    magick_info = (MagickInfo *)GetMagickInfoArray(&exception);
    for(m = magick_info; m != NULL; m = m->next)
    {
        rb_hash_aset(formats, rb_str_new2(m->name), MagickInfo_to_format(m));
    }

    magick_free(magick_info);
    return formats;

#elif defined(HAVE_GETMAGICKINFOLIST)    // ImageMagick 6.0.0

    const MagickInfo **magick_info;
    unsigned long number_formats, x;
    volatile VALUE formats;
#if !defined(HAVE_OLD_GETMAGICKINFOLIST)
    ExceptionInfo exception;
#endif

    class = class;      // defeat "never referenced" message from icc
    formats = rb_hash_new();

#if defined(HAVE_OLD_GETMAGICKINFOLIST)
    magick_info = GetMagickInfoList("*", &number_formats);

#else
    // IM 6.1.3 added an exception argument to GetMagickInfoList
    GetExceptionInfo(&exception);
    magick_info = GetMagickInfoList("*", &number_formats, &exception);
    HANDLE_ERROR
#endif

    for(x = 0; x < number_formats; x++)
    {
        rb_hash_aset(formats
            , rb_str_new2(magick_info[x]->name)
            , MagickInfo_to_format((MagickInfo *)magick_info[x]));
    }
    return formats;

#else

    MagickInfo *m;
    volatile VALUE formats;
    ExceptionInfo exception;

    class = class;      // defeat "never referenced" message from icc

    formats = rb_hash_new();

    GetExceptionInfo(&exception);
    m = (MagickInfo *)GetMagickInfo("*", &exception);
    HANDLE_ERROR

    for ( ; m != NULL; m = m->next)
    {
        rb_hash_aset(formats, rb_str_new2(m->name), MagickInfo_to_format(m));
    }

    return formats;

#endif
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
        (void) rb_funcall2((VALUE)monitor, ID_call, 3, (VALUE *)args);
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

#if defined(HAVE_SETIMAGEPROGRESSMONITOR)
        rb_warning("Magick.set_monitor is deprecated; use Image#monitor= or Image::Info#monitor= instead.");
#endif

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
    rm_not_implemented();
    return (VALUE)0;
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
    volatile VALUE observable;

    InitializeMagick("RMagick");

    Module_Magick = rb_define_module("Magick");

    /*-----------------------------------------------------------------------*/
    /* Create IDs for frequently used methods, etc.                          */
    /*-----------------------------------------------------------------------*/

    ID__dummy_img_      = rb_intern("_dummy_img_");
    ID_call             = rb_intern("call");
    ID_changed          = rb_intern("changed");
    ID_cur_image        = rb_intern("cur_image");
    ID_dup              = rb_intern("dup");
    ID_enumerators      = rb_intern("enumerators");
    ID_fill             = rb_intern("fill");
    ID_flag             = rb_intern("flag");
    ID_from_s           = rb_intern("from_s");
    ID_Geometry         = rb_intern("Geometry");
    ID_GeometryValue    = rb_intern("GeometryValue");
    ID_height           = rb_intern("height");
    ID_initialize_copy  = rb_intern("initialize_copy");
    ID_length           = rb_intern("length");
    ID_notify_observers = rb_intern("notify_observers");
    ID_new              = rb_intern("new");
    ID_push             = rb_intern("push");
    ID_spaceship        = rb_intern("<=>");
    ID_to_s             = rb_intern("to_s");
    ID_values           = rb_intern("values");
    ID_width            = rb_intern("width");
    ID_x                = rb_intern("x");
    ID_y                = rb_intern("y");

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
    rb_define_singleton_method(Class_Image, "read_inline", Image_read_inline, 1);
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
    DCL_ATTR_ACCESSOR(Image, endian)
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
    DCL_ATTR_WRITER(Image, monitor)
    DCL_ATTR_ACCESSOR(Image, montage)
    DCL_ATTR_READER(Image, normalized_mean_error)
    DCL_ATTR_READER(Image, normalized_maximum_error)
    DCL_ATTR_READER(Image, number_colors)
    DCL_ATTR_ACCESSOR(Image, offset)
    DCL_ATTR_WRITER(Image, opacity)
    DCL_ATTR_READER(Image, orientation)
    DCL_ATTR_ACCESSOR(Image, page)
#if defined(HAVE_IMAGE_QUALITY)
    DCL_ATTR_READER(Image, quality)
#endif
    DCL_ATTR_READER(Image, quantum_depth)
    DCL_ATTR_ACCESSOR(Image, rendering_intent)
    DCL_ATTR_READER(Image, rows)
    DCL_ATTR_READER(Image, scene)
    DCL_ATTR_ACCESSOR(Image, start_loop)
    DCL_ATTR_ACCESSOR(Image, class_type)
    DCL_ATTR_ACCESSOR(Image, ticks_per_second)
    DCL_ATTR_ACCESSOR(Image, tile_info)
    DCL_ATTR_READER(Image, total_colors)
    DCL_ATTR_ACCESSOR(Image, units)
    DCL_ATTR_ACCESSOR(Image, virtual_pixel_method)
    DCL_ATTR_ACCESSOR(Image, x_resolution)
    DCL_ATTR_ACCESSOR(Image, y_resolution)

    rb_define_method(Class_Image, "adaptive_threshold", Image_adaptive_threshold, -1);
    rb_define_method(Class_Image, "add_noise", Image_add_noise, 1);
    rb_define_method(Class_Image, "affine_transform", Image_affine_transform, 1);
    rb_define_method(Class_Image, "[]", Image_aref, 1);
    rb_define_method(Class_Image, "[]=", Image_aset, 2);
    rb_define_method(Class_Image, "properties", Image_properties, 0);
    rb_define_method(Class_Image, "bilevel_channel", Image_bilevel_channel, -1);
    rb_define_method(Class_Image, "black_threshold", Image_black_threshold, -1);
    rb_define_method(Class_Image, "blur_image", Image_blur_image, -1);
    rb_define_method(Class_Image, "blur_channel", Image_blur_channel, -1);
    rb_define_method(Class_Image, "border", Image_border, 3);
    rb_define_method(Class_Image, "border!", Image_border_bang, 3);
    rb_define_method(Class_Image, "change_geometry", Image_change_geometry, 1);
    rb_define_method(Class_Image, "change_geometry!", Image_change_geometry, 1);
    rb_define_method(Class_Image, "changed?", Image_changed_q, 0);
    rb_define_method(Class_Image, "channel", Image_channel, 1);
    // An alias for compare_channel
    rb_define_method(Class_Image, "channel_compare", Image_compare_channel, -1);
    rb_define_method(Class_Image, "compare_channel", Image_compare_channel, -1);
    rb_define_method(Class_Image, "channel_depth", Image_channel_depth, -1);
    rb_define_method(Class_Image, "channel_extrema", Image_channel_extrema, -1);
    rb_define_method(Class_Image, "channel_mean", Image_channel_mean, -1);
    rb_define_method(Class_Image, "channel_threshold", Image_channel_threshold, -1);
    rb_define_method(Class_Image, "charcoal", Image_charcoal, -1);
    rb_define_method(Class_Image, "chop", Image_chop, 4);
    rb_define_method(Class_Image, "clone", Image_clone, 0);
    rb_define_method(Class_Image, "color_flood_fill", Image_color_flood_fill, 5);
    rb_define_method(Class_Image, "color_histogram", Image_color_histogram, 0);
    rb_define_method(Class_Image, "colorize", Image_colorize, -1);
    rb_define_method(Class_Image, "colormap", Image_colormap, -1);
    rb_define_method(Class_Image, "composite", Image_composite, -1);
    rb_define_method(Class_Image, "composite!", Image_composite_bang, -1);
    rb_define_method(Class_Image, "composite_affine", Image_composite_affine, 2);
    rb_define_method(Class_Image, "compress_colormap!", Image_compress_colormap_bang, 0);
    rb_define_method(Class_Image, "contrast", Image_contrast, -1);
    rb_define_method(Class_Image, "convolve", Image_convolve, 2);
    rb_define_method(Class_Image, "convolve_channel", Image_convolve_channel, -1);
    rb_define_method(Class_Image, "copy", Image_copy, 0);
    rb_define_method(Class_Image, "crop", Image_crop, -1);
    rb_define_method(Class_Image, "crop!", Image_crop_bang, -1);
    rb_define_method(Class_Image, "cycle_colormap", Image_cycle_colormap, 1);
    rb_define_method(Class_Image, "despeckle", Image_despeckle, 0);
    rb_define_method(Class_Image, "difference", Image_difference, 1);
    rb_define_method(Class_Image, "dispatch", Image_dispatch, -1);
    rb_define_method(Class_Image, "display", Image_display, 0);
    rb_define_method(Class_Image, "distortion_channel", Image_distortion_channel, -1);
    rb_define_method(Class_Image, "_dump", Image__dump, 1);
    rb_define_method(Class_Image, "dup", Image_dup, 0);
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
    rb_define_method(Class_Image, "gamma_channel", Image_gamma_channel, -1);
    rb_define_method(Class_Image, "gamma_correct", Image_gamma_correct, -1);
    rb_define_method(Class_Image, "gaussian_blur", Image_gaussian_blur, -1);
    rb_define_method(Class_Image, "gaussian_blur_channel", Image_gaussian_blur_channel, -1);
    rb_define_method(Class_Image, "get_pixels", Image_get_pixels, 4);
    rb_define_method(Class_Image, "gray?", Image_gray_q, 0);
    rb_define_method(Class_Image, "grey?", Image_gray_q, 0);
    rb_define_method(Class_Image, "grayscale_pseudo_class", Image_grayscale_pseudo_class, -1);
    rb_define_method(Class_Image, "implode", Image_implode, -1);
    rb_define_method(Class_Image, "import_pixels", Image_import_pixels, -1);
    rb_define_method(Class_Image, "initialize_copy", Image_init_copy, 1);
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
    rb_define_method(Class_Image, "negate_channel", Image_negate_channel, -1);
    rb_define_method(Class_Image, "normalize", Image_normalize, 0);
    rb_define_method(Class_Image, "normalize_channel", Image_normalize_channel, -1);
    rb_define_method(Class_Image, "oil_paint", Image_oil_paint, -1);
    rb_define_method(Class_Image, "opaque", Image_opaque, 2);
    rb_define_method(Class_Image, "opaque?", Image_opaque_q, 0);
    rb_define_method(Class_Image, "ordered_dither", Image_ordered_dither, 0);
    rb_define_method(Class_Image, "palette?", Image_palette_q, 0);
    rb_define_method(Class_Image, "pixel_color", Image_pixel_color, -1);
    rb_define_method(Class_Image, "posterize", Image_posterize, -1);
//  rb_define_method(Class_Image, "plasma", Image_plasma, 6);
    rb_define_method(Class_Image, "preview", Image_preview, 1);
    rb_define_method(Class_Image, "profile!", Image_profile_bang, 2);
    rb_define_method(Class_Image, "quantize", Image_quantize, -1);
    rb_define_method(Class_Image, "quantum_operator", Image_quantum_operator, -1);
    rb_define_method(Class_Image, "radial_blur", Image_radial_blur, 1);
    rb_define_method(Class_Image, "raise", Image_raise, -1);
    rb_define_method(Class_Image, "random_channel_threshold", Image_random_channel_threshold, 2);
    rb_define_method(Class_Image, "random_threshold_channel", Image_random_threshold_channel, -1);
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
    rb_define_method(Class_Image, "sepiatone", Image_sepiatone, -1);
    rb_define_method(Class_Image, "set_channel_depth", Image_set_channel_depth, 2);
    rb_define_method(Class_Image, "shade", Image_shade, -1);
    rb_define_method(Class_Image, "shadow", Image_shadow, -1);
    rb_define_method(Class_Image, "sharpen", Image_sharpen, -1);
    rb_define_method(Class_Image, "sharpen_channel", Image_sharpen_channel, -1);
    rb_define_method(Class_Image, "shave", Image_shave, 2);
    rb_define_method(Class_Image, "shave!", Image_shave_bang, 2);
    rb_define_method(Class_Image, "shear", Image_shear, 2);
    rb_define_method(Class_Image, "sigmoidal_contrast_channel", Image_sigmoidal_contrast_channel, -1);
    rb_define_method(Class_Image, "signature", Image_signature, 0);
    rb_define_method(Class_Image, "solarize", Image_solarize, -1);
    rb_define_method(Class_Image, "<=>", Image_spaceship, 1);
    rb_define_method(Class_Image, "splice", Image_splice, -1);
    rb_define_method(Class_Image, "spread", Image_spread, -1);
    rb_define_method(Class_Image, "statistics", Image_statistics, 0);
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
    rb_define_method(Class_Image, "trim", Image_trim, 0);
    rb_define_method(Class_Image, "trim!", Image_trim_bang, 0);
    rb_define_method(Class_Image, "unsharp_mask", Image_unsharp_mask, -1);
    rb_define_method(Class_Image, "unsharp_mask_channel", Image_unsharp_mask_channel, -1);
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
    rb_define_alias(Class_ImageList, "__ary_map__", "map");

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
    DCL_ATTR_WRITER(Draw, stroke_width)
    DCL_ATTR_WRITER(Draw, text_antialias)
    DCL_ATTR_WRITER(Draw, undercolor)

    rb_define_method(Class_Draw, "annotate", Draw_annotate, 6);
    rb_define_method(Class_Draw, "clone", Draw_clone, 0);
    rb_define_method(Class_Draw, "composite", Draw_composite, -1);
    rb_define_method(Class_Draw, "draw", Draw_draw, 1);
    rb_define_method(Class_Draw, "dup", Draw_dup, 0);
    rb_define_method(Class_Draw, "get_type_metrics", Draw_get_type_metrics, -1);
    rb_define_method(Class_Draw, "get_multiline_type_metrics", Draw_get_multiline_type_metrics, -1);
    rb_define_method(Class_Draw, "initialize", Draw_initialize, 0);
    rb_define_method(Class_Draw, "initialize_copy", Draw_init_copy, 1);
    rb_define_method(Class_Draw, "inspect", Draw_inspect, 0);
    rb_define_method(Class_Draw, "primitive", Draw_primitive, 1);

    /*-----------------------------------------------------------------------*/
    /* Class Magick::Pixel                                                   */
    /*-----------------------------------------------------------------------*/

    Class_Pixel = rb_define_class_under(Module_Magick, "Pixel", rb_cObject);

    // include Observable in Pixel for Image::View class
    rb_require("observer");
    observable = rb_const_get(rb_cObject, rb_intern("Observable"));
    rb_include_module(Class_Pixel, observable);

    // include Comparable
    rb_include_module(Class_Pixel, rb_mComparable);

    // Magick::Pixel has 3 constructors: "new" "from_color", and "from_HSL".
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_define_alloc_func(Class_Pixel, Pixel_alloc);
#else
    rb_define_singleton_method(Class_Pixel, "new", Pixel_new, -1);
#endif
    rb_define_singleton_method(Class_Pixel, "from_color", Pixel_from_color, 1);
    rb_define_singleton_method(Class_Pixel, "from_HSL", Pixel_from_HSL, 1);

    // Define the RGBA attributes
    DCL_ATTR_ACCESSOR(Pixel, red)
    DCL_ATTR_ACCESSOR(Pixel, green)
    DCL_ATTR_ACCESSOR(Pixel, blue)
    DCL_ATTR_ACCESSOR(Pixel, opacity)

    // Define the CMYK attributes
    DCL_ATTR_ACCESSOR(Pixel, cyan)
    DCL_ATTR_ACCESSOR(Pixel, magenta)
    DCL_ATTR_ACCESSOR(Pixel, yellow)
    DCL_ATTR_ACCESSOR(Pixel, black)


    // Define the instance methods
    rb_define_method(Class_Pixel, "<=>", Pixel_spaceship, 1);
    rb_define_method(Class_Pixel, "===", Pixel_case_eq, 1);
    rb_define_method(Class_Pixel, "initialize", Pixel_initialize, -1);
    rb_define_method(Class_Pixel, "initialize_copy", Pixel_init_copy, 1);
    rb_define_method(Class_Pixel, "clone", Pixel_clone, 0);
    rb_define_method(Class_Pixel, "dup", Pixel_dup, 0);
    rb_define_method(Class_Pixel, "fcmp", Pixel_fcmp, -1);
    rb_define_method(Class_Pixel, "intensity", Pixel_intensity, 0);
    rb_define_method(Class_Pixel, "to_color", Pixel_to_color, -1);
    rb_define_method(Class_Pixel, "to_HSL", Pixel_to_HSL, 0);
    rb_define_method(Class_Pixel, "to_s", Pixel_to_s, 0);

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
    rb_define_method(Class_Montage, "freeze", rm_no_freeze, 0);

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
    rb_define_method(Class_Info, "freeze", rm_no_freeze, 0);
    rb_define_method(Class_Info, "define", Info_define, -1);
    rb_define_method(Class_Info, "[]=", Info_aset, 3);
    rb_define_method(Class_Info, "[]", Info_aref, 2);
    rb_define_method(Class_Info, "undefine", Info_undefine, 2);

    DCL_ATTR_ACCESSOR(Info, antialias)
    DCL_ATTR_ACCESSOR(Info, authenticate)
    DCL_ATTR_ACCESSOR(Info, background_color)
    DCL_ATTR_ACCESSOR(Info, border_color)
    DCL_ATTR_ACCESSOR(Info, colorspace)
    DCL_ATTR_ACCESSOR(Info, comment)        // new in 6.0.0
    DCL_ATTR_ACCESSOR(Info, compression)
    DCL_ATTR_ACCESSOR(Info, delay)          // new in 6.0.0
    DCL_ATTR_ACCESSOR(Info, density)
    DCL_ATTR_ACCESSOR(Info, depth)
    DCL_ATTR_ACCESSOR(Info, dispose)        // new in 6.0.0
    DCL_ATTR_ACCESSOR(Info, dither)
    DCL_ATTR_ACCESSOR(Info, extract)    // new in 5.5.6, replaces tile
    DCL_ATTR_ACCESSOR(Info, filename)
    DCL_ATTR_ACCESSOR(Info, fill)
    DCL_ATTR_ACCESSOR(Info, font)
    DCL_ATTR_ACCESSOR(Info, format)
    DCL_ATTR_ACCESSOR(Info, fuzz)
    DCL_ATTR_ACCESSOR(Info, gravity)
    DCL_ATTR_ACCESSOR(Info, group)
    DCL_ATTR_ACCESSOR(Info, interlace)
    DCL_ATTR_ACCESSOR(Info, label)          // new in 6.0.0
    DCL_ATTR_ACCESSOR(Info, matte_color)
    DCL_ATTR_WRITER(Info, monitor)
    DCL_ATTR_ACCESSOR(Info, monochrome)
    DCL_ATTR_ACCESSOR(Info, number_scenes)  // new in 5.5.6, replaces subrange
    DCL_ATTR_ACCESSOR(Info, orientation)    // new in 6.0.0
    DCL_ATTR_ACCESSOR(Info, page)
    DCL_ATTR_ACCESSOR(Info, pointsize)
    DCL_ATTR_ACCESSOR(Info, quality)
    DCL_ATTR_ACCESSOR(Info, sampling_factor)
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
    rb_define_method(Class_ImageMagickError, "initialize", ImageMagickError_initialize, -1);
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
    DEF_ENUM(AlignType)
        ENUMERATOR(UndefinedAlign)
        ENUMERATOR(LeftAlign)
        ENUMERATOR(CenterAlign)
        ENUMERATOR(RightAlign)
    END_ENUM

    // AnchorType constants (for Draw#text_anchor - these are not defined by ImageMagick)
    DEF_ENUM(AnchorType)
        ENUMERATOR(StartAnchor)
        ENUMERATOR(MiddleAnchor)
        ENUMERATOR(EndAnchor)
    END_ENUM

    // ChannelType constants
    DEF_ENUM(ChannelType)
        ENUMERATOR(UndefinedChannel)
        ENUMERATOR(RedChannel)
        ENUMERATOR(CyanChannel)
        ENUMERATOR(GreenChannel)
        ENUMERATOR(MagentaChannel)
        ENUMERATOR(BlueChannel)
        ENUMERATOR(YellowChannel)
        ENUMERATOR(OpacityChannel)
        ENUMERATOR(BlackChannel)
        ENUMERATOR(MatteChannel)
#if defined(HAVE_INDEXCHANNEL)
        ENUMERATOR(IndexChannel)    // 5.5.8
#endif
#if defined(HAVE_GRAYCHANNEL)
        ENUMERATOR(GrayChannel)
#endif
#if defined(HAVE_ALLCHANNELS)
        ENUMERATOR(AllChannels)
#endif
    END_ENUM

    // ClassType constants
    DEF_ENUM(ClassType)
        ENUMERATOR(UndefinedClass)
        ENUMERATOR(PseudoClass)
        ENUMERATOR(DirectClass)
    END_ENUM

    // ColorspaceType constants
    DEF_ENUM(ColorspaceType)
        ENUMERATOR(UndefinedColorspace)
        ENUMERATOR(RGBColorspace)
        ENUMERATOR(GRAYColorspace)
        ENUMERATOR(TransparentColorspace)
        ENUMERATOR(OHTAColorspace)
        ENUMERATOR(XYZColorspace)
        ENUMERATOR(YCbCrColorspace)
        ENUMERATOR(YCCColorspace)
        ENUMERATOR(YIQColorspace)
        ENUMERATOR(YPbPrColorspace)
        ENUMERATOR(YUVColorspace)
        ENUMERATOR(CMYKColorspace)
        rb_define_const(Module_Magick, "SRGBColorspace"
                      , rm_enum_new(Class_ColorspaceType
                      , ID2SYM(rb_intern("SRGBColorspace"))
                      , INT2FIX(sRGBColorspace)));
#if defined(HAVE_HSLCOLORSPACE)
        ENUMERATOR(HSLColorspace)       // 5.5.7
#endif
#if defined(HAVE_HWBCOLORSPACE)
        ENUMERATOR(HWBColorspace)       // 5.5.7
#endif
#if defined(HAVE_HSBCOLORSPACE)
        ENUMERATOR(HSBColorspace)       // 6.0.0
#endif
    END_ENUM

    // ComplianceType constants are defined as enums but used as bit flags
    DEF_ENUM(ComplianceType)
        ENUMERATOR(UndefinedCompliance)

        // AllCompliance is 0xffff, not too useful for us!
        rb_define_const(Module_Magick, "AllCompliance"
                  , rm_enum_new(Class_AnchorType
                  , ID2SYM(rb_intern("AllCompliance"))
                  , INT2FIX(SVGCompliance|X11Compliance|XPMCompliance)));

#if defined(HAVE_NOCOMPLIANCE)
        ENUMERATOR(NoCompliance)
#endif
        ENUMERATOR(SVGCompliance)
        ENUMERATOR(X11Compliance)
        ENUMERATOR(XPMCompliance)
    END_ENUM

    // CompositeOperator constants
    DEF_ENUM(CompositeOperator)
        ENUMERATOR(UndefinedCompositeOp)
        ENUMERATOR(NoCompositeOp)
        ENUMERATOR(AddCompositeOp)
        ENUMERATOR(AtopCompositeOp)
        ENUMERATOR(BumpmapCompositeOp)
        ENUMERATOR(ClearCompositeOp)
        ENUMERATOR(ColorizeCompositeOp)
        ENUMERATOR(CopyBlueCompositeOp)
        ENUMERATOR(CopyCompositeOp)
        ENUMERATOR(CopyGreenCompositeOp)
        ENUMERATOR(CopyOpacityCompositeOp)
        ENUMERATOR(CopyRedCompositeOp)
#if defined(HAVE_COPYCYANCOMPOSITEOP)   // CYMK added 5.5.7
        ENUMERATOR(CopyCyanCompositeOp)
        ENUMERATOR(CopyMagentaCompositeOp)
        ENUMERATOR(CopyYellowCompositeOp)
        ENUMERATOR(CopyBlackCompositeOp)
#endif
        ENUMERATOR(DarkenCompositeOp)
        ENUMERATOR(DifferenceCompositeOp)
        ENUMERATOR(DisplaceCompositeOp)
        ENUMERATOR(DissolveCompositeOp)
#if defined(HAVE_DSTCOMPOSITEOP)
        ENUMERATOR(DstAtopCompositeOp)    // Added 6.0.2?
        ENUMERATOR(DstCompositeOp)
        ENUMERATOR(DstInCompositeOp)
        ENUMERATOR(DstOutCompositeOp)
        ENUMERATOR(DstOverCompositeOp)
#endif
        ENUMERATOR(HueCompositeOp)
        ENUMERATOR(InCompositeOp)
        ENUMERATOR(LightenCompositeOp)
        ENUMERATOR(LuminizeCompositeOp)
        ENUMERATOR(MinusCompositeOp)
        ENUMERATOR(ModulateCompositeOp)
        ENUMERATOR(MultiplyCompositeOp)
        ENUMERATOR(OutCompositeOp)
        ENUMERATOR(OverCompositeOp)
        ENUMERATOR(OverlayCompositeOp)
        ENUMERATOR(PlusCompositeOp)
#if defined(HAVE_REPLACECOMPOSITEOP)    // Added 5.5.8
        ENUMERATOR(ReplaceCompositeOp)    // synonym for CopyCompositeOp
#endif
        ENUMERATOR(SaturateCompositeOp)
        ENUMERATOR(ScreenCompositeOp)
#if defined(HAVE_DSTCOMPOSITEOP)
        ENUMERATOR(SrcAtopCompositeOp)
        ENUMERATOR(SrcCompositeOp)
        ENUMERATOR(SrcInCompositeOp)
        ENUMERATOR(SrcOutCompositeOp)
        ENUMERATOR(SrcOverCompositeOp)
#endif
        ENUMERATOR(SubtractCompositeOp)
        ENUMERATOR(ThresholdCompositeOp)
        ENUMERATOR(XorCompositeOp)

#if defined(HAVE_COLORDODGECOMPOSITEOP)
        ENUMERATOR(BlendCompositeOp)
        ENUMERATOR(ColorBurnCompositeOp)
        ENUMERATOR(ColorDodgeCompositeOp)
        ENUMERATOR(ExclusionCompositeOp)
        ENUMERATOR(HardLightCompositeOp)
        ENUMERATOR(SoftLightCompositeOp)
#endif
    END_ENUM

    // CompressionType constants
    DEF_ENUM(CompressionType)
        ENUMERATOR(UndefinedCompression)
        ENUMERATOR(NoCompression)
        ENUMERATOR(BZipCompression)
        ENUMERATOR(FaxCompression)
        ENUMERATOR(Group4Compression)
        ENUMERATOR(JPEGCompression)
#if defined(HAVE_JPEG2000COMPRESSION)
        ENUMERATOR(JPEG2000Compression)
#endif
        ENUMERATOR(LosslessJPEGCompression)
        ENUMERATOR(LZWCompression)
        ENUMERATOR(RLECompression)              // preferred
        ENUMERATOR(RunlengthEncodedCompression) // deprecated
        ENUMERATOR(ZipCompression)
    END_ENUM

    // DecorationType constants
    DEF_ENUM(DecorationType)
        ENUMERATOR(NoDecoration)
        ENUMERATOR(UnderlineDecoration)
        ENUMERATOR(OverlineDecoration)
        ENUMERATOR(LineThroughDecoration)
    END_ENUM

#if defined(HAVE_DISPOSETYPE)
    // DisposeType constants (5.5.1)
    DEF_ENUM(DisposeType)
        ENUMERATOR(UndefinedDispose)
        ENUMERATOR(BackgroundDispose)
        ENUMERATOR(NoneDispose)
        ENUMERATOR(PreviousDispose)
    END_ENUM
#endif

    DEF_ENUM(EndianType)
        ENUMERATOR(UndefinedEndian)
        ENUMERATOR(LSBEndian)
        ENUMERATOR(MSBEndian)
    END_ENUM

    // FilterTypes constants
    DEF_ENUM(FilterTypes)
        ENUMERATOR(UndefinedFilter)
        ENUMERATOR(PointFilter)
        ENUMERATOR(BoxFilter)
        ENUMERATOR(TriangleFilter)
        ENUMERATOR(HermiteFilter)
        ENUMERATOR(HanningFilter)
        ENUMERATOR(HammingFilter)
        ENUMERATOR(BlackmanFilter)
        ENUMERATOR(GaussianFilter)
        ENUMERATOR(QuadraticFilter)
        ENUMERATOR(CubicFilter)
        ENUMERATOR(CatromFilter)
        ENUMERATOR(MitchellFilter)
        ENUMERATOR(LanczosFilter)
        ENUMERATOR(BesselFilter)
        ENUMERATOR(SincFilter)
    END_ENUM

    // GravityType constants
    DEF_ENUM(GravityType)
#if defined(HAVE_UNDEFINEDGRAVITY)
        ENUMERATOR(UndefinedGravity)
#else
        // Provide this enumerator in older (pre 6.0.0) versions of ImageMagick
        _enum = rm_enum_new(_cls, ID2SYM(rb_intern("UndefinedGravity")), INT2FIX(0));\
        rb_define_const(Module_Magick, "UndefinedGravity", _enum);
#endif
        ENUMERATOR(ForgetGravity)
        ENUMERATOR(NorthWestGravity)
        ENUMERATOR(NorthGravity)
        ENUMERATOR(NorthEastGravity)
        ENUMERATOR(WestGravity)
        ENUMERATOR(CenterGravity)
        ENUMERATOR(EastGravity)
        ENUMERATOR(SouthWestGravity)
        ENUMERATOR(SouthGravity)
        ENUMERATOR(SouthEastGravity)
        ENUMERATOR(StaticGravity)
    END_ENUM

    // ImageType constants
    DEF_ENUM(ImageType)
        ENUMERATOR(UndefinedType)
        ENUMERATOR(BilevelType)
        ENUMERATOR(GrayscaleType)
        ENUMERATOR(GrayscaleMatteType)
        ENUMERATOR(PaletteType)
        ENUMERATOR(PaletteMatteType)
        ENUMERATOR(TrueColorType)
        ENUMERATOR(TrueColorMatteType)
        ENUMERATOR(ColorSeparationType)
        ENUMERATOR(ColorSeparationMatteType)
        ENUMERATOR(OptimizeType)
    END_ENUM

    // InterlaceType constants
    DEF_ENUM(InterlaceType)
        ENUMERATOR(UndefinedInterlace)
        ENUMERATOR(NoInterlace)
        ENUMERATOR(LineInterlace)
        ENUMERATOR(PlaneInterlace)
        ENUMERATOR(PartitionInterlace)
    END_ENUM

#if defined(HAVE_COMPAREIMAGECHANNELS)
    DEF_ENUM(MetricType)
        ENUMERATOR(UndefinedMetric)
        ENUMERATOR(MeanAbsoluteErrorMetric)
        ENUMERATOR(MeanSquaredErrorMetric)
        ENUMERATOR(PeakAbsoluteErrorMetric)
        ENUMERATOR(PeakSignalToNoiseRatioMetric)
        ENUMERATOR(RootMeanSquaredErrorMetric)
    END_ENUM
#endif

    // NoiseType constants
    DEF_ENUM(NoiseType)
        ENUMERATOR(UniformNoise)
        ENUMERATOR(GaussianNoise)
        ENUMERATOR(MultiplicativeGaussianNoise)
        ENUMERATOR(ImpulseNoise)
        ENUMERATOR(LaplacianNoise)
        ENUMERATOR(PoissonNoise)
    END_ENUM

#if defined(HAVE_IMAGE_ORIENTATION)
    // Orientation constants
    DEF_ENUM(OrientationType)
        ENUMERATOR(UndefinedOrientation)
        ENUMERATOR(TopLeftOrientation)
        ENUMERATOR(TopRightOrientation)
        ENUMERATOR(BottomRightOrientation)
        ENUMERATOR(BottomLeftOrientation)
        ENUMERATOR(LeftTopOrientation)
        ENUMERATOR(RightTopOrientation)
        ENUMERATOR(RightBottomOrientation)
        ENUMERATOR(LeftBottomOrientation)
    END_ENUM
#endif

    // Paint method constants
    DEF_ENUM(PaintMethod)
        ENUMERATOR(PointMethod)
        ENUMERATOR(ReplaceMethod)
        ENUMERATOR(FloodfillMethod)
        ENUMERATOR(FillToBorderMethod)
        ENUMERATOR(ResetMethod)
    END_ENUM

    // PreviewType
    DEF_ENUM(PreviewType)
        ENUMERATOR(UndefinedPreview)
        ENUMERATOR(RotatePreview)
        ENUMERATOR(ShearPreview)
        ENUMERATOR(RollPreview)
        ENUMERATOR(HuePreview)
        ENUMERATOR(SaturationPreview)
        ENUMERATOR(BrightnessPreview)
        ENUMERATOR(GammaPreview)
        ENUMERATOR(SpiffPreview)
        ENUMERATOR(DullPreview)
        ENUMERATOR(GrayscalePreview)
        ENUMERATOR(QuantizePreview)
        ENUMERATOR(DespecklePreview)
        ENUMERATOR(ReduceNoisePreview)
        ENUMERATOR(AddNoisePreview)
        ENUMERATOR(SharpenPreview)
        ENUMERATOR(BlurPreview)
        ENUMERATOR(ThresholdPreview)
        ENUMERATOR(EdgeDetectPreview)
        ENUMERATOR(SpreadPreview)
        ENUMERATOR(SolarizePreview)
        ENUMERATOR(ShadePreview)
        ENUMERATOR(RaisePreview)
        ENUMERATOR(SegmentPreview)
        ENUMERATOR(SwirlPreview)
        ENUMERATOR(ImplodePreview)
        ENUMERATOR(WavePreview)
        ENUMERATOR(OilPaintPreview)
        ENUMERATOR(CharcoalDrawingPreview)
        ENUMERATOR(JPEGPreview)
    END_ENUM

#if defined(HAVE_QUANTUMOPERATORREGIONIMAGE) || defined(HAVE_EVALUATEIMAGECHANNEL)
    DEF_ENUM(QuantumExpressionOperator)
        ENUMERATOR(UndefinedQuantumOperator)
        ENUMERATOR(AddQuantumOperator)
        ENUMERATOR(AndQuantumOperator)
        ENUMERATOR(DivideQuantumOperator)
        ENUMERATOR(LShiftQuantumOperator)
        ENUMERATOR(MultiplyQuantumOperator)
        ENUMERATOR(OrQuantumOperator)
        ENUMERATOR(RShiftQuantumOperator)
        ENUMERATOR(SubtractQuantumOperator)
        ENUMERATOR(XorQuantumOperator)
    END_ENUM
#endif

    // RenderingIntent
    DEF_ENUM(RenderingIntent)
        ENUMERATOR(UndefinedIntent)
        ENUMERATOR(SaturationIntent)
        ENUMERATOR(PerceptualIntent)
        ENUMERATOR(AbsoluteIntent)
        ENUMERATOR(RelativeIntent)
    END_ENUM

    // ResolutionType constants
    DEF_ENUM(ResolutionType)
        ENUMERATOR(UndefinedResolution)
        ENUMERATOR(PixelsPerInchResolution)
        ENUMERATOR(PixelsPerCentimeterResolution)
    END_ENUM

    // StorageType
    DEF_ENUM(StorageType)
#if defined(HAVE_UNDEFINEDGRAVITY)      // UndefinedGravity & UndefinedPixel were both introduced in IM 6.0.0
        ENUMERATOR(UndefinedPixel)
#endif
        ENUMERATOR(CharPixel)
        ENUMERATOR(DoublePixel)
        ENUMERATOR(FloatPixel)
        ENUMERATOR(IntegerPixel)
        ENUMERATOR(LongPixel)
#if defined(HAVE_QUANTUMPIXEL)
        ENUMERATOR(QuantumPixel)
#endif
        ENUMERATOR(ShortPixel)
    END_ENUM

    // StretchType constants
    DEF_ENUM(StretchType)
        ENUMERATOR(NormalStretch)
        ENUMERATOR(UltraCondensedStretch)
        ENUMERATOR(ExtraCondensedStretch)
        ENUMERATOR(CondensedStretch)
        ENUMERATOR(SemiCondensedStretch)
        ENUMERATOR(SemiExpandedStretch)
        ENUMERATOR(ExpandedStretch)
        ENUMERATOR(ExtraExpandedStretch)
        ENUMERATOR(UltraExpandedStretch)
        ENUMERATOR(AnyStretch)
    END_ENUM

    // StyleType constants
    DEF_ENUM(StyleType)
        ENUMERATOR(NormalStyle)
        ENUMERATOR(ItalicStyle)
        ENUMERATOR(ObliqueStyle)
        ENUMERATOR(AnyStyle)
    END_ENUM

    // VirtualPixelMethod
    DEF_ENUM(VirtualPixelMethod)
        ENUMERATOR(UndefinedVirtualPixelMethod)
        ENUMERATOR(EdgeVirtualPixelMethod)
        ENUMERATOR(MirrorVirtualPixelMethod)
        ENUMERATOR(TileVirtualPixelMethod)
#if defined(HAVE_TRANSPARENTVIRTUALPIXELMETHOD)
        ENUMERATOR(BackgroundVirtualPixelMethod)
        ENUMERATOR(TransparentVirtualPixelMethod)
#endif
    END_ENUM

    // WeightType constants
    DEF_ENUM(WeightType)
        ENUMERATOR(AnyWeight)
        ENUMERATOR(NormalWeight)
        ENUMERATOR(BoldWeight)
        ENUMERATOR(BolderWeight)
        ENUMERATOR(LighterWeight)
    END_ENUM

    /*-----------------------------------------------------------------------*/
    /* Struct classes                                                        */
    /*-----------------------------------------------------------------------*/

    // Pass NULL as the structure name to keep them from polluting the Struct
    // namespace. The only way to use these classes is via the Magick:: namespace.

    // Magick::AffineMatrix
    Class_AffineMatrix = rb_struct_define(NULL, "sx", "rx", "ry", "sy", "tx", "ty", NULL);
    rb_define_const(Module_Magick, "AffineMatrix", Class_AffineMatrix);

#if defined(HAVE_GETIMAGESTATISTICS)
    // These classes are defined for >= GM 1.1.

    // Magick::Statistics
    Class_Statistics = rb_struct_define(NULL, "red", "green", "blue", "opacity", NULL);
    rb_define_const(Module_Magick, "Statistics", Class_Statistics);
    // Magick::ChannelStatistics
    Class_StatisticsChannel = rb_struct_define(NULL, "max", "min", "mean", "stddev", "var", NULL);
    rb_define_const(Class_Statistics, "Channel", Class_StatisticsChannel);
#endif


    // Magick::Primary
    Class_Primary = rb_struct_define(NULL, "x", "y", "z", NULL);
    rb_define_method(Class_Primary, "to_s", PrimaryInfo_to_s, 0);
    rb_define_const(Module_Magick, "Primary", Class_Primary);

    // Magick::Chromaticity
    Class_Chromaticity = rb_struct_define(NULL
                                            , "red_primary"
                                            , "green_primary"
                                            , "blue_primary"
                                            , "white_point"
                                            , NULL);
    rb_define_method(Class_Chromaticity, "to_s", ChromaticityInfo_to_s, 0);
    rb_define_const(Module_Magick, "Chromaticity", Class_Chromaticity);

    // Magick::Color
    Class_Color = rb_struct_define(NULL, "name", "compliance", "color", NULL);
    rb_define_method(Class_Color, "to_s", Color_to_s, 0);
    rb_define_const(Module_Magick, "Color", Class_Color);

    // Magick::Point
    Class_Point = rb_struct_define(NULL, "x", "y", NULL);
    rb_define_const(Module_Magick, "Point", Class_Point);

    // Magick::Rectangle
    Class_Rectangle = rb_struct_define(NULL, "width", "height", "x", "y", NULL);
    rb_define_method(Class_Rectangle, "to_s", RectangleInfo_to_s, 0);
    rb_define_const(Module_Magick, "Rectangle", Class_Rectangle);

    // Magick::Segment
    Class_Segment = rb_struct_define(NULL, "x1", "y1", "x2", "y2", NULL);
    rb_define_method(Class_Segment, "to_s", SegmentInfo_to_s, 0);
    rb_define_const(Module_Magick, "Segment", Class_Segment);

    // Magick::Font
    Class_Font = rb_struct_define(NULL, "name", "description",
                                      "family", "style", "stretch", "weight",
                                      "encoding", "foundry", "format", NULL);
    rb_define_method(Class_Font, "to_s", Font_to_s, 0);
    rb_define_const(Module_Magick, "Font", Class_Font);

    // Magick::TypeMetric
    Class_TypeMetric = rb_struct_define(NULL, "pixels_per_em", "ascent", "descent",
                                        "width", "height", "max_advance", "bounds",
                                        "underline_position", "underline_thickness", NULL);
    rb_define_method(Class_TypeMetric, "to_s", TypeMetric_to_s, 0);
    rb_define_const(Module_Magick, "TypeMetric", Class_TypeMetric);


}

/*
    Static:     version_constants
    Purpose:    create Version, Magick_version, and Version_long constants.
*/
static void version_constants(void)
{
    const char *mgk_version;
    volatile VALUE str;
    char long_version[1000];

    mgk_version = GetMagickVersion(NULL);

    str = rb_str_new2(mgk_version);
    rb_obj_freeze(str);
    rb_define_const(Module_Magick, "Magick_version", str);

    str = rb_str_new2(PACKAGE_STRING);
    rb_obj_freeze(str);
    rb_define_const(Module_Magick, "Version", str);

    sprintf(long_version,
        "This is %s ($Date: 2005/12/31 14:35:59 $) Copyright (C) 2005 by Timothy P. Hunter\n"
        "Built with %s\n"
        "Built for %s\n"
        "Web page: http://rmagick.rubyforge.org\n"
        "Email: rmagick@rubyforge.org\n",
        PACKAGE_STRING, mgk_version, RUBY_VERSION_STRING);

    str = rb_str_new2(long_version);
    rb_obj_freeze(str);
    rb_define_const(Module_Magick, "Long_version", str);

}
