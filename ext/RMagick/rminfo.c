/* $Id: rminfo.c,v 1.8 2003/09/18 13:21:13 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2003 by Timothy P. Hunter
| Name:     rminfo.c
| Author:   Tim Hunter
| Purpose:  Info class method definitions for RMagick.
\============================================================================*/

#include "rmagick.h"

DEF_ATTR_ACCESSOR(Info, antialias, bool)

/*
    Method:     Info#background_color
    Purpose:    return the name of the background color as a String
    Note:       Compare with Image#background_color!
*/
VALUE
Info_background_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return PixelPacket_to_Color_Name_Info(info, &info->background_color);
}

/*
    Method:     Info#background_color=<aString>
    Purpose:    set the background color
    Raises:     ArgumentError
*/
VALUE
Info_background_color_eq(VALUE self, VALUE bc_arg)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->background_color, bc_arg);
    return self;
}

/*
    Method:     Info#border_color
    Purpose:    return the name of the border color as a String
    Note:       Compare with Image#border_color!
*/
VALUE
Info_border_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return PixelPacket_to_Color_Name_Info(info, &info->border_color);
}

/*
    Method:     Info#border_color=<aString>
    Purpose:    set the border color
    Raises:     ArgumentError
*/
VALUE
Info_border_color_eq(VALUE self, VALUE bc_arg)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->border_color, bc_arg);
    return self;
}

DEF_ATTR_READER(Info, colorspace, int)

/*
    Method:     Info#colorspace=
    Purpose:    Set the colorspace type
    Raises:     ArgumentError
*/
VALUE
Info_colorspace_eq(VALUE self, VALUE colorspace)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    NUM_TO_ENUM(colorspace, info->colorspace, ColorspaceType);
    return self;
}

DEF_ATTR_READER(Info, compression, int)

/*
    Method:     Info#compression=
    Purpose:    Set the compression type
    Raises:     ArgumentError
*/
VALUE
Info_compression_eq(VALUE self, VALUE type)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    NUM_TO_ENUM(type, info->compression, CompressionType);
    return self;
}

DEF_ATTR_READER(Info, density, str)

/*
    Method:     Info#density=<aString>
    Purpose:    Set the text rendering density, e.g. "72x72"
    Raise:      ArgumentError
*/
VALUE
Info_density_eq(VALUE self, VALUE density)
{
    Info *info;
    char *dens;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(density) || STRING_PTR(density) == NULL)
    {
        magick_free(info->density);
        info->density = NULL;
        return self;
    }

    dens = STRING_PTR(density);
    if (!IsGeometry(dens))
    {
        rb_raise(rb_eArgError, "invalid density geometry: %s", dens);
    }
    magick_clone_string(&info->density, dens);

    return self;
}

DEF_ATTR_READER(Info, depth, int)

/*
    Method:     Info#depth=
    Purpose:    Set the depth (8, 16, 32).
    Raises:     ArgumentError
*/
VALUE
Info_depth_eq(VALUE self, VALUE depth)
{
    Info *info;
    int d;

    Data_Get_Struct(self, Info, info);
    d = NUM2INT(depth);
    switch (d)
    {
        case 8:                     // always okay
#if QuantumDepth == 16 || QuantumDepth == 32
        case 16:
#endif
#if QuantumDepth == 32
        case 32:
#endif
            break;
        default:
            rb_raise(rb_eArgError, "invalid depth (%d)", d);
            break;
    }

    info->depth = d;
    return self;
}

DEF_ATTR_ACCESSOR(Info, dither, bool)

#ifdef HAVE_IMAGE_EXTRACT_INFO

/*
    Method:     aString=Info#extract
                Info#extract=aString
    Purpose:    Get/set the extract string, e.g. "200x200+100+100"
    Raise:      ArgumentError
    Notes:      defined for IM 5.5.6 and later
*/
DEF_ATTR_READER(Info, extract, str)

VALUE
Info_extract_eq(VALUE self, VALUE extract)
{
    Info *info;
    char *extr;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(extract) || STRING_PTR(extract) == NULL)
    {
        magick_free(info->extract);
        info->extract = NULL;
        return self;
    }

    extr = STRING_PTR(extract);
    if (!IsGeometry(extr))
    {
        rb_raise(rb_eArgError, "invalid extract geometry: %s", extr);
    }
    magick_clone_string(&info->extract, extr);

    return self;
}
/*
    Method:     aString=Info#tile
                Info#tile=aString
    Purpose:    Get/set the "tile" string, e.g. "200x200+100+100"
    Raise:      ArgumentError
    Notes:      defined for IM 5.5.6 and later. Actually these are effectively
                aliases for extract & extract= but with warning messages.
*/

VALUE
Info_tile(VALUE self)
{
    rb_warning("RMagick: tile is deprecated in this release of ImageMagick. Use extract instead.");
    return Info_extract(self);
}

VALUE
Info_tile_eq(VALUE self, VALUE tile)
{
    rb_warning("RMagick: tile= is deprecated in this release of ImageMagick. Use extract= instead.");
    return Info_extract_eq(self, tile);
}

#else

/*
    Method:     aString=Info#extract
                Info#extract=aString
    Purpose:    Get/set the extract string, e.g. "200x200+100+100"
    Raise:      ArgumentError
    Notes:      defined for IM 5.5.6 and later
*/
VALUE
Info_extract(VALUE self)
{
    not_implemented("extract");
    return (VALUE)0;
}
VALUE
Info_extract_eq(VALUE self, VALUE extr)
{
    not_implemented("extract=");
    return (VALUE)0;
}

/*
    Method:     aString = Info#tile
                Info#tile=aString
    Purpose:    Get/set the tile string, e.g. "200x200+100+100"
    Raise:      ArgumentError
    Notes:      defined for IM 5.5.5 and earlier, before the tile field was
                deprecated and replaced by extract
*/
DEF_ATTR_READER(Info, tile, str)

VALUE
Info_tile_eq(VALUE self, VALUE tile)
{
    Info *info;
    char *til;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(tile) || STRING_PTR(tile) == NULL)
    {
        magick_free(info->tile);
        info->tile = NULL;
        return self;
    }

    til = STRING_PTR(tile);
    if (!IsGeometry(til))
    {
        rb_raise(rb_eArgError, "invalid tile geometry: %s", til);
    }
    magick_clone_string(&info->tile, til);

    return self;
}
#endif

/*
    Methods:    aString=Info#filename
                Info#filename=aString
    Purpose:    Get/set the "filename"
    Notes:      Only used for Image#capture
                Returns "" if filename not set
*/
VALUE
Info_filename(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return rb_str_new2(info->filename);
}

VALUE
Info_filename_eq(VALUE self, VALUE filename)
{
    Info *info;
    char *fname;

    Data_Get_Struct(self, Info, info);

    // Allow "nil" - remove current filename
    if (NIL_P(filename) || STRING_PTR(filename) == NULL)
    {
        info->filename[0] = '\0';
    }
    else
    {
        // Otherwise copy in filename
        fname = STRING_PTR(filename);
        strncpy(info->filename, fname, MaxTextExtent);
    }
    return self;
}

/*
    Methods:    aString=Info#font
                Info#font=aString
    Purpose:    Get/set the text font
*/
DEF_ATTR_READER(Info, font, str)

VALUE
Info_font_eq(VALUE self, VALUE font_arg)
{
    Info *info;
    char *font;

    Data_Get_Struct(self, Info, info);
    if (NIL_P(font_arg) || STRING_PTR(font_arg) == NULL)
    {
        magick_free(info->font);
        info->font = NULL;
    }
    else
    {
        font = STRING_PTR(font_arg);
        magick_clone_string(&info->font, font);
    }
    return self;
}

/*
    Method:     Info#format
    Purpose:    Return the image encoding format
*/
VALUE Info_format(VALUE self)
{
    Info *info;
    const MagickInfo *magick_info ;
    ExceptionInfo exception;

    Data_Get_Struct(self, Info, info);
    if (*info->magick)
    {
        GetExceptionInfo(&exception);
        magick_info = GetMagickInfo(info->magick, &exception);
        return magick_info ? rb_str_new2(magick_info->name) : Qnil;
    }

    return Qnil;
}

/*
    Method:     Info#format=
    Purpose:    Set the image encoding format
*/
VALUE
Info_format_eq(VALUE self, VALUE magick)
{
    Info *info;
    const MagickInfo *m;
    char *mgk;
    ExceptionInfo exception;

    Data_Get_Struct(self, Info, info);

    GetExceptionInfo(&exception);

    mgk = STRING_PTR(magick);
    m = GetMagickInfo(mgk, &exception);
    HANDLE_ERROR

    if (!m)
    {
        rb_raise(rb_eArgError, "unknown format: %s", mgk);
    }

    strncpy(info->magick, m->name, MaxTextExtent-1);
    return self;
}

DEF_ATTR_ACCESSOR(Info, fuzz, dbl)
DEF_ATTR_ACCESSOR(Info, group, long)

/*
    Method:     Info#image_type
                Info#image_type=
    Purpose:    Set the classification type
    Raises:     ArgumentError
*/
DEF_ATTR_READERF(Info, image_type, type, int)

VALUE
Info_image_type_eq(VALUE self, VALUE type)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    NUM_TO_ENUM(type, info->type, ImageType);
    return self;
}

DEF_ATTR_READER(Info, interlace, int)

/*
    Method:     Info#interlace=
    Purpose:    Set the interlace type
    Raises:     ArgumentError
*/
VALUE
Info_interlace_eq(VALUE self, VALUE inter)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    NUM_TO_ENUM(inter, info->interlace, InterlaceType);
    return self;
}

/*
    Method:     Info#matte_color
    Purpose:    return the name of the matte color as a String
    Note:       Compare with Image#matte_color!
*/
VALUE
Info_matte_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return PixelPacket_to_Color_Name_Info(info, &info->matte_color);
}

/*
    Method:     Info#matte_color=<aString>
    Purpose:    set the matte color
    Raises:     ArgumentError
*/
VALUE
Info_matte_color_eq(VALUE self, VALUE matte_arg)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->matte_color, matte_arg);
    return self;
}

DEF_ATTR_ACCESSOR(Info, monochrome, bool)

#ifdef HAVE_IMAGEINFO_NUMBER_SCENES
DEF_ATTR_ACCESSOR(Info, number_scenes, ulong)
#else

/*
    Methods:    num = Info#number_scenes
                Info#number_scenes = num
    Purpose:    alias for subrange when IM < 5.5.6
*/
VALUE
Info_number_scenes(VALUE self)
{
    return Info_subrange(self);
}

VALUE
Info_number_scenes_eq(VALUE self, VALUE nscenes)
{
    return Info_subrange_eq(self, nscenes);
}
#endif

DEF_ATTR_READER(Info, page, str)

/*
    Method:     Info#page=<aString>
    Purpose:    store the Postscript page geometry
*/
VALUE
Info_page_eq(VALUE self, VALUE page_arg)
{
    Info *info;
    char *geometry;

    Data_Get_Struct(self, Info, info);
    if (NIL_P(page_arg) || STRING_PTR(page_arg) == NULL)
    {
        magick_free(info->page);
        info->page = NULL;
        return self;
    }

    geometry=PostscriptGeometry(STRING_PTR(page_arg));
    magick_clone_string(&info->page, geometry);

    return self;
}

DEF_ATTR_ACCESSOR(Info, quality, long)

#ifdef HAVE_IMAGEINFO_NUMBER_SCENES

// Info#scene, scene= is the IM >= 5.5.6 version of the now-deprecated
// subimage accessors.
DEF_ATTR_ACCESSOR(Info, scene, ulong)

/*
    Methods:    num=Info#subimage
                Info#subimage=num
    Purpose:    Get/set the "subimage" value, for IM >= 5.5.6
    Raises:     ArgumentError
    Notes:      synonyms for Info#scene, Info#scene=
*/
VALUE
Info_subimage(VALUE self)
{
    rb_warning("RMagick: subimage is deprecated in this release of ImageMagick. Use scene instead.");
    return Info_scene(self);
}

VALUE
Info_subimage_eq(VALUE self, VALUE subimage)
{
    rb_warning("RMagick: subimage= is deprecated in this release of ImageMagick. Use scene= instead.");
    return Info_scene_eq(self, subimage);
}

/*
    Methods:    num=Info#subrange
                Info#subrange=num
    Purpose:    Get/set the "subrange" value, for IM >= 5.5.6
    Raises:     ArgumentError
    Notes:      synonyms for Info#number_scenes, Info#number_scenes=
*/
VALUE
Info_subrange(VALUE self)
{
    rb_warning("RMagick: subrange is deprecated in this release of ImageMagick. Use number_scenes instead.");
    return Info_number_scenes(self);
}

VALUE
Info_subrange_eq(VALUE self, VALUE subrange)
{
    rb_warning("RMagick: subrange= is deprecated in this release of ImageMagick. Use number_scenes= instead.");
    return Info_number_scenes_eq(self, subrange);
}

#else

/*
    Methods:    num=Info#scene
                Info#scene=num
    Purpose:    Get/set the scene number, for IM < 5.5.6
    Raises:     ArgumentError
    Notes:      synonyms for Info#subimage, Info#subimage=
*/
VALUE
Info_scene(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return UINT2NUM(info->subimage);
}

VALUE
Info_scene_eq(VALUE self, VALUE scene)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    info->subimage = NUM2ULONG(scene);
    return self;
}

DEF_ATTR_ACCESSOR(Info, subimage, ulong)
DEF_ATTR_ACCESSOR(Info, subrange, ulong)

#endif

DEF_ATTR_READER(Info, server_name, str)

/*
    Method:     Info#server_name=<aString>
    Purpose:    Set the server name
*/
VALUE
Info_server_name_eq(VALUE self, VALUE server_arg)
{
    Info *info;
    char *server;

    Data_Get_Struct(self, Info, info);
    if (NIL_P(server_arg) || STRING_PTR(server_arg) == NULL)
    {
        magick_free(info->server_name);
        info->server_name = NULL;
    }
    else
    {
        server = STRING_PTR(server_arg);
        magick_clone_string(&info->server_name, server);
    }
    return self;
}

DEF_ATTR_READER(Info, size, str)

/*
    Method:     Info#size=<aString>
    Purpose:    Set the size (a Geometry string, i.e. WxH{+-}x{+-}y
    Raises:     ArgumentError
*/
VALUE
Info_size_eq(VALUE self, VALUE size_arg)
{
    Info *info;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(size_arg) || STRING_PTR(size_arg) == NULL)
    {
        magick_free(info->size);
        info->size = NULL;
        return self;
    }

    if (!IsGeometry(STRING_PTR(size_arg)))
    {
        rb_raise(rb_eArgError, "invalid size geometry");
    }
    magick_clone_string(&info->size, STRING_PTR(size_arg));

    return self;
}

DEF_ATTR_READER(Info, units, int)

/*
    Method:     Info#units=
    Purpose:    Set the resolution type
    Raises:     ArgumentError
*/
VALUE
Info_units_eq(VALUE self, VALUE units)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    NUM_TO_ENUM(units, info->units, ResolutionType);
    return self;
}

DEF_ATTR_READER(Info, view, str)

/*
    Method:     Info#view=
    Purpose:    Set FlashPix viewing parameters
*/
VALUE
Info_view_eq(VALUE self, VALUE view_arg)
{
    Info *info;
    char *view;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(view_arg) || STRING_PTR(view_arg) == NULL)
    {
       magick_free(info->view);
       info->view = NULL;
    }
    else
    {
        view = STRING_PTR(view_arg);
        magick_clone_string(&info->view, view);
    }
    return self;
}

//DEF_ATTR_ACCESSOR(Info, verbose, bool)


/*
    Method:     Info.new
    Purpose:    Create an Info object by calling CloneInfo
*/
#if RUBY_VERSION < 0x180
VALUE
Info_new(VALUE class)
{
    Info *info;
    volatile VALUE new_obj;

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Info object");
    }
    new_obj = Data_Wrap_Struct(class, NULL, DestroyImageInfo, info);
    rb_obj_call_init(new_obj, 0, NULL);
    return new_obj;
}

/*
    Extern:     rm_info_new
    Purpose:    provide a Info.new method for internal use
    Notes:      takes no parameters, but runs the parm block if present
*/
VALUE
rm_info_new()
{
    return Info_new(Class_Info);
}
#else

/*
    Extern:     Info_alloc
    Purpose:    Create an ImageInfo object
*/
VALUE
Info_alloc(VALUE class)
{
    Info *info;
    volatile VALUE info_obj;

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Info object");
    }
    info_obj = Data_Wrap_Struct(class, NULL, DestroyImageInfo, info);
    return info_obj;
}
/*
    Extern:     rm_info_new
    Purpose:    provide a Info.new method for internal use
    Notes:      takes no parameters, but runs the parm block if present
*/
VALUE
rm_info_new()
{
    volatile VALUE info_obj;

    info_obj = Info_alloc(Class_Info);
    return Info_initialize(info_obj);
}
#endif

/*
    Method:     Info#initialize
    Purpose:    If an initializer block is present, run it.
*/
VALUE
Info_initialize(VALUE self)
{
    if (rb_block_given_p())
    {
        // Run the block in self's context
        rb_obj_instance_eval(0, NULL, self);
    }
    return self;
}
