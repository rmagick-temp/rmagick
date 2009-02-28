/* $Id: rmmontage.c,v 1.4 2009/02/28 23:50:36 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2009 by Timothy P. Hunter
| Name:     rmdraw.c
| Author:   Tim Hunter
| Purpose:  Contains Montage class methods.
\============================================================================*/

#include "rmagick.h"





/*
    Static:     destroy_Montage
    Purpose:    destory the MontageInfo struct and free the Montage struct
    Notes:      if the Magick::Montage#texture method wrote a texture file,
                the file is deleted here.
*/
static void
destroy_Montage(void *obj)
{
    Montage *montage = obj;

    // If we saved a temporary texture image, delete it now.
    if (montage->info && montage->info->texture != NULL)
    {
        rm_delete_temp_image(montage->info->texture);
        magick_free(montage->info->texture);
        montage->info->texture = NULL;
    }
    if (montage->info)
    {
        (void) DestroyMontageInfo(montage->info);
        montage->info = NULL;
    }
    xfree(montage);
}


/*
    Method:     Montage.new
    Purpose:    Create a new Montage object
*/
VALUE
Montage_alloc(VALUE class)
{
    MontageInfo *montage_info;
    Montage *montage;
    Info *image_info;
    volatile VALUE montage_obj;

    // DO NOT call rm_info_new - we don't want to support an Info parm block.
    image_info = CloneImageInfo(NULL);
    if (!image_info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Info object");
    }

    montage_info = CloneMontageInfo(image_info, NULL);
    (void) (void) DestroyImageInfo(image_info);

    if (!montage_info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Magick::Montage object");
    }

    montage = ALLOC(Montage);
    montage->info = montage_info;
    montage->compose = OverCompositeOp;
    montage_obj = Data_Wrap_Struct(class, NULL, destroy_Montage, montage);

    return montage_obj;
}


/*
    Method:     Magick::Montage#background_color(color-name)
    Purpose:    set background_color value
*/
VALUE
Montage_background_color_eq(VALUE self, VALUE color)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    Color_to_PixelPacket(&montage->info->background_color, color);
    return self;
}


/*
    Method:     Magick::Montage#border_color(color-name)
    Purpose:    set border_color value
*/
VALUE
Montage_border_color_eq(VALUE self, VALUE color)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    Color_to_PixelPacket(&montage->info->border_color, color);
    return self;
}


/*
    Method:     Magick::Montage#border_width(width)
    Purpose:    set border_width value
*/
VALUE
Montage_border_width_eq(VALUE self, VALUE width)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    montage->info->border_width = NUM2ULONG(width);
    return self;
}


/*
    Method:     Magick::Montage#compose(width)
    Purpose:    set a composition operator
*/
VALUE
Montage_compose_eq(VALUE self, VALUE compose)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    VALUE_TO_ENUM(compose, montage->compose, CompositeOperator);
    return self;
}


/*
    Method:     Magick::Montage#filename(name)
    Purpose:    set filename value
*/
VALUE
Montage_filename_eq(VALUE self, VALUE filename)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    strncpy(montage->info->filename, StringValuePtr(filename), MaxTextExtent-1);
    return self;
}


/*
    Method:     Magick::Montage#fill(color-name)
    Purpose:    set fill value
*/
VALUE
Montage_fill_eq(VALUE self, VALUE color)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    Color_to_PixelPacket(&montage->info->fill, color);
    return self;
}


/*
    Method:     Magick::Montage#font(font-name)
    Purpose:    set font value
*/
VALUE
Montage_font_eq(VALUE self, VALUE font)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    magick_clone_string(&montage->info->font, StringValuePtr(font));

    return self;
}


/*
    Method:     Magick::Montage#frame(frame-geometry)
    Purpose:    set frame value
    Notes:      The geometry is a string in the form:
                <width>x<height>+<outer-bevel-width>+<inner-bevel-width>
                or a Geometry object
*/
VALUE
Montage_frame_eq(VALUE self, VALUE frame_arg)
{
    Montage *montage;
    volatile VALUE frame;

    Data_Get_Struct(self, Montage, montage);
    frame = rm_to_s(frame_arg);
    magick_clone_string(&montage->info->frame, StringValuePtr(frame));

    return self;
}


/*
    Method:     Magick::Montage#geometry(geometry)
    Purpose:    set geometry value
*/
VALUE
Montage_geometry_eq(VALUE self, VALUE geometry_arg)
{
    Montage *montage;
    volatile VALUE geometry;

    Data_Get_Struct(self, Montage, montage);
    geometry = rm_to_s(geometry_arg);
    magick_clone_string(&montage->info->geometry, StringValuePtr(geometry));

    return self;
}


/*
    Method:     Magick::Montage#gravity(gravity-type)
    Purpose:    set gravity value
*/
VALUE
Montage_gravity_eq(VALUE self, VALUE gravity)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    VALUE_TO_ENUM(gravity, montage->info->gravity, GravityType);
    return self;
}


/*
    Method:     Magick::Montage#initialize
    Purpose:    Place-holder
*/
VALUE
Montage_initialize(VALUE self)
{
    // Nothing to do!
    return self;
}


/*
    Method:     Magick::Montage#matte_color(color-name)
    Purpose:    set matte_color value
*/
VALUE
Montage_matte_color_eq(VALUE self, VALUE color)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    Color_to_PixelPacket(&montage->info->matte_color, color);
    return self;
}


/*
    Method:     Magick::Montage#pointsize=size
    Purpose:    set pointsize value
*/
VALUE
Montage_pointsize_eq(VALUE self, VALUE size)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    montage->info->pointsize = NUM2DBL(size);
    return self;
}


/*
    Method:     Magick::Montage#shadow=shadow
    Purpose:    set shadow value
*/
VALUE
Montage_shadow_eq(VALUE self, VALUE shadow)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    montage->info->shadow = (MagickBooleanType) RTEST(shadow);
    return self;
}


/*
    Method:     Magick::Montage#stroke(color-name)
    Purpose:    set stroke value
*/
VALUE
Montage_stroke_eq(VALUE self, VALUE color)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    Color_to_PixelPacket(&montage->info->stroke, color);
    return self;
}


/*
    Method:     Montage#texture(texture-image)
    Purpose:    set texture value
*/
VALUE
Montage_texture_eq(VALUE self, VALUE texture)
{
    Montage *montage;
    Image *texture_image;
    char temp_name[MaxTextExtent];

    Data_Get_Struct(self, Montage, montage);

    // If we had a previously defined temp texture image,
    // remove it now in preparation for this new one.
    if (montage->info->texture)
    {
        rm_delete_temp_image(montage->info->texture);
        magick_free(montage->info->texture);
        montage->info->texture = NULL;
    }

    texture = rm_cur_image(texture);
    texture_image = rm_check_destroyed(texture);

    // Write a temp copy of the image & save its name.
    rm_write_temp_image(texture_image, temp_name);
    magick_clone_string(&montage->info->texture, temp_name);

    return self;
}


/*
    Method:     Magick::Montage#tile(tile)
    Purpose:    set tile value
*/
VALUE
Montage_tile_eq(VALUE self, VALUE tile_arg)
{
    Montage *montage;
    volatile VALUE tile;

    Data_Get_Struct(self, Montage, montage);
    tile = rm_to_s(tile_arg);
    magick_clone_string(&montage->info->tile, StringValuePtr(tile));

    return self;
}


/*
    Method:     Magick::Montage#title(title)
    Purpose:    set title value
*/
VALUE
Montage_title_eq(VALUE self, VALUE title)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    magick_clone_string(&montage->info->title, StringValuePtr(title));
    return self;
}


/*
    Extern:     rm_montage_new()
    Purpose:    Return a new Magick::Montage object
*/

VALUE
rm_montage_new(void)
{
    return Montage_initialize(Montage_alloc(Class_Montage));
}

