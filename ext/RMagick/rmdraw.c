/* $Id: rmdraw.c,v 1.13 2004/06/12 21:52:40 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2004 by Timothy P. Hunter
| Name:     rmdraw.c
| Author:   Tim Hunter
| Purpose:  Contains Draw class methods.
|           Contains Montage class methods.
\============================================================================*/

#include "rmagick.h"

static void mark_Draw(void *);
static void destroy_Draw(void *);
static void destroy_Montage(void *);

/*
    Method:     Draw#affine=
    Purpose:    set the affine matrix from an Magick::AffineMatrix
*/
VALUE
Draw_affine_eq(VALUE self, VALUE matrix)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    AffineMatrix_to_AffineMatrix(&draw->info->affine, matrix);
    return self;
}

/*
    Method:     Draw#align=
    Purpose:    set the text alignment
*/
VALUE
Draw_align_eq(VALUE self, VALUE align)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(align, draw->info->align, AlignType);
    return self;
}

/*
    Method:     Draw#decorate=
    Purpose:    decorate attribute writer
*/
VALUE
Draw_decorate_eq(VALUE self, VALUE decorate)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(decorate, draw->info->decorate, DecorationType);
    return self;
}

/*
    Method:     Draw#density=
    Purpose:    density attribute writer
*/
VALUE
Draw_density_eq(VALUE self, VALUE density)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->density, STRING_PTR(density));

    return self;
}

/*
    Method:     Draw#encoding=
    Purpose:    encoding attribute writer
*/
VALUE
Draw_encoding_eq(VALUE self, VALUE encoding)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->encoding, STRING_PTR(encoding));

    return self;
}

/*
    Method:     Draw#fill=
    Purpose:    fill attribute writer
*/
VALUE
Draw_fill_eq(VALUE self, VALUE fill)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->fill, fill);
    return self;
}

/*
    Method:     Draw#font=
    Purpose:    font attribute writer
*/
VALUE
Draw_font_eq(VALUE self, VALUE font)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->font, STRING_PTR(font));

    return self;
}

/*
    Method:     Draw#font_family=
    Purpose:    font_family attribute writer
*/
VALUE
Draw_font_family_eq(VALUE self, VALUE family)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->family, STRING_PTR(family));

    return self;
}

/*
    Method:     Draw#font_stretch=
    Purpose:    font_stretch attribute writer
*/
VALUE
Draw_font_stretch_eq(VALUE self, VALUE stretch)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(stretch, draw->info->stretch, StretchType);
    return self;
}

/*
    Method:     Draw#font_style=
    Purpose:    font_style attribute writer
*/
VALUE
Draw_font_style_eq(VALUE self, VALUE style)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(style, draw->info->style, StyleType);
    return self;
}

/*
    Method:     Draw#font_weight=
    Purpose:    font_weight attribute writer
    Notes:      The font weight can be one of the font weight constants
                or a number between 100 and 900
*/


VALUE
Draw_font_weight_eq(VALUE self, VALUE weight)
{
    Draw *draw;
    WeightType w;

    rm_check_frozen(self);
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

/*
    Method:     Draw#gravity=
    Purpose:    gravity attribute writer
    Notes:      From Magick++'s Image.h header file:
       Gravity affects text placement in bounding area according to rules:
        NorthWestGravity  text bottom-left corner placed at top-left
        NorthGravity      text bottom-center placed at top-center
        NorthEastGravity  text bottom-right corner placed at top-right
        WestGravity       text left-center placed at left-center
        CenterGravity     text center placed at center
        EastGravity       text right-center placed at right-center
        SouthWestGravity  text top-left placed at bottom-left
        SouthGravity      text top-center placed at bottom-center
        SouthEastGravity  text top-right placed at bottom-right
*/
VALUE
Draw_gravity_eq(VALUE self, VALUE grav)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    VALUE_TO_ENUM(grav, draw->info->gravity, GravityType);

    return self;
}

/*
    Method:     Draw#pointsize=
    Purpose:    pointsize attribute writer
*/
VALUE
Draw_pointsize_eq(VALUE self, VALUE pointsize)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->pointsize = NUM2DBL(pointsize);
    return self;
}

/*
    Method:     Magick::Draw#rotation=degrees
    Purpose:    set rotation attribute value
    Notes:      Taken from Magick++'s Magick::Image::annotate method
                Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
*/
VALUE
Draw_rotation_eq(VALUE self, VALUE deg)
{
    Draw *draw;
    double degrees;
    AffineMatrix affine, current;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    degrees = NUM2DBL(deg);
    if (degrees != 0.0)
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

/*
    Method:     Draw#stroke=
    Purpose:    stroke attribute writer
*/
VALUE
Draw_stroke_eq(VALUE self, VALUE stroke)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->stroke, stroke);
    return self;
}

/*
    Method:     Draw#text_antialias=
    Purpose:    text_antialias attribute writer
*/
VALUE
Draw_text_antialias_eq(VALUE self, VALUE text_antialias)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    draw->info->text_antialias = RTEST(text_antialias);
    return self;
}

/*
    Method:     Draw#undercolor=
    Purpose:    undercolor attribute writer
*/
VALUE
Draw_undercolor_eq(VALUE self, VALUE undercolor)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->undercolor, undercolor);
    return self;
}

/*
    Method:     Draw#annotate(img, w, h, x, y, text) <{optional parms}>
    Purpose:    annotates an image with text
    Returns:    self
    Notes:      Additional Draw attribute methods may be called in the
                optional block, which is executed in the context of an
                Draw object.
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

    // If we have an optional parm block, run it in self's context,
    // allowing the app a chance to modify the object's attributes
    if (rb_block_given_p())
    {
        rb_obj_instance_eval(0, NULL, self);
    }

    rm_check_frozen(ImageList_cur_image(image_arg));
    Data_Get_Struct(ImageList_cur_image(image_arg), Image, image);

    // Copy text to Draw structure
    magick_clone_string(&draw->info->text, STRING_PTR(text));

    // Create geometry string, copy to Draw structure, overriding
    // any previously existing value.
    width  = NUM2ULONG(width_arg);
    height = NUM2ULONG(height_arg);
    x      = NUM2LONG(x_arg);
    y      = NUM2LONG(y_arg);

    // If either the width or height is 0, both must be 0.
    if (width == 0 || height == 0)
    {
        if (width != 0 || height != 0)
        {
            rb_raise(rb_eArgError, "invalid geometry %lux%lu%+ld%+ld", width, height, x, y);
        }
        sprintf(geometry_str, "%+ld%+ld", x, y);
    }

    // WxH is non-zero
    else
    {
        sprintf(geometry_str, "%lux%lu%+ld%+ld", width, height, x, y);
    }

    magick_clone_string(&draw->info->geometry, geometry_str);

    (void) AnnotateImage(image, draw->info);

    draw->info->affine = keep;

    HANDLE_IMG_ERROR(image)

    return self;
}


/*
    Method:     Draw#clone
    Notes:      see dup, init_copy
*/
VALUE
Draw_clone(VALUE self)
{
    volatile VALUE clone;

    clone = Draw_dup(self);
    if (OBJ_FROZEN(self))
    {
        rb_obj_freeze(clone);
    }

    return clone;
}


/*
    Method:     Draw#composite(x,y,width,height,img,operator=OverCompositeOp)
    Purpose:    Implement the "image" drawing primitive
    Notes:      The "img" argument can be either an ImageList object
                or an Image argument.
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
    char name[MaxTextExtent];
                            // Buffer for "image" primitive
    char primitive[MaxTextExtent];

    if (argc < 5 || argc > 6)
    {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for 5 or 6)", argc);
    }

    x = NUM2DBL(argv[0]);
    y = NUM2DBL(argv[1]);
    width  = NUM2DBL(argv[2]);
    height = NUM2DBL(argv[3]);

    // The default composition operator is "Over".
    if (argc == 6)
    {
        VALUE_TO_ENUM(argv[5], cop, CompositeOperator);

        switch(cop)
        {
            case AddCompositeOp:
                op = "Add";
                break;
            case AtopCompositeOp:
                op = "Atop";
                break;
            case BumpmapCompositeOp:
                op = "Bumpmap";
                break;
            case ClearCompositeOp:
                op = "Clear";
                break;
            case CopyBlueCompositeOp:
                op = "CopyBlue";
                break;
            case CopyGreenCompositeOp:
                op = "CopyGreen";
                break;
            case CopyOpacityCompositeOp:
                op = "CopyOpacity";
                break;
            case CopyRedCompositeOp:
                op = "CopyRed";
                break;
            case CopyCompositeOp:
                op = "Copy";
                break;
            case DifferenceCompositeOp:
                op = "Difference";
                break;
            case InCompositeOp:
                op = "In";
                break;
            case MinusCompositeOp:
                op = "Minus";
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
            case PlusCompositeOp:
                op = "Plus";
                break;
            case SubtractCompositeOp:
                op = "Subtract";
                break;
            case XorCompositeOp:
                op = "Xor";
                break;
            default:
                rb_raise(rb_eArgError, "unknown composite operator (%d)", cop);
                break;
        }
    }

    Data_Get_Struct(self, Draw, draw);

    // Retrieve the image to composite
    image = ImageList_cur_image(argv[4]);

    // Create a temp copy of the composite image
    Data_Get_Struct(image, Image, comp_img);
    write_temp_image(comp_img, name);

    // Add the temp filename to the filename array
    if (!draw->tmpfile_ary)
    {
        draw->tmpfile_ary = rb_ary_new();
    }
    rb_ary_push(draw->tmpfile_ary, rb_str_new2(name));

    // Form the drawing primitive
    (void) sprintf(primitive, "image %s %g,%g,%g,%g '%s'", op, x, y, width, height, name);


    // Send "primitive" to self.
    (void) rb_funcall(self, rb_intern("primitive"), 1, rb_str_new2(primitive));

    return self;
}

/*
    Method:     Draw#draw(i)
    Purpose:    Execute the stored drawing primitives on the current image
                image
*/
VALUE
Draw_draw(VALUE self, VALUE image_arg)
{
    Draw *draw;
    Image *image;

    Data_Get_Struct(self, Draw, draw);
    if (draw->primitives == 0)
    {
        rb_raise(rb_eArgError, "nothing to draw");
    }

    rm_check_frozen(ImageList_cur_image(image_arg));
    Data_Get_Struct(ImageList_cur_image(image_arg), Image, image);

    // Point the DrawInfo structure at the current set of primitives.
    magick_clone_string(&(draw->info->primitive), STRING_PTR(draw->primitives));

    (void) DrawImage(image, draw->info);
    HANDLE_IMG_ERROR(image)

    magick_free(draw->info->primitive);
    draw->info->primitive = NULL;

    return self;
}


/*
    Methods:    Draw#dup
    Purpose:    Copy a Draw object
    Notes:      Constructs a new Draw object, then calls initialize_copy
*/
VALUE
Draw_dup(VALUE self)
{
    Draw *draw;
    volatile VALUE dup;

    draw = ALLOC(Draw);
    memset(draw, '\0', sizeof(Draw));
    dup = Data_Wrap_Struct(CLASS_OF(self), mark_Draw, destroy_Draw, draw);
    if (rb_obj_tainted(self))
    {
        rb_obj_taint(dup);
    }
    return rb_funcall(dup, ID_initialize_copy, 1, self);
}


/*
    Method:     Draw#get_type_metrics([image, ]text)
    Purpose:    returns measurements for a given font and text string
    Notes:      If the image argument has been omitted, use a dummy
                image, but make sure the text has none of the special
                characters that refer to image attributes.
*/

static VALUE get_dummy_tm_img(VALUE klass)
{
    volatile VALUE dummy_img = 0;
    Info *info;
    Image *image;

    if (rb_cvar_defined(klass, ID__dummy_img_) != Qtrue)
    {

        info = CloneImageInfo(NULL);
        if (!info)
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        image = AllocateImage(info);
        if (!image)
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        DestroyImageInfo(info);
        dummy_img = rm_image_new(image);

        RUBY18(rb_cvar_set(klass, ID__dummy_img_, dummy_img, 0));
        RUBY16(rb_cvar_set(klass, ID__dummy_img_, dummy_img));
    }
    dummy_img = rb_cvar_get(klass, ID__dummy_img_);

    return dummy_img;
}


VALUE
Draw_get_type_metrics(
    int argc,
    VALUE *argv,
    VALUE self)
{
    static char attrs[] = "bcdefghiklmnopqrstuwxyz";
#define ATTRS_L (sizeof(attrs)-1)
    Image *image;
    Draw *draw;
    TypeMetric metrics;
    char *text;
    long text_l;
    int x;
    unsigned int okay;

    switch (argc)
    {
        case 1:                   // use default image
            text = STRING_PTR_LEN(argv[0], text_l);

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
            Data_Get_Struct(ImageList_cur_image(argv[0]), Image, image);
            text = STRING_PTR(argv[1]);
            break;                  // okay
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;
    }

    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->text, text);

    okay = GetTypeMetrics(image, draw->info, &metrics);

    if (!okay)
    {
        rb_warning("RMagick: get_type_metrics failed");
        return Qnil;
    }
    return TypeMetric_from_TypeMetric(&metrics);
}

/*
    Method:     Draw#initialize_copy
    Purpose:    initialize clone, dup methods
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

/*
    Method:     Draw#initialize <{ info initializers }>
    Purpose:    Initialize Draw object
    Notes:      Here are the DrawInfo fields that are copied from Info.
                These are the only Info fields that can be usefully
                set in the initializer block.

                DrawInfo            Info
                --------            ---------
                stroke_antialias    antialias
                font                font
                density             density
                text_antialias      antialias
                pointsize           pointsize
                border_color        border_color
                server_name         server_name
                debug               debug
*/
VALUE
Draw_initialize(VALUE self)
{
    Draw *draw;
    Info *info;
    volatile VALUE info_obj;

    Data_Get_Struct(self, Draw, draw);

    // Create a new Info object, running the info parms block in the process
    info_obj = rm_info_new();

    // Use the Info structure to create the DrawInfo structure
    Data_Get_Struct(info_obj, Info, info);
    draw->info = CloneDrawInfo(info, NULL);

    draw->primitives = 0;
    draw->tmpfile_ary = 0;

    return self;
}

/*
    Method:     inspect
    Purpose:    display the primitives
*/
VALUE
Draw_inspect(VALUE self)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    return draw->primitives ? draw->primitives : rb_str_new2("(no primitives defined)");
}

/*
    Method:     Draw.new/Draw.allocate
    Purpose:    Create a new Draw object
    Raises:     ImageMagickError if no memory
*/
VALUE
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
Draw_alloc(VALUE class)
#else
Draw_new(VALUE class)
#endif
{
    Draw *draw;
    volatile VALUE draw_obj;

    draw = ALLOC(Draw);
    memset(draw, '\0', sizeof(Draw));
    draw_obj = Data_Wrap_Struct(class, mark_Draw, destroy_Draw, draw);

#if !defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_obj_call_init(draw_obj, 0, NULL);
#endif

    return draw_obj;
}

/*
    Method:     Draw#primitive
    Purpose:    Add a drawing primitive to the list of primitives in the
                Draw object
*/
VALUE
Draw_primitive(VALUE self, VALUE primitive)
{
    Draw *draw;

    rm_check_frozen(self);
    Data_Get_Struct(self, Draw, draw);

    if (!draw->primitives)
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

/*
    Static:     mark_Draw
    Purpose:    mark referenced objects
*/
static void
mark_Draw(void *drawptr)
{
    Draw *draw = (Draw *)drawptr;

    if (draw->tmpfile_ary)
    {
        rb_gc_mark(draw->tmpfile_ary);
    }
    if (draw->primitives)
    {
        rb_gc_mark(draw->primitives);
    }
}

/*
    Static:     destroy_Draw
    Purpose:    free the memory associated with an Draw object
*/
static void
destroy_Draw(void *drawptr)
{
    Draw *draw = (Draw *)drawptr;
    volatile VALUE tmpfile;

    DestroyDrawInfo(draw->info);

    // Erase any temporary image files.
    if (draw->tmpfile_ary)
    {
        while ((tmpfile = rb_ary_shift(draw->tmpfile_ary)) != Qnil)
        {
            delete_temp_image(STRING_PTR(tmpfile));
        }
    }

    xfree(drawptr);
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
    strncpy(montage->info->filename, STRING_PTR(filename), MaxTextExtent-1);
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
    magick_clone_string(&montage->info->font, STRING_PTR(font));

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
    frame = rb_funcall(frame_arg, ID_to_s, 0);
    magick_clone_string(&montage->info->frame, STRING_PTR(frame));

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
    geometry = rb_funcall(geometry_arg, ID_to_s, 0);
    magick_clone_string(&montage->info->geometry, STRING_PTR(geometry));

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
    Method:     Montage.new
    Purpose:    Create a new Montage object
*/
VALUE
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
Montage_alloc(VALUE class)
#else
Montage_new(VALUE class)
#endif
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
    DestroyImageInfo(image_info);

    if (!montage_info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to initialize Magick::Montage object");
    }

    montage = ALLOC(Montage);
    montage->info = montage_info;
    montage->compose = OverCompositeOp;
    montage_obj = Data_Wrap_Struct(class, NULL, destroy_Montage, montage);

#if !defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    rb_obj_call_init(montage_obj, 0, NULL);
#endif

    return montage_obj;
}

/*
    Extern:     rm_montage_new()
    Purpose:    Return a new Magick::Montage object
*/

VALUE rm_montage_new(void)
{
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
    return Montage_initialize(Montage_alloc(Class_Montage));
#else
    return Montage_new(Class_Montage);
#endif
}

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
    if (montage->info->texture != NULL)
    {
        delete_temp_image(montage->info->texture);
    }
    DestroyMontageInfo(montage->info);
    xfree(montage);
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
    montage->info->shadow = RTEST(shadow);
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
    char tmpnam[MaxTextExtent];

    Data_Get_Struct(self, Montage, montage);

    // If we had a previously defined temp texture image,
    // remove it now in preparation for this new one.
    if (montage->info->texture)
    {
        magick_free(montage->info->texture);
        montage->info->texture = NULL;
    }

    Data_Get_Struct(ImageList_cur_image(texture), Image, texture_image);

    // Write a temp copy of the image & save its name.
    write_temp_image(texture_image, tmpnam);
    magick_clone_string(&montage->info->texture, tmpnam);

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
    tile = rb_funcall(tile_arg, ID_to_s, 0);
    magick_clone_string(&montage->info->tile, STRING_PTR(tile));

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
    magick_clone_string(&montage->info->title, STRING_PTR(title));
    return self;
}

