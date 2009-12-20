/**************************************************************************//**
 * Contains Draw class methods.
 *
 * Copyright &copy; 2002 - 2009 by Timothy P. Hunter
 *
 * Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or
 *
 * @file     rmdraw.c
 * @version  $Id: rmdraw.c,v 1.83 2009/12/20 02:33:33 baror Exp $
 * @author   Tim Hunter
 ******************************************************************************/

#include "rmagick.h"
#include "float.h"

static void mark_Draw(void *);
static void destroy_Draw(void *);
static VALUE new_DrawOptions(void);

/** Method that gets type metrics */
typedef MagickBooleanType (get_type_metrics_func_t)(Image *, const DrawInfo *, TypeMetric *);
static VALUE get_type_metrics(int, VALUE *, VALUE, get_type_metrics_func_t);


/**
 * Set the affine matrix from an Magick::AffineMatrix.
 *
 * Ruby usage:
 *   - @verbatim Draw#affine= @endverbatim
 *
 * @param self this object
 * @param matrix the affine matrix to set
 * @return self
 */
VALUE
Draw_affine_eq(VALUE self, VALUE matrix)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Export_AffineMatrix(&draw->info->affine, matrix);
    return self;
}


/**
 * Set the text alignment.
 *
 * Ruby usage:
 *   - @verbatim Draw#align= @endverbatim
 *
 * @param self this object
 * @param align the alignment
 * @return self
 */
VALUE
Draw_align_eq(VALUE self, VALUE align)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(align, draw->info->align, AlignType);
    return self;
}


/**
 * Decorate attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#decorate= @endverbatim
 *
 * @param self this object
 * @param decorate the decorate
 * @return self
 */
VALUE
Draw_decorate_eq(VALUE self, VALUE decorate)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(decorate, draw->info->decorate, DecorationType);
    return self;
}


/**
 * Density attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#density= @endverbatim
 *
 * @param self this object
 * @param density the density
 * @return self
 */
VALUE
Draw_density_eq(VALUE self, VALUE density)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->density, StringValuePtr(density));

    return self;
}


/**
 * Encoding attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#encoding= @endverbatim
 *
 * @param self this object
 * @param encoding the encoding
 * @return self
 */
VALUE
Draw_encoding_eq(VALUE self, VALUE encoding)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->encoding, StringValuePtr(encoding));

    return self;
}


/**
 * Fill attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#fill= @endverbatim
 *
 * @param self this object
 * @param fill the fill
 * @return self
 */
VALUE
Draw_fill_eq(VALUE self, VALUE fill)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->fill, fill);
    return self;
}


/**
 * Accept an image as a fill pattern.
 *
 * Ruby usage:
 *   - @verbatim Draw#fill_pattern= @endverbatim
 *
 * @param self this object
 * @param pattern the fill pattern
 * @return self
 * @see Draw_stroke_pattern_eq
 * @see Draw_tile_eq
 */
VALUE
Draw_fill_pattern_eq(VALUE self, VALUE pattern)
{
    Draw *draw;
    Image *image;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    if (draw->info->fill_pattern != NULL)
    {
        // Do not trace destruction
        DestroyImage(draw->info->fill_pattern);
        draw->info->fill_pattern = NULL;
    }

    if (!NIL_P(pattern))
    {
        pattern = rm_cur_image(pattern);
        image = rm_check_destroyed(pattern);
        // Do not trace creation
        draw->info->fill_pattern = rm_clone_image(image);
    }

    return self;
}


/**
 * Font attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#font= @endverbatim
 *
 * @param self this object
 * @param font the font
 * @return self
 */
VALUE
Draw_font_eq(VALUE self, VALUE font)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->font, StringValuePtr(font));

    return self;
}


/**
 * Font family attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#family= @endverbatim
 *
 * @param self this object
 * @param family the family
 * @return self
 */
VALUE
Draw_font_family_eq(VALUE self, VALUE family)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->family, StringValuePtr(family));

    return self;
}


/**
 * Font_stretch attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#font_stretch= @endverbatim
 *
 * @param self this object
 * @param stretch the font_stretch
 * @return self
 */
VALUE
Draw_font_stretch_eq(VALUE self, VALUE stretch)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(stretch, draw->info->stretch, StretchType);
    return self;
}


/**
 * Font_style attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#font_style= @endverbatim
 *
 * @param self this object
 * @param style the font_style
 * @return self
 */
VALUE
Draw_font_style_eq(VALUE self, VALUE style)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(style, draw->info->style, StyleType);
    return self;
}


/**
 * Font_weight attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#font_weight= @endverbatim
 *
 * Notes:
 *   - The font weight can be one of the font weight constants or a number
 *     between 100 and 900
 *
 * @param self this object
 * @param weight the font_weight
 * @return self
 */
VALUE
Draw_font_weight_eq(VALUE self, VALUE weight)
{
    Draw *draw;
    WeightType w;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    if (FIXNUM_P(weight))
    {
        w = (WeightType) FIX2INT(weight);
        if (w < 100 || w > 900)
        {
            rb_raise(rb_eArgError, "invalid font weight (%d given)", w);
        }
        draw->info->weight = w;
    }
    else
    {
        VALUE_TO_ENUM(weight, w, WeightType);
        switch (w)
        {
            case AnyWeight:
                draw->info->weight = 0;
                break;
            case NormalWeight:
                draw->info->weight = 400;
                break;
            case BoldWeight:
                draw->info->weight = 700;
                break;
            case BolderWeight:
                if (draw->info->weight <= 800)
                    draw->info->weight += 100;
                break;
            case LighterWeight:
                if (draw->info->weight >= 100)
                    draw->info->weight -= 100;
                break;
            default:
                rb_raise(rb_eArgError, "unknown font weight");
                break;
        }
    }

    return self;
}


/**
 * Gravity attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#gravity= @endverbatim
 *
 * Notes:
 *   - From Magick++'s Image.h header file:
 *     Gravity affects text placement in bounding area according to rules:
 *     - NorthWestGravity  text bottom-left corner placed at top-left
 *     - NorthGravity      text bottom-center placed at top-center
 *     - NorthEastGravity  text bottom-right corner placed at top-right
 *     - WestGravity       text left-center placed at left-center
 *     - CenterGravity     text center placed at center
 *     - EastGravity       text right-center placed at right-center
 *     - SouthWestGravity  text top-left placed at bottom-left
 *     - SouthGravity      text top-center placed at bottom-center
 *     - SouthEastGravity  text top-right placed at bottom-right
 *
 * @param self this object
 * @param grav the gravity
 * @return self
 */
VALUE
Draw_gravity_eq(VALUE self, VALUE grav)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(grav, draw->info->gravity, GravityType);

    return self;
}


/**
 * Space between two letters.
 *
 * Ruby usage:
 *   - @verbatim Draw#gravity=float @endverbatim
 *
 * Notes:
 *   - New for ImageMagick 6.4.7-8
 *
 * @param self this object
 * @param kerning the kerning
 * @return self
 */
VALUE
Draw_kerning_eq(VALUE self, VALUE kerning)
{
#if defined(HAVE_ST_KERNING)
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->kerning = NUM2DBL(kerning);
    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
    self = self;
    kerning = kerning;
#endif
}


/**
 * Space between two lines.
 *
 * Ruby usage:
 *   - @verbatim Draw#interline_spacing= @endverbatim
 *
 * Notes:
 *   - New for ImageMagick 6.5.5-8
 *
 * @param self this object
 * @param spacing the spacing
 * @return self
 */
VALUE
Draw_interline_spacing_eq(VALUE self, VALUE spacing)
{
#if defined(HAVE_ST_INTERLINE_SPACING)
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->interline_spacing = NUM2DBL(spacing);
    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
    self = self;
    spacing = spacing;
#endif
}


/**
 * Space between two words.
 *
 * Ruby usage:
 *   - @verbatim Draw#interword_spacing= @endverbatim
 *
 * Notes:
 *   - New for ImageMagick 6.4.8-0
 *
 * @param self this object
 * @param spacing the spacing
 * @return self
 */
VALUE
Draw_interword_spacing_eq(VALUE self, VALUE spacing)
{
#if defined(HAVE_ST_INTERWORD_SPACING)
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->interword_spacing = NUM2DBL(spacing);
    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
    self = self;
    spacing = spacing;
#endif
}


/**
 * Convert an image to a blob and the blob to a String.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Returns Qnil if there is no image
 *
 * @param image the Image to convert
 * @return Ruby string representation of image
 * @see str_to_image
 */
static VALUE
image_to_str(Image *image)
{
    volatile VALUE dimg = Qnil;
    unsigned char *blob;
    size_t length;
    Info *info;
    ExceptionInfo exception;

    if (image)
    {
        info = CloneImageInfo(NULL);
        GetExceptionInfo(&exception);
        blob = ImageToBlob(info, image, &length, &exception);
        DestroyImageInfo(info);
        CHECK_EXCEPTION();
        DestroyExceptionInfo(&exception);
        dimg = rb_str_new((char *)blob, (long)length);
        magick_free((void*)blob);
    }

    return dimg;
}


/**
 * Undo the image_to_str, above.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Returns NULL if the argument is Qnil
 *
 * @param str the Ruby string to convert
 * @return Image represented by str
 * @see image_to_str
 */
static
Image *str_to_image(VALUE str)
{
    Image *image = NULL;
    Info *info;
    ExceptionInfo exception;

    if (str != Qnil)
    {
        info = CloneImageInfo(NULL);
        GetExceptionInfo(&exception);
        image = BlobToImage(info, RSTRING_PTR(str), RSTRING_LEN(str), &exception);
        DestroyImageInfo(info);
        CHECK_EXCEPTION();
        DestroyExceptionInfo(&exception);
    }

    return image;
}


/**
 * Custom marshal for Draw objects.
 *
 * Ruby usage:
 *   - @verbatim Draw#marshal_dump @endverbatim
 *
 * Notes:
 *   - Instead of trying to replicate Ruby's support for cross-system
 *     marshalling, exploit it. Convert the Draw fields to Ruby objects and
 *     store them in a hash. Let Ruby marshal the hash.
 *   - Commented out code that dumps/loads fields that are used internally by
 *     ImageMagick and shouldn't be marshaled. I left the code as placeholders
 *     so I'll know which fields have been deliberately omitted.
 *
 * @param self this object
 * @return the marshalled object (as a Ruby hash)
 * @todo Handle gradients when christy gets the new gradient support added (23Dec08)
 */
VALUE
Draw_marshal_dump(VALUE self)
{
    Draw *draw;
    VALUE ddraw;

    Data_Get_Struct(self, Draw, draw);

    // Raise an exception if the Draw has a non-NULL gradient or element_reference field
    if (draw->info->element_reference.type != UndefinedReference
        || draw->info->gradient.type != UndefinedGradient)
    {
        rb_raise(rb_eTypeError, "can't dump gradient definition");
    }

    ddraw = rb_hash_new();

    // rb_hash_aset(ddraw, CSTR2SYM("primitive"), MAGICK_STRING_TO_OBJ(draw->info->primitive)); internal
    // rb_hash_aset(ddraw, CSTR2SYM("geometry"), MAGICK_STRING_TO_OBJ(draw->info->geometry)); set by "text" primitive
    // rb_hash_aset(ddraw, CSTR2SYM("viewbox"), Import_RectangleInfo(&draw->info->viewbox)); internal
    rb_hash_aset(ddraw, CSTR2SYM("affine"), Import_AffineMatrix(&draw->info->affine));
    rb_hash_aset(ddraw, CSTR2SYM("gravity"), INT2FIX(draw->info->gravity));
    rb_hash_aset(ddraw, CSTR2SYM("fill"), Pixel_from_PixelPacket(&draw->info->fill));
    rb_hash_aset(ddraw, CSTR2SYM("stroke"), Pixel_from_PixelPacket(&draw->info->stroke));
    rb_hash_aset(ddraw, CSTR2SYM("stroke_width"), rb_float_new(draw->info->stroke_width));
    // rb_hash_aset(ddraw, CSTR2SYM("gradient"), Qnil);  // not used yet
    rb_hash_aset(ddraw, CSTR2SYM("fill_pattern"), image_to_str(draw->info->fill_pattern));
    rb_hash_aset(ddraw, CSTR2SYM("tile"), Qnil); // deprecated
    rb_hash_aset(ddraw, CSTR2SYM("stroke_pattern"), image_to_str(draw->info->stroke_pattern));
    rb_hash_aset(ddraw, CSTR2SYM("stroke_antialias"), draw->info->stroke_antialias ? Qtrue : Qfalse);
    rb_hash_aset(ddraw, CSTR2SYM("text_antialias"), draw->info->text_antialias ? Qtrue : Qfalse);
    // rb_hash_aset(ddraw, CSTR2SYM("fill_rule"), INT2FIX(draw->info->fill_rule)); internal
    // rb_hash_aset(ddraw, CSTR2SYM("linecap"), INT2FIX(draw->info->linecap));
    // rb_hash_aset(ddraw, CSTR2SYM("linejoin"), INT2FIX(draw->info->linejoin));
    // rb_hash_aset(ddraw, CSTR2SYM("miterlimit"), ULONG2NUM(draw->info->miterlimit));
    // rb_hash_aset(ddraw, CSTR2SYM("dash_offset"), rb_float_new(draw->info->dash_offset));
    rb_hash_aset(ddraw, CSTR2SYM("decorate"), INT2FIX(draw->info->decorate));
    // rb_hash_aset(ddraw, CSTR2SYM("compose"), INT2FIX(draw->info->compose)); set via "image" primitive
    // rb_hash_aset(ddraw, CSTR2SYM("text"), MAGICK_STRING_TO_OBJ(draw->info->text)); set via "text" primitive
    // rb_hash_aset(ddraw, CSTR2SYM("face"), Qnil);  internal
    rb_hash_aset(ddraw, CSTR2SYM("font"), MAGICK_STRING_TO_OBJ(draw->info->font));
    // rb_hash_aset(ddraw, CSTR2SYM("metrics"), Qnil);   internal
    rb_hash_aset(ddraw, CSTR2SYM("family"), MAGICK_STRING_TO_OBJ(draw->info->family));
    rb_hash_aset(ddraw, CSTR2SYM("style"), INT2FIX(draw->info->style));
    rb_hash_aset(ddraw, CSTR2SYM("stretch"), INT2FIX(draw->info->stretch));
    rb_hash_aset(ddraw, CSTR2SYM("weight"), ULONG2NUM(draw->info->weight));
    rb_hash_aset(ddraw, CSTR2SYM("encoding"), MAGICK_STRING_TO_OBJ(draw->info->encoding));
    rb_hash_aset(ddraw, CSTR2SYM("pointsize"), rb_float_new(draw->info->pointsize));
    rb_hash_aset(ddraw, CSTR2SYM("density"), MAGICK_STRING_TO_OBJ(draw->info->density));
    rb_hash_aset(ddraw, CSTR2SYM("align"), INT2FIX(draw->info->align));
    rb_hash_aset(ddraw, CSTR2SYM("undercolor"), Pixel_from_PixelPacket(&draw->info->undercolor));
    // rb_hash_aset(ddraw, CSTR2SYM("border_color"), Pixel_from_PixelPacket(&draw->info->border_color)); Montage and Polaroid
    // rb_hash_aset(ddraw, CSTR2SYM("server_name"), MAGICK_STRING_TO_OBJ(draw->info->server_name));
    // rb_hash_aset(ddraw, CSTR2SYM("dash_pattern"), dash_pattern_to_array(draw->info->dash_pattern)); internal
    // rb_hash_aset(ddraw, CSTR2SYM("clip_mask"), MAGICK_STRING_TO_OBJ(draw->info->clip_mask)); internal
    // rb_hash_aset(ddraw, CSTR2SYM("bounds"), Import_SegmentInfo(&draw->info->bounds)); internal
    rb_hash_aset(ddraw, CSTR2SYM("clip_units"), INT2FIX(draw->info->clip_units));
    rb_hash_aset(ddraw, CSTR2SYM("opacity"), QUANTUM2NUM(draw->info->opacity));
    // rb_hash_aset(ddraw, CSTR2SYM("render"), draw->info->render ? Qtrue : Qfalse); internal
    // rb_hash_aset(ddraw, CSTR2SYM("element_reference"), Qnil);     // not used yet
    // rb_hash_aset(ddraw, CSTR2SYM("debug"), draw->info->debug ? Qtrue : Qfalse);
#if defined(HAVE_ST_KERNING)
    rb_hash_aset(ddraw, CSTR2SYM("kerning"), rb_float_new(draw->info->kerning));
#endif
#if defined(HAVE_ST_INTERWORD_SPACING)
    rb_hash_aset(ddraw, CSTR2SYM("interword_spacing"), rb_float_new(draw->info->interword_spacing));
#endif

    // Non-DrawInfo fields
    rb_hash_aset(ddraw, CSTR2SYM("primitives"), draw->primitives);
    // rb_hash_aset(ddraw, CSTR2SYM("shadow_color"), Pixel_from_PixelPacket(&draw->shadow_color)); Polaroid-only

    return ddraw;
}


/**
 * Support Marsal.load.
 *
 * Ruby usage:
 *   - @verbatim Draw#marshal_load @endverbatim
 *
 * Notes:
 *   - On entry all fields are all-bits-0
 *
 * @param self this object
 * @param ddraw the marshalled object
 * @return self, once marshalled
 */
VALUE
Draw_marshal_load(VALUE self, VALUE ddraw)
{
    Draw *draw;
    Pixel *pixel;
    volatile VALUE val;

    Data_Get_Struct(self, Draw, draw);

    draw->info = magick_malloc(sizeof(DrawInfo));
    if (!draw->info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }
    GetDrawInfo(NULL, draw->info);

    OBJ_TO_MAGICK_STRING(draw->info->geometry, rb_hash_aref(ddraw, CSTR2SYM("geometry")));

    //val = rb_hash_aref(ddraw, CSTR2SYM("viewbox"));
    //Export_RectangleInfo(&draw->info->viewbox, val);

    val = rb_hash_aref(ddraw, CSTR2SYM("affine"));
    Export_AffineMatrix(&draw->info->affine, val);

    draw->info->gravity = (GravityType) FIX2INT(rb_hash_aref(ddraw, CSTR2SYM("gravity")));

    val = rb_hash_aref(ddraw, CSTR2SYM("fill"));
    Data_Get_Struct(val, Pixel, pixel);
    draw->info->fill =  *pixel;

    val = rb_hash_aref(ddraw, CSTR2SYM("stroke"));
    Data_Get_Struct(val, Pixel, pixel);
    draw->info->stroke = *pixel;

    draw->info->stroke_width = NUM2DBL(rb_hash_aref(ddraw, CSTR2SYM("stroke_width")));
    draw->info->fill_pattern = str_to_image(rb_hash_aref(ddraw, CSTR2SYM("fill_pattern")));
    draw->info->stroke_pattern = str_to_image(rb_hash_aref(ddraw, CSTR2SYM("stroke_pattern")));
    draw->info->stroke_antialias = RTEST(rb_hash_aref(ddraw, CSTR2SYM("stroke_antialias")));
    draw->info->text_antialias = RTEST(rb_hash_aref(ddraw, CSTR2SYM("text_antialias")));
    draw->info->decorate = (DecorationType) FIX2INT(rb_hash_aref(ddraw, CSTR2SYM("decorate")));
    OBJ_TO_MAGICK_STRING(draw->info->font, rb_hash_aref(ddraw, CSTR2SYM("font")));
    OBJ_TO_MAGICK_STRING(draw->info->family, rb_hash_aref(ddraw, CSTR2SYM("family")));

    draw->info->style = (StyleType) FIX2INT(rb_hash_aref(ddraw, CSTR2SYM("style")));
    draw->info->stretch = (StretchType) FIX2INT(rb_hash_aref(ddraw, CSTR2SYM("stretch")));
    draw->info->weight = NUM2ULONG(rb_hash_aref(ddraw, CSTR2SYM("weight")));
    OBJ_TO_MAGICK_STRING(draw->info->encoding, rb_hash_aref(ddraw, CSTR2SYM("encoding")));
    draw->info->pointsize = NUM2DBL(rb_hash_aref(ddraw, CSTR2SYM("pointsize")));
    OBJ_TO_MAGICK_STRING(draw->info->density, rb_hash_aref(ddraw, CSTR2SYM("density")));
    draw->info->align = (AlignType) FIX2INT(rb_hash_aref(ddraw, CSTR2SYM("align")));

    val = rb_hash_aref(ddraw, CSTR2SYM("undercolor"));
    Data_Get_Struct(val, Pixel, pixel);
    draw->info->undercolor = *pixel;

    draw->info->clip_units = FIX2INT(rb_hash_aref(ddraw, CSTR2SYM("clip_units")));
    draw->info->opacity = NUM2QUANTUM(rb_hash_aref(ddraw, CSTR2SYM("opacity")));
#if defined(HAVE_ST_KERNING)
    draw->info->kerning = NUM2DBL(rb_hash_aref(ddraw, CSTR2SYM("kerning")));
#endif
#if defined(HAVE_ST_INTERWORD_SPACING)
    draw->info->interword_spacing = NUM2DBL(rb_hash_aref(ddraw, CSTR2SYM("interword_spacing")));
#endif

    draw->primitives = rb_hash_aref(ddraw, CSTR2SYM("primitives"));

    return self;
}


/**
 * Pointsize attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#pointsize= @endverbatim
 *
 * @param self this object
 * @param pointsize the pointsize
 * @return self
 */
VALUE
Draw_pointsize_eq(VALUE self, VALUE pointsize)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->pointsize = NUM2DBL(pointsize);
    return self;
}


/**
 * Set rotation attribute value.
 *
 * Ruby usage:
 *   - @verbatim Magick::Draw#rotation= @endverbatim
 *
 * Notes:
 *   - Argument should be in degrees
 *   - Taken from Magick++'s Magick::Image::annotate method.
 *     Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
 *
 * @param self this object
 * @param deg the number of degrees
 * @return self
 */
VALUE
Draw_rotation_eq(VALUE self, VALUE deg)
{
    Draw *draw;
    double degrees;
    AffineMatrix affine, current;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    degrees = NUM2DBL(deg);
    if (fabs(degrees) > DBL_EPSILON)
    {
        affine.sx=1.0;
        affine.rx=0.0;
        affine.ry=0.0;
        affine.sy=1.0;
        affine.tx=0.0;
        affine.ty=0.0;

        current = draw->info->affine;
        affine.sx=cos(DegreesToRadians(fmod(degrees,360.0)));
        affine.rx=sin(DegreesToRadians(fmod(degrees,360.0)));
        affine.ry=(-sin(DegreesToRadians(fmod(degrees,360.0))));
        affine.sy=cos(DegreesToRadians(fmod(degrees,360.0)));

        draw->info->affine.sx=current.sx*affine.sx+current.ry*affine.rx;
        draw->info->affine.rx=current.rx*affine.sx+current.sy*affine.rx;
        draw->info->affine.ry=current.sx*affine.ry+current.ry*affine.sy;
        draw->info->affine.sy=current.rx*affine.ry+current.sy*affine.sy;
        draw->info->affine.tx=current.sx*affine.tx+current.ry*affine.ty+current.tx;
    }

    return self;
}


/**
 * Stroke attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#stroke= @endverbatim
 *
 * @param self this object
 * @param stroke the stroke
 * @return self
 */
VALUE
Draw_stroke_eq(VALUE self, VALUE stroke)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->stroke, stroke);
    return self;
}


/**
 * Accept an image as a stroke pattern.
 *
 * Ruby usage:
 *   - @verbatim Draw#stroke_pattern= @endverbatim
 *
 * @param self this object
 * @param pattern the pattern
 * @return self
 * @see Draw_fill_pattern_eq
 */
VALUE
Draw_stroke_pattern_eq(VALUE self, VALUE pattern)
{
    Draw *draw;
    Image *image;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    if (draw->info->stroke_pattern != NULL)
    {
        // Do not trace destruction
        DestroyImage(draw->info->stroke_pattern);
        draw->info->stroke_pattern = NULL;
    }

    if (!NIL_P(pattern))
    {
        // DestroyDrawInfo destroys the clone
        pattern = rm_cur_image(pattern);
        image = rm_check_destroyed(pattern);
        // Do not trace creation
        draw->info->stroke_pattern = rm_clone_image(image);
    }

    return self;
}


/**
 * Stroke_width attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#stroke_width= @endverbatim
 *
 * @param self this object
 * @param stroke_width the stroke_width
 * @return self
 */
VALUE
Draw_stroke_width_eq(VALUE self, VALUE stroke_width)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->stroke_width = NUM2DBL(stroke_width);
    return self;
}


/**
 * Text_antialias attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#text_antialias= @endverbatim
 *
 * @param self this object
 * @param text_antialias the text_antialias
 * @return self
 */
VALUE
Draw_text_antialias_eq(VALUE self, VALUE text_antialias)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->text_antialias = (MagickBooleanType) RTEST(text_antialias);
    return self;
}


/**
 * Tile attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#tile= @endverbatim
 *
 * @param self this object
 * @param image the image to tile
 * @return self
 */
VALUE
Draw_tile_eq(VALUE self, VALUE image)
{
    return Draw_fill_pattern_eq(self, image);
}


/**
 * Undercolor attribute writer.
 *
 * Ruby usage:
 *   - @verbatim Draw#undercolor= @endverbatim
 *
 * @param self this object
 * @param undercolor the undercolor
 * @return self
 */
VALUE
Draw_undercolor_eq(VALUE self, VALUE undercolor)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->undercolor, undercolor);
    return self;
}


/**
 * Annotates an image with text.
 *
 * Ruby usage:
 *   - @verbatim Draw#annotate(img, w, h, x, y, text) <{optional parms}> @endverbatim
 *
 * Notes:
 *   - Additional Draw attribute methods may be called in the optional block,
 *     which is executed in the context of an Draw object.
 *
 * @param self this object
 * @param image_arg the image
 * @param width_arg the width
 * @param height_arg the height
 * @param x_arg x position
 * @param y_arg y position
 * @param text the annotation text
 * @return self
 */
VALUE Draw_annotate(
                   VALUE self,
                   VALUE image_arg,
                   VALUE width_arg,
                   VALUE height_arg,
                   VALUE x_arg,
                   VALUE y_arg,
                   VALUE text)
{
    Draw *draw;
    Image *image;
    unsigned long width, height;
    long x, y;
    AffineMatrix keep;
    char geometry_str[50];

    // Save the affine matrix in case it is modified by
    // Draw#rotation=
    Data_Get_Struct(self, Draw, draw);
    keep = draw->info->affine;

    image_arg = rm_cur_image(image_arg);
    image = rm_check_frozen(image_arg);

    // If we have an optional parm block, run it in self's context,
    // allowing the app a chance to modify the object's attributes
    if (rb_block_given_p())
    {
        (void)rb_obj_instance_eval(0, NULL, self);
    }

    // Translate & store in Draw structure
    draw->info->text = InterpretImageProperties(NULL, image, StringValuePtr(text));
    if (!draw->info->text)
    {
        rb_raise(rb_eArgError, "no text");
    }

    // Create geometry string, copy to Draw structure, overriding
    // any previously existing value.
    width  = NUM2ULONG(width_arg);
    height = NUM2ULONG(height_arg);
    x      = NUM2LONG(x_arg);
    y      = NUM2LONG(y_arg);

    if (width == 0 && height == 0)
    {
        sprintf(geometry_str, "%+ld%+ld", x, y);
    }

    // WxH is non-zero
    else
    {
        sprintf(geometry_str, "%lux%lu%+ld%+ld", width, height, x, y);
    }

    magick_clone_string(&draw->info->geometry, geometry_str);

    (void) AnnotateImage(image, draw->info);

    magick_free(draw->info->text);
    draw->info->text = NULL;
    draw->info->affine = keep;

    rm_check_image_exception(image, RetainOnError);

    return self;
}


/**
 * Clones this object.
 *
 * Ruby usage:
 *   - @verbatim Draw#clone @endverbatim
 *
 * @param self this object
 * @return the clone
 * @see Draw_dup
 * @see Draw_init_copy
 */
VALUE
Draw_clone(VALUE self)
{
    volatile VALUE clone;

    clone = Draw_dup(self);
    if (OBJ_FROZEN(self))
    {
        OBJ_FREEZE(clone);
    }

    return clone;
}


/**
 * Implement the "image" drawing primitive.
 *
 * Ruby usage:
 *   - @verbatim Draw#composite(x,y,width,height,img) @endverbatim
 *   - @verbatim Draw#composite(x,y,width,height,img,operator) @endverbatim
 *
 * Notes:
 *   - Default operator is overComposite
 *   - The "img" argument can be either an ImageList object or an Image
 *     argument.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */
VALUE
Draw_composite(int argc, VALUE *argv, VALUE self)
{
    Draw *draw;
    const char *op = "Over";
    double x, y, width, height;
    CompositeOperator cop = OverCompositeOp;
    volatile VALUE image;
    Image *comp_img;
    struct TmpFile_Name *tmpfile_name;
    char name[MaxTextExtent];
    // Buffer for "image" primitive
    char primitive[MaxTextExtent];

    if (argc < 5 || argc > 6)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 5 or 6)", argc);
    }

    // Retrieve the image to composite
    image = rm_cur_image(argv[4]);
    (void) rm_check_destroyed(image);

    x = NUM2DBL(argv[0]);
    y = NUM2DBL(argv[1]);
    width  = NUM2DBL(argv[2]);
    height = NUM2DBL(argv[3]);

    // The default composition operator is "Over".
    if (argc == 6)
    {
        VALUE_TO_ENUM(argv[5], cop, CompositeOperator);

        switch (cop)
        {
            case AddCompositeOp:
                op = "Add";
                break;
            case AtopCompositeOp:
                op = "Atop";
                break;
            case BlendCompositeOp:
                op = "Blend";
                break;
#if defined(HAVE_ENUM_BLURCOMPOSITEOP)
            case BlurCompositeOp:
                op = "Blur";
                break;
#endif
            case BumpmapCompositeOp:
                op = "Bumpmap";
                break;
            case ChangeMaskCompositeOp:
                op = "ChangeMask";
                break;
            case ClearCompositeOp:
                op = "Clear";
                break;
            case ColorBurnCompositeOp:
                op = "ColorBurn";
                break;
            case ColorDodgeCompositeOp:
                op = "ColorDodge";
                break;
            case ColorizeCompositeOp:
                op = "Colorize";
                break;
            case CopyCompositeOp:
                op = "Copy";
                break;
            case CopyBlackCompositeOp:
                op = "CopyBlack";
                break;
            case CopyBlueCompositeOp:
                op = "CopyBlue";
                break;
            case CopyCyanCompositeOp:
                op = "CopyCyan";
                break;
            case CopyGreenCompositeOp:
                op = "CopyGreen";
                break;
            case CopyMagentaCompositeOp:
                op = "CopyMagenta";
                break;
            case CopyOpacityCompositeOp:
                op = "CopyOpacity";
                break;
            case CopyRedCompositeOp:
                op = "CopyRed";
                break;
            case CopyYellowCompositeOp:
                op = "CopyYellow";
                break;
            case DarkenCompositeOp:
                op = "Darken";
                break;
#if defined(HAVE_ENUM_DISTORTCOMPOSITEOP)
            case DistortCompositeOp:
                op = "Distort";
                break;
#endif
            case DivideCompositeOp:
                op = "Divide";
                break;
            case DstCompositeOp:
                op = "Dst";
                break;
            case DstAtopCompositeOp:
                op = "DstAtop";
                break;
            case DstInCompositeOp:
                op = "DstIn";
                break;
            case DstOutCompositeOp:
                op = "DstOut";
                break;
            case DstOverCompositeOp:
                op = "DstOver";
                break;
            case DifferenceCompositeOp:
                op = "Difference";
                break;
            case DisplaceCompositeOp:
                op = "Displace";
                break;
            case DissolveCompositeOp:
                op = "Dissolve";
                break;
            case ExclusionCompositeOp:
                op = "Exclusion";
                break;
            case HardLightCompositeOp:
                op = "HardLight";
                break;
            case HueCompositeOp:
                op = "Hue";
                break;
            case InCompositeOp:
                op = "In";
                break;
            case LightenCompositeOp:
                op = "Lighten";
                break;
#if defined(HAVE_ENUM_LINEARBURNCOMPOSITEOP)
            case LinearBurnCompositeOp:
                op = "LinearBurn";
                break;
#endif
#if defined(HAVE_ENUM_LINEARDODGECOMPOSITEOP)
            case LinearDodgeCompositeOp:
                op = "LinearDodge";
                break;
#endif
            case LinearLightCompositeOp:
                op = "LinearLight";
                break;
            case LuminizeCompositeOp:
                op = "Luminize";
                break;
            case MinusCompositeOp:
                op = "Minus";
                break;
            case ModulateCompositeOp:
                op = "Modulate";
                break;
            case MultiplyCompositeOp:
                op = "Multiply";
                break;
            case OutCompositeOp:
                op = "Out";
                break;
            case OverCompositeOp:
                op = "Over";
                break;
            case OverlayCompositeOp:
                op = "Overlay";
                break;
#if defined(HAVE_ENUM_PEGTOPLIGHTCOMPOSITEOP)
            case PegtopLightCompositeOp:
                op = "PegtopLight";
                break;
#endif
#if defined(HAVE_ENUM_PINLIGHTCOMPOSITEOP)
            case PinLightCompositeOp:
                op = "PinLight";
                break;
#endif
            case PlusCompositeOp:
                op = "Plus";
                break;
            case ReplaceCompositeOp:
                op = "Replace";
                break;
            case SaturateCompositeOp:
                op = "Saturate";
                break;
            case ScreenCompositeOp:
                op = "Screen";
                break;
            case SoftLightCompositeOp:
                op = "SoftLight";
                break;
            case SrcCompositeOp:
                op = "Src";
                break;
            case SrcAtopCompositeOp:
                op = "SrcAtop";
                break;
            case SrcInCompositeOp:
                op = "SrcIn";
                break;
            case SrcOutCompositeOp:
                op = "SrcOut";
                break;
            case SrcOverCompositeOp:
                op = "SrcOver";
                break;
            case SubtractCompositeOp:
                op = "Subtract";
                break;
            case ThresholdCompositeOp:
                op = "Threshold";
                break;
#if defined(HAVE_ENUM_VIVIDLIGHTCOMPOSITEOP)
            case VividLightCompositeOp:
                op = "VividLight";
                break;
#endif
            case XorCompositeOp:
                op = "Xor";
                break;
            default:
                rb_raise(rb_eArgError, "unknown composite operator (%d)", cop);
                break;
        }
    }

    Data_Get_Struct(self, Draw, draw);

    // Create a temp copy of the composite image
    Data_Get_Struct(image, Image, comp_img);
    rm_write_temp_image(comp_img, name);

    // Add the temp filename to the filename array.
    // Use Magick storage since we need to keep the list around
    // until destroy_Draw is called.
    tmpfile_name = magick_malloc(sizeof(struct TmpFile_Name)+strlen(name));
    strcpy(tmpfile_name->name, name);
    tmpfile_name->next = draw->tmpfile_ary;
    draw->tmpfile_ary = tmpfile_name;

    // Form the drawing primitive
    (void) sprintf(primitive, "image %s %g,%g,%g,%g '%s'", op, x, y, width, height, name);


    // Send "primitive" to self.
    (void) rb_funcall(self, rb_intern("primitive"), 1, rb_str_new2(primitive));

    return self;
}


/**
 * Execute the stored drawing primitives on the current image.
 *
 * Ruby usage:
 *   - @verbatim Draw#draw(i) @endverbatim
 *
 * @param self this object
 * @param image_arg the image argument
 * @return self
 */
VALUE
Draw_draw(VALUE self, VALUE image_arg)
{
    Draw *draw;
    Image *image;

    image_arg = rm_cur_image(image_arg);
    image = rm_check_frozen(image_arg);

    Data_Get_Struct(self, Draw, draw);
    if (draw->primitives == 0)
    {
        rb_raise(rb_eArgError, "nothing to draw");
    }

    // Point the DrawInfo structure at the current set of primitives.
    magick_clone_string(&(draw->info->primitive), StringValuePtr(draw->primitives));

    (void) DrawImage(image, draw->info);
    rm_check_image_exception(image, RetainOnError);

    magick_free(draw->info->primitive);
    draw->info->primitive = NULL;

    return self;
}


/**
 * Copy a Draw object.
 *
 * Ruby usage:
 *   - @verbatim Draw#dup @endverbatim
 *
 * Notes:
 *   - Constructs a new Draw object, then calls initialize_copy.
 *
 * @param self this object
 * @return the duplicate
 * @see Draw_clone
 * @see Draw_init_copy
 */
VALUE
Draw_dup(VALUE self)
{
    Draw *draw;
    volatile VALUE dup;

    draw = ALLOC(Draw);
    memset(draw, 0, sizeof(Draw));
    dup = Data_Wrap_Struct(CLASS_OF(self), mark_Draw, destroy_Draw, draw);
    if (rb_obj_tainted(self))
    {
        (void)rb_obj_taint(dup);
    }
    return rb_funcall(dup, rm_ID_initialize_copy, 1, self);
}


/**
 * Returns measurements for a given font and text string.
 *
 * Ruby usage:
 *   - @verbatim Draw#get_type_metrics(text) @endverbatim
 *   - @verbatim Draw#get_type_metrics(image, text) @endverbatim
 *
 * Notes:
 *   - If the image argument has been omitted, use a dummy image, but make sure
 *     the text has none of the special characters that refer to image
 *     attributes.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the duplicate
 */
VALUE
Draw_get_type_metrics(
                     int argc,
                     VALUE *argv,
                     VALUE self)
{
    return get_type_metrics(argc, argv, self, GetTypeMetrics);
}


/**
 * Returns measurements for a given font and text string.
 *
 * Ruby usage:
 *   - @verbatim Draw#get_multiline_type_metrics(text) @endverbatim
 *   - @verbatim Draw#get_multiline_type_metrics(image, text) @endverbatim
 *
 * Notes:
 *   - If the image argument has been omitted, use a dummy image, but make sure
 *     the text has none of the special characters that refer to image
 *     attributes.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the duplicate
 */
VALUE
Draw_get_multiline_type_metrics(
                               int argc,
                               VALUE *argv,
                               VALUE self)
{
    return get_type_metrics(argc, argv, self, GetMultilineTypeMetrics);
}


/**
 * Initialize clone, dup methods.
 *
 * Ruby usage:
 *   - @verbatim Draw#initialize_copy @endverbatim
 *
 * @param self this object
 * @param orig the original object
 * @return self
 * @see Draw_clone
 * @see Draw_dup
 */
VALUE Draw_init_copy(VALUE self, VALUE orig)
{
    Draw *copy, *original;

    Data_Get_Struct(orig, Draw, original);
    Data_Get_Struct(self, Draw, copy);

    copy->info = CloneDrawInfo(NULL, original->info);
    if (!copy->info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    if (original->primitives)
    {
        copy->primitives = rb_str_dup(original->primitives);
    }

    return self;
}


/**
 * Initialize Draw object.
 *
 * Ruby usage:
 *   - @verbatim Draw#initialize <{ info initializers }> @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
Draw_initialize(VALUE self)
{
    Draw *draw, *draw_options;
    volatile VALUE options;

    Data_Get_Struct(self, Draw, draw);

    options = new_DrawOptions();
    Data_Get_Struct(options, Draw, draw_options);
    draw->info = draw_options->info;
    draw_options->info = NULL;

    return self;
}


/**
 * Display the primitives.
 *
 * Ruby usage:
 *   - @verbatim Draw#inspect @endverbatim
 *
 * @param self this object
 * @return the draw primitives or the Ruby string "(no primitives defined)" if
 * they are not defined
 */
VALUE
Draw_inspect(VALUE self)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    return draw->primitives ? draw->primitives : rb_str_new2("(no primitives defined)");
}


/**
 * Create a new Draw object.
 *
 * Ruby usage:
 *   - @verbatim Draw.new @endverbatim
 *   - @verbatim Draw.allocate @endverbatim
 *
 * @param class the Ruby Draw class
 * @return a new Draw object
 * @throw ImageMagickError if no memory
 */
VALUE Draw_alloc(VALUE class)
{
    Draw *draw;
    volatile VALUE draw_obj;

    draw = ALLOC(Draw);
    memset(draw, 0, sizeof(Draw));
    draw_obj = Data_Wrap_Struct(class, mark_Draw, destroy_Draw, draw);

    return draw_obj;
}


/**
 * Add a drawing primitive to the list of primitives in the Draw object.
 *
 * Ruby usage:
 *   - @verbatim Draw#primitive @endverbatim
 *
 * @param self this object
 * @param primitive the primitive to add
 * @return self
 */
VALUE
Draw_primitive(VALUE self, VALUE primitive)
{
    Draw *draw;

    rb_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    if (draw->primitives == (VALUE)0)
    {
        draw->primitives = primitive;
    }
    else
    {
        draw->primitives = rb_str_concat(draw->primitives, rb_str_new2("\n"));
        draw->primitives = rb_str_concat(draw->primitives, primitive);
    }

    return self;
}


/**
 * Mark referenced objects.
 *
 * No Ruby usage (internal function)
 *
 * @param drawptr pointer to a Draw object
 */
static void
mark_Draw(void *drawptr)
{
    Draw *draw = (Draw *)drawptr;

    if (draw->primitives != (VALUE)0)
    {
        rb_gc_mark(draw->primitives);
    }
}


/**
 * Free the memory associated with an Draw object.
 *
 * No Ruby usage (internal function)
 *
 * @param drawptr pointer to a Draw object
 */
static void
destroy_Draw(void *drawptr)
{
    Draw *draw = (Draw *)drawptr;
    struct TmpFile_Name *tmpfile_name;

    if (draw->info)
    {
        (void) DestroyDrawInfo(draw->info);
        draw->info = NULL;
    }

    // Erase any temporary image files.
    while (draw->tmpfile_ary)
    {
        tmpfile_name = draw->tmpfile_ary;
        draw->tmpfile_ary = draw->tmpfile_ary->next;
        rm_delete_temp_image(tmpfile_name->name);
        magick_free(tmpfile_name);
    }

    xfree(drawptr);
}


/**
 * Allocate & initialize a DrawOptions object.
 *
 * No Ruby usage (internal function)
 *
 * @return a new DrawOptions object
 */
static VALUE
new_DrawOptions(void)
{
    return DrawOptions_initialize(Draw_alloc(Class_DrawOptions));
}


/**
 * Create a DrawOptions object.
 *
 * Ruby usage:
 *   - @verbatim DrawOptions#allocate @endverbatim
 *   - @verbatim DrawOptions#new @endverbatim
 *
 * Notes:
 *   - The DrawOptions class is the same as the Draw class except is has only
 *     the attribute writer functions
 *
 * @param class the Ruby DrawOptions class
 * @return a new DrawOptions object
 */
VALUE
DrawOptions_alloc(VALUE class)
{
    Draw *draw_options;
    volatile VALUE draw_options_obj;

    draw_options = ALLOC(Draw);
    memset(draw_options, 0, sizeof(Draw));
    draw_options_obj = Data_Wrap_Struct(class, mark_Draw, destroy_Draw, draw_options);

    return draw_options_obj;
}


/**
 * Initialize a DrawOptions object.
 *
 * Ruby usage:
 *   - @verbatim DrawOptions#initialize @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
DrawOptions_initialize(VALUE self)
{
    Draw *draw_options;

    Data_Get_Struct(self, Draw, draw_options);
    draw_options->info = magick_malloc(sizeof(DrawInfo));
    if (!draw_options->info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    GetDrawInfo(NULL, draw_options->info);

    if (rb_block_given_p())
    {
        // Run the block in self's context
        (void) rb_obj_instance_eval(0, NULL, self);
    }

    return self;
}


/**
 * Allocate a new Magick::PolaroidOptions object.
 *
 * Ruby usage:
 *   - @verbatim Magick::PolaroidOptions#allocate @endverbatim
 *   - @verbatim Magick::PolaroidOptions#new @endverbatim
 *
 * Notes:
 *   - Internally a PolaroidOptions object is the same as a Draw object. The
 *     methods are implemented by Draw methods in rmdraw.c.
 *
 * @param class the Ruby PoloradoidOptions class
 * @return a new DrawOptions object
 */
VALUE
PolaroidOptions_alloc(VALUE class)
{
    volatile VALUE polaroid_obj;
    ImageInfo *image_info;
    Draw *draw;

    image_info = CloneImageInfo(NULL);

    draw = ALLOC(Draw);
    memset(draw, 0, sizeof(*draw));

    draw->info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
    (void)(void) DestroyImageInfo(image_info);

    polaroid_obj = Data_Wrap_Struct(class, NULL, destroy_Draw, draw);

    return polaroid_obj;
}


/**
 * Yield to an optional block.
 *
 * Ruby usage:
 *   - @verbatim Magick::PolaroidOptions#initialize @endverbatim
 *
 * @param self this object
 * @return self
 */
VALUE
PolaroidOptions_initialize(VALUE self)
{
    Draw *draw;
    ExceptionInfo exception;

    // Default shadow color
    Data_Get_Struct(self, Draw, draw);

    GetExceptionInfo(&exception);
    (void) QueryColorDatabase("gray75", &draw->shadow_color, &exception);
    CHECK_EXCEPTION()
    (void) QueryColorDatabase("#dfdfdf", &draw->info->border_color, &exception);

    if (rb_block_given_p())
    {
        // Run the block in self's context
        (void) rb_obj_instance_eval(0, NULL, self);
    }
    return self;
}


/**
 * Allocate a PolaroidOptions instance.
 *
 * No Ruby usage (internal function)
 *
 * @return new PolaroidOptions object
 */
VALUE
rm_polaroid_new(void)
{
    return PolaroidOptions_initialize(PolaroidOptions_alloc(Class_PolaroidOptions));
}


/**
 * Set the shadow color attribute.
 *
 * Ruby usage:
 *   - @verbatim PolaroidOptions#shadow_color= @endverbatim
 *
 * @param self this object
 * @param shadow the shadow color
 * @return self
 */
VALUE
PolaroidOptions_shadow_color_eq(VALUE self, VALUE shadow)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->shadow_color, shadow);
    return self;
}


/**
 * Set the border color attribute.
 *
 * Ruby usage:
 *   - @verbatim PolaroidOptions#border_color= @endverbatim
 *
 * @param self this object
 * @param border the border color
 * @return self
 */
VALUE
PolaroidOptions_border_color_eq(VALUE self, VALUE border)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->border_color, border);
    return self;
}


/**
 * Create a dummy object of an image class.
 *
 * No Ruby usage (internal function)
 *
 * @param klass the class for which to create a dummy
 * @return the newly allocated dummy
 */
static VALUE
get_dummy_tm_img(VALUE klass)
{
#define DUMMY_IMG_CLASS_VAR "@@_dummy_img_"
    volatile VALUE dummy_img = 0;
    Info *info;
    Image *image;

    if (rb_cvar_defined(klass, rb_intern(DUMMY_IMG_CLASS_VAR)) != Qtrue)
    {

        info = CloneImageInfo(NULL);
        if (!info)
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        image = AcquireImage(info);
        if (!image)
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        (void) DestroyImageInfo(info);
        dummy_img = rm_image_new(image);

        rb_cv_set(klass, DUMMY_IMG_CLASS_VAR, dummy_img);
    }
    dummy_img = rb_cv_get(klass, DUMMY_IMG_CLASS_VAR);

    return dummy_img;
}


/**
 * Call a get-type-metrics function.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - called by Draw_get_type_metrics and Draw_get_multiline_type_metrics
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @param getter which type metrics to get
 * @return the type metrics
 * @see Draw_get_type_metrics
 * @see Draw_get_multiline_type_metrics
 */
static VALUE
get_type_metrics(
                int argc,
                VALUE *argv,
                VALUE self,
                get_type_metrics_func_t getter)
{
    static char attrs[] = "OPbcdefghiklmnopqrstuwxyz[@#%";
 #define ATTRS_L ((int)(sizeof(attrs)-1))
    Image *image;
    Draw *draw;
    volatile VALUE t;
    TypeMetric metrics;
    char *text = NULL;
    long text_l;
    long x;
    unsigned int okay;

    switch (argc)
    {
        case 1:                   // use default image
            text = rm_str2cstr(argv[0], &text_l);

            for (x = 0; x < text_l; x++)
            {
                // Ensure text string doesn't refer to image attributes.
                if (text[x] == '%' && x < text_l-1)
                {
                    int y;
                    char spec = text[x+1];

                    for (y = 0; y < ATTRS_L; y++)
                    {
                        if (spec == attrs[y])
                        {
                            rb_raise(rb_eArgError,
                                     "text string contains image attribute reference `%%%c'",
                                     spec);
                        }
                    }
                }
            }

            Data_Get_Struct(get_dummy_tm_img(CLASS_OF(self)), Image, image);
            break;
        case 2:
            t = rm_cur_image(argv[0]);
            image = rm_check_destroyed(t);
            text = rm_str2cstr(argv[1], &text_l);
            break;                  // okay
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    if (text_l == 0)
    {
        rb_raise(rb_eArgError, "no text to measure");
    }

    Data_Get_Struct(self, Draw, draw);
    draw->info->text = InterpretImageProperties(NULL, image, text);
    if (!draw->info->text)
    {
        rb_raise(rb_eArgError, "no text to measure");
    }

    okay = (*getter)(image, draw->info, &metrics);

    magick_free(draw->info->text);
    draw->info->text = NULL;

    if (!okay)
    {
        rm_check_image_exception(image, RetainOnError);

        // Shouldn't get here...
        rb_raise(rb_eRuntimeError, "Can't measure text. Are the fonts installed? "
                 "Is the FreeType library installed?");
    }
    return Import_TypeMetric(&metrics);
}
