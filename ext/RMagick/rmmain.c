/* $Id: rmmain.c,v 1.9 2003/07/30 23:49:03 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2003 by Timothy P. Hunter
| Name:     rmmain.c
| Author:   Tim Hunter
| Purpose:  Contains all module, class, method declarations.
|           Defines all constants
|           Contains Draw class methods.
|           Contains Magick module methods.
\============================================================================*/

#define MAIN                        // Define external variables
#include "rmagick.h"

/*----------------------------------------------------------------------------\
| External declarations
\----------------------------------------------------------------------------*/
void Init_RMagick(void);

static void mark_Draw(void *);
static void destroy_Draw(void *);
static void destroy_Montage(void *);


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
    const ColorInfo **color_ary;
    ExceptionInfo exception;
    VALUE ary, el;
    int x;

    GetExceptionInfo(&exception);

    color_ary = GetColorInfoArray(&exception);
    HANDLE_ERROR

    ary = rb_ary_new();

    x = 0;
    while (color_ary[x])
    {
        rb_ary_push(ary, ColorInfo_to_Struct(color_ary[x]));
        x += 1;
    }

    magick_free(color_ary);

#else
    const ColorInfo *color_list;
    ColorInfo *color;
    ExceptionInfo exception;
    VALUE ary, el;

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
        rb_ary_push(ary, ColorInfo_to_Struct(color));
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
    VALUE ary;

    GetExceptionInfo(&exception);

    type_list = GetTypeInfo("*", &exception);
    HANDLE_ERROR

    // If block, iterate over fonts
    if (rb_block_given_p())
    {
        for (type = (TypeInfo *)type_list; type; type = type->next)
        {
            next = type->next;  // Protect against recursive calls to GetTypeInfo
            if (! type->stealth)
            {
                rb_yield(TypeInfo_to_Struct(type));
            }
        }

        return class;
    }
    else
    {
        ary = rb_ary_new();
        for (type = (TypeInfo *)type_list; type; type = type->next)
        {
            rb_ary_push(ary, TypeInfo_to_Struct(type));
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
    VALUE formats;
    ExceptionInfo exception;
    char mode[5] = {0};

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
    const ExtendedSignedIntegralType quantum,
    const ExtendedUnsignedIntegralType span,
    ExceptionInfo *exception)
{
    VALUE monitor;
    VALUE args[3];

    if (rb_cvar_defined(Module_Magick, Magick_Monitor))
    {
        args[0] = rb_str_new2(text);
        // Convert these possibly-64-bit types to 32-bit types that
        // Ruby can handle.
        args[1] = INT2NUM((long) quantum);
        args[2] = UINT2NUM((unsigned long) span);

        monitor = rb_cvar_get(Module_Magick, Magick_Monitor);
        (void) rb_funcall2(monitor, call_ID, 3, args);
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
#else
    rb_notimplement();
#endif
    return class;
}


/*
    Method:     Draw#affine=
    Purpose:    set the affine matrix from an Magick::AffineMatrix
*/
static VALUE
Draw_affine_eq(VALUE self, VALUE matrix)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    Struct_to_AffineMatrix(&draw->info->affine, matrix);
    return self;
}

/*
    Method:     Draw#align=
    Purpose:    set the text alignment
*/
static VALUE
Draw_align_eq(VALUE self, VALUE align)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    draw->info->align = Num_to_AlignType(align);
    return self;
}

/*
    Method:     Draw#decorate=
    Purpose:    decorate attribute writer
*/
static VALUE
Draw_decorate_eq(VALUE self, VALUE decoration)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    draw->info->decorate = Num_to_DecorationType(decoration);
    return self;
}

/*
    Method:     Draw#density=
    Purpose:    density attribute writer
*/
static VALUE
Draw_density_eq(VALUE self, VALUE density)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->density, STRING_PTR(density));

    return self;
}

/*
    Method:     Draw#encoding=
    Purpose:    encoding attribute writer
*/
static VALUE
Draw_encoding_eq(VALUE self, VALUE encoding)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->encoding, STRING_PTR(encoding));

    return self;
}

/*
    Method:     Draw#fill=
    Purpose:    fill attribute writer
*/
static VALUE
Draw_fill_eq(VALUE self, VALUE fill)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->fill, fill);
    return self;
}

/*
    Method:     Draw#font=
    Purpose:    font attribute writer
*/
static VALUE
Draw_font_eq(VALUE self, VALUE font)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->font, STRING_PTR(font));

    return self;
}

/*
    Method:     Draw#font_family=
    Purpose:    font_family attribute writer
*/
static VALUE
Draw_font_family_eq(VALUE self, VALUE family)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    magick_clone_string(&draw->info->family, STRING_PTR(family));

    return self;
}

/*
    Method:     Draw#font_stretch=
    Purpose:    font_stretch attribute writer
*/
static VALUE
Draw_font_stretch_eq(VALUE self, VALUE stretch)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    draw->info->stretch = Num_to_StretchType(stretch);
    return self;
}

/*
    Method:     Draw#font_style=
    Purpose:    font_style attribute writer
*/
static VALUE
Draw_font_style_eq(VALUE self, VALUE style)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    draw->info->style = Num_to_StyleType(style);
    return self;
}

/*
    Method:     Draw#font_weight=
    Purpose:    font_weight attribute writer
    Notes:      The font weight can be one of the font weight constants
                or a number between 100 and 900
*/

typedef enum {
    AnyWeight,
    NormalWeight,
    BoldWeight,
    BolderWeight,
    LighterWeight
    } WeightType;


static VALUE
Draw_font_weight_eq(VALUE self, VALUE weight)
{
    Draw *draw;
    WeightType w = FIX2INT(weight);

    Data_Get_Struct(self, Draw, draw);

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
            if (w >= 100 && w <= 900)
                draw->info->weight = w;
            else
                rb_raise(rb_eArgError, "invalid font weight (%d given)", w);
            break;
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
static VALUE
Draw_gravity_eq(VALUE self, VALUE grav)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    draw->info->gravity = Num_to_GravityType(grav);

    return self;
}

/*
    Method:     Draw#pointsize=
    Purpose:    pointsize attribute writer
*/
static VALUE
Draw_pointsize_eq(VALUE self, VALUE pointsize)
{
    Draw *draw;

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
static VALUE
Draw_rotation_eq(VALUE self, VALUE deg)
{
    Draw *draw;
    double degrees;
    AffineMatrix affine, current;

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
static VALUE
Draw_stroke_eq(VALUE self, VALUE stroke)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    Color_to_PixelPacket(&draw->info->stroke, stroke);
    return self;
}

/*
    Method:     Draw#text_antialias=
    Purpose:    text_antialias attribute writer
*/
static VALUE
Draw_text_antialias_eq(VALUE self, VALUE text_antialias)
{
    Draw *draw;

    Data_Get_Struct(self, Draw, draw);
    draw->info->text_antialias = RTEST(text_antialias);
    return self;
}

/*
    Method:     Draw#undercolor=
    Purpose:    undercolor attribute writer
*/
static VALUE
Draw_undercolor_eq(VALUE self, VALUE undercolor)
{
    Draw *draw;

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
VALUE
static Draw_annotate(
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
    Method:     Draw#composite(x,y,width,height,img<,operator>)
    Purpose:    Implement the "image" drawing primitive
    Notes:      The "img" argument can be either an ImageList object
                or an Image argument
*/
static VALUE
Draw_composite(int argc, VALUE *argv, VALUE self)
{
    Draw *draw;
    const char *op = "Over";
    double x, y, width, height;
    int cop;
    VALUE image;
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
        if (TYPE(argv[5]) != T_FIXNUM)
        {
            rb_raise(rb_eTypeError, "composite operator must be a Fixnum (%s given)",
                                rb_class2name(CLASS_OF(argv[5])));
        }
        cop = FIX2INT(argv[5]);
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
    Method:     Draw#get_type_metrics([image, ]text)
    Purpose:    returns measurements for a given font and text string
    Notes:      If the image argument has been omitted, use a dummy
                image, but make sure the text has none of the special 
                characters that refer to image attributes.
*/

static VALUE get_dummy_tm_img(VALUE klass)
{
    VALUE dummy_img = 0;
    Info *info;
    Image *image;

    if (rb_cvar_defined(klass, _dummy_img__ID) != Qtrue)
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
        
        RUBY18(rb_cvar_set(klass, _dummy_img__ID, dummy_img, 0));
        RUBY16(rb_cvar_set(klass, _dummy_img__ID, dummy_img));
    }
    dummy_img = rb_cvar_get(klass, _dummy_img__ID);
    
    return dummy_img;
}


static VALUE
Draw_get_type_metrics(
    int argc,
    VALUE *argv,
    VALUE self)
{
    static char attrs[] = "bcdefhiklmnopqstuwxy";
#define ATTRS_L (sizeof(attrs)-1)
    Image *image;
    Draw *draw;
    TypeMetric metrics;
    char *text;
    Strlen_t text_l;
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
    return TypeMetric_to_Struct(&metrics);
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
static VALUE
Draw_initialize(VALUE self)
{
    Draw *draw;
    Info *info;
    VALUE info_obj;

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
    return draw->primitives ? draw->primitives : rb_str_new2("");
}

/*
    Method:     Draw.new
    Purpose:    Create a new Draw object
    Raises:     ImageMagickError if no memory
*/
static VALUE
RUBY16(Draw_new(VALUE class))
RUBY18(Draw_alloc(VALUE class))
{
    Draw *draw;
    VALUE draw_obj;

    draw = ALLOC(Draw);
    memset(draw, '\0', sizeof(Draw));
    draw_obj = Data_Wrap_Struct(class, mark_Draw, destroy_Draw, draw);

    RUBY16(rb_obj_call_init(draw_obj, 0, NULL);)

    return draw_obj;
}

/*
    Method:     Draw#primitive
    Purpose:    Add a drawing primitive to the list of primitives in the
                Draw object
*/
static VALUE
Draw_primitive(VALUE self, VALUE primitive)
{
    Draw *draw;

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
    VALUE tmpfile;

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
static VALUE
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
static VALUE
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
static VALUE
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
static VALUE
Montage_compose_eq(VALUE self, VALUE compose)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    montage->compose = Num_to_CompositeOperator(compose);
    return self;
}

/*
    Method:     Magick::Montage#filename(name)
    Purpose:    set filename value
*/
static VALUE
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
static VALUE
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
static VALUE
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
    Notes:      The geometry is in the form:
                <width>x<height>+<outer-bevel-width>+<inner-bevel-width>
*/
static VALUE
Montage_frame_eq(VALUE self, VALUE frame)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    magick_clone_string(&montage->info->frame, STRING_PTR(frame));

    return self;
}

/*
    Method:     Magick::Montage#geometry(geometry)
    Purpose:    set geometry value
*/
static VALUE
Montage_geometry_eq(VALUE self, VALUE geometry)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    magick_clone_string(&montage->info->geometry, STRING_PTR(geometry));

    return self;
}

/*
    Method:     Magick::Montage#gravity(gravity-type)
    Purpose:    set gravity value
*/
static VALUE
Montage_gravity_eq(VALUE self, VALUE gravity)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    montage->info->gravity = Num_to_GravityType(gravity);
    return self;
}

/*
    Method:     Magick::Montage#initialize
    Purpose:    Place-holder
*/
static VALUE
Montage_initialize(VALUE self)
{
    // Nothing to do!
    return self;
}

/*
    Method:     Magick::Montage#matte_color(color-name)
    Purpose:    set matte_color value
*/
static VALUE
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
static VALUE
RUBY16(Montage_new(VALUE class))
RUBY18(Montage_alloc(VALUE class))
{
    MontageInfo *montage_info;
    Montage *montage;
    Info *image_info;
    VALUE montage_obj;

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

    RUBY16(rb_obj_call_init(montage_obj, 0, NULL);)

    return montage_obj;
}

/*
    Extern:     rm_montage_new()
    Purpose:    Return a new Magick::Montage object
*/

VALUE rm_montage_new()
{
    RUBY16(return Montage_new(Class_Montage);)
    RUBY18(return Montage_initialize(Montage_alloc(Class_Montage)));
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
static VALUE
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
static VALUE
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
static VALUE
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
static VALUE
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
static VALUE
Montage_tile_eq(VALUE self, VALUE tile)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    magick_clone_string(&montage->info->tile, STRING_PTR(tile));

    return self;
}

/*
    Method:     Magick::Montage#title(title)
    Purpose:    set title value
*/
static VALUE
Montage_title_eq(VALUE self, VALUE title)
{
    Montage *montage;

    Data_Get_Struct(self, Montage, montage);
    magick_clone_string(&montage->info->title, STRING_PTR(title));
    return self;
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
    const char *mgk_version;

    InitializeMagick("RMagick");

    Module_Magick = rb_define_module("Magick");

    // Module Magick methods
    rb_define_module_function(Module_Magick, "colors", Magick_colors, 0);
    rb_define_module_function(Module_Magick, "fonts", Magick_fonts, 0);
    rb_define_module_function(Module_Magick, "init_formats", Magick_init_formats, 0);
    rb_define_module_function(Module_Magick, "set_monitor", Magick_set_monitor, 1);
    rb_define_module_function(Module_Magick, "set_cache_threshold", Magick_set_cache_threshold, 1);
    rb_define_module_function(Module_Magick, "set_log_event_mask", Magick_set_log_event_mask, -1);
    rb_define_module_function(Module_Magick, "set_log_format", Magick_set_log_format, 1);

    // Class Magick::Image methods
    Class_Image = rb_define_class_under(Module_Magick, "Image", rb_cObject);

    // Define an alias for Object#display before we override it
    rb_define_alias(Class_Image, "__display__", "display");

    RUBY16(rb_define_singleton_method(Class_Image, "new", Image_new, -1);)
    RUBY16(rb_define_method(Class_Image, "initialize", Image_initialize, 4);)
    RUBY18(rb_define_alloc_func(Class_Image, Image_alloc);)
    RUBY18(rb_define_method(Class_Image, "initialize", Image_initialize, -1);)

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
    rb_define_method(Class_Image, "changed?", Image_changed_q, 0);
    rb_define_method(Class_Image, "channel", Image_channel, 1);
    rb_define_method(Class_Image, "channel_threshold", Image_channel_threshold, -1);
    rb_define_method(Class_Image, "charcoal", Image_charcoal, -1);
    rb_define_method(Class_Image, "chop", Image_chop, 4);
    rb_define_method(Class_Image, "color_flood_fill", Image_color_flood_fill, 5);
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
    rb_define_method(Class_Image, "profile!", Image_profile_bang, 3);
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
    rb_define_method(Class_Image, "to_color", Image_to_color, 1);
    rb_define_method(Class_Image, "to_blob", Image_to_blob, 0);
    rb_define_method(Class_Image, "transparent", Image_transparent, -1);
    rb_define_method(Class_Image, "unsharp_mask", Image_unsharp_mask, 4);
    rb_define_method(Class_Image, "wave", Image_wave, -1);
    rb_define_method(Class_Image, "white_threshold", Image_white_threshold, -1);
    rb_define_method(Class_Image, "write", Image_write, 1);

    // class Magick::ImageList methods (in addition to the methods defined in RMagick.rb)
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

    // class Magick::Draw methods
    Class_Draw = rb_define_class_under(Module_Magick, "Draw", rb_cObject);
    RUBY16(rb_define_singleton_method(Class_Draw, "new", Draw_new, 0);)
    RUBY18(rb_define_alloc_func(Class_Draw, Draw_alloc);)

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

    // Class Magick::ImageList::Montage methods
    Class_Montage = rb_define_class_under(Class_ImageList, "Montage", rb_cObject);

    RUBY16(rb_define_singleton_method(Class_Montage, "new", Montage_new, 0);)
    RUBY18(rb_define_alloc_func(Class_Montage, Montage_alloc));
    
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

    // class Magick::Image::Info methods
    Class_Info = rb_define_class_under(Class_Image, "Info", rb_cObject);


    RUBY16(rb_define_singleton_method(Class_Info, "new", Info_new, 0);)
    RUBY18(rb_define_alloc_func(Class_Info, Info_alloc));

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
//  DCL_ATTR_ACCESSOR(Info, pen) obsolete
//  DCL_ATTR_ACCESSOR(Info, ping)
//  DCL_ATTR_ACCESSOR(Info, pointsize)
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
//  DCL_ATTR_ACCESSOR(Info, verbose)

    // class Magick::GradientFill
    Class_GradientFill = rb_define_class_under(Module_Magick, "GradientFill", rb_cObject);

    RUBY16(rb_define_singleton_method(Class_GradientFill, "new", GradientFill_new, 6);)
    RUBY18(rb_define_alloc_func(Class_GradientFill, GradientFill_alloc));

    rb_define_method(Class_GradientFill, "initialize", GradientFill_initialize, 6);
    rb_define_method(Class_GradientFill, "fill", GradientFill_fill, 1);

    // class Magick::TextureFill
    Class_TextureFill = rb_define_class_under(Module_Magick, "TextureFill", rb_cObject);
    
    RUBY16(rb_define_singleton_method(Class_TextureFill, "new", TextureFill_new, 1);)
    RUBY18(rb_define_alloc_func(Class_TextureFill, TextureFill_alloc));
    
    rb_define_method(Class_TextureFill, "initialize", TextureFill_initialize, 1);
    rb_define_method(Class_TextureFill, "fill", TextureFill_fill, 1);

    // class Magick::ImageMagickError < StandardError
    Class_ImageMagickError = rb_define_class_under(Module_Magick, "ImageMagickError", rb_eStandardError);
    rb_define_method(Class_ImageMagickError, "initialize", ImageMagickError_initialize, 2);
    RUBY16(rb_enable_super(Class_ImageMagickError, "initialize"));
    rb_define_attr(Class_ImageMagickError, MAGICK_LOC, True, False);


    // Miscellaneous constants
    rb_define_const(Module_Magick, "MaxRGB", INT2FIX(MaxRGB));
    rb_define_const(Module_Magick, "QuantumDepth", INT2FIX(QuantumDepth));

    mgk_version = GetMagickVersion(NULL);
    rb_define_const(Module_Magick, "Magick_version", rb_str_new2(mgk_version));

    rb_define_const(Module_Magick, "Version", rb_str_new2(PACKAGE_STRING));

    // AlignType constants
    DEF_CONST(UndefinedAlign);      DEF_CONST(LeftAlign);
    DEF_CONST(CenterAlign);         DEF_CONST(RightAlign); 
    
    // AnchorType constants (for Draw#text_anchor - these are not defined by ImageMagick)
    rb_define_const(Module_Magick, "StartAnchor", INT2FIX(1));
    rb_define_const(Module_Magick, "MiddleAnchor", INT2FIX(2));
    rb_define_const(Module_Magick, "EndAnchor", INT2FIX(3));

    // ChannelType constants
    DEF_CONST(UndefinedChannel);    DEF_CONST(RedChannel);
    DEF_CONST(CyanChannel);         DEF_CONST(GreenChannel);
    DEF_CONST(MagentaChannel);      DEF_CONST(BlueChannel);
    DEF_CONST(YellowChannel);       DEF_CONST(OpacityChannel);
    DEF_CONST(BlackChannel);        DEF_CONST(MatteChannel);

    // ClassType constants
    DEF_CONST(UndefinedClass);      DEF_CONST(PseudoClass);
    DEF_CONST(DirectClass);

    // ColorspaceType constants
    DEF_CONST(UndefinedColorspace); DEF_CONST(RGBColorspace);
    DEF_CONST(GRAYColorspace);      DEF_CONST(TransparentColorspace);
    DEF_CONST(OHTAColorspace);      DEF_CONST(XYZColorspace);
    DEF_CONST(YCbCrColorspace);     DEF_CONST(YCCColorspace);
    DEF_CONST(YIQColorspace);       DEF_CONST(YPbPrColorspace);
    DEF_CONST(YUVColorspace);       DEF_CONST(CMYKColorspace);
    rb_define_const(Module_Magick, "SRGBColorspace", INT2FIX(sRGBColorspace));

    // ComplianceType constants
    // AllCompliance is 0xffff, not too useful for us!
    rb_define_const(Module_Magick, "AllCompliance", INT2FIX(SVGCompliance|X11Compliance|XPMCompliance));
#if HAVE_NOCOMPLIANCE
    DEF_CONST(NoCompliance);
#endif
    DEF_CONST(SVGCompliance);
    DEF_CONST(X11Compliance);
    DEF_CONST(XPMCompliance);

    // CompositeOperator constants
    DEF_CONST(UndefinedCompositeOp);    DEF_CONST(OverCompositeOp);
    DEF_CONST(InCompositeOp);           DEF_CONST(OutCompositeOp);
    DEF_CONST(AtopCompositeOp);         DEF_CONST(XorCompositeOp);
    DEF_CONST(PlusCompositeOp);         DEF_CONST(MinusCompositeOp);
    DEF_CONST(AddCompositeOp);          DEF_CONST(SubtractCompositeOp);
    DEF_CONST(DifferenceCompositeOp);   DEF_CONST(MultiplyCompositeOp);
    DEF_CONST(BumpmapCompositeOp);      DEF_CONST(CopyCompositeOp);
    DEF_CONST(CopyRedCompositeOp);      DEF_CONST(CopyGreenCompositeOp);
    DEF_CONST(CopyBlueCompositeOp);     DEF_CONST(CopyOpacityCompositeOp);
    DEF_CONST(ClearCompositeOp);        DEF_CONST(DissolveCompositeOp);
    DEF_CONST(DisplaceCompositeOp);     DEF_CONST(ModulateCompositeOp);
    DEF_CONST(ThresholdCompositeOp);    DEF_CONST(NoCompositeOp);
    DEF_CONST(DarkenCompositeOp);       DEF_CONST(LightenCompositeOp);
    DEF_CONST(HueCompositeOp);          DEF_CONST(SaturateCompositeOp);
    DEF_CONST(ColorizeCompositeOp);     DEF_CONST(LuminizeCompositeOp);
    DEF_CONST(ScreenCompositeOp);       DEF_CONST(OverlayCompositeOp);

    // CompressionType constants
    DEF_CONST(UndefinedCompression);        DEF_CONST(NoCompression);
    DEF_CONST(BZipCompression);             DEF_CONST(FaxCompression);
    DEF_CONST(Group4Compression);           DEF_CONST(JPEGCompression);
    DEF_CONST(LosslessJPEGCompression);     DEF_CONST(LZWCompression);
    DEF_CONST(RunlengthEncodedCompression); DEF_CONST(ZipCompression);

    // DecorationType constants
    DEF_CONST(NoDecoration);        DEF_CONST(UnderlineDecoration);
    DEF_CONST(OverlineDecoration);  DEF_CONST(LineThroughDecoration);

#if HAVE_DISPOSETYPE
    // DisposeType constants (5.5.1)
    DEF_CONST(UndefinedDispose);    DEF_CONST(BackgroundDispose);
    DEF_CONST(NoneDispose);         DEF_CONST(PreviousDispose);
#endif

    // FilterType constants
    DEF_CONST(UndefinedFilter);     DEF_CONST(PointFilter);
    DEF_CONST(BoxFilter);           DEF_CONST(TriangleFilter);
    DEF_CONST(HermiteFilter);       DEF_CONST(HanningFilter);
    DEF_CONST(HammingFilter);       DEF_CONST(BlackmanFilter);
    DEF_CONST(GaussianFilter);      DEF_CONST(QuadraticFilter);
    DEF_CONST(CubicFilter);         DEF_CONST(CatromFilter);
    DEF_CONST(MitchellFilter);      DEF_CONST(LanczosFilter);
    DEF_CONST(BesselFilter);        DEF_CONST(SincFilter);

    // GravityType constants
    DEF_CONST(ForgetGravity);       DEF_CONST(NorthWestGravity);
    DEF_CONST(NorthGravity);        DEF_CONST(NorthEastGravity);
    DEF_CONST(WestGravity);         DEF_CONST(CenterGravity);
    DEF_CONST(EastGravity);         DEF_CONST(SouthWestGravity);
    DEF_CONST(SouthGravity);        DEF_CONST(SouthEastGravity);
    DEF_CONST(StaticGravity);

    // ImageType constants
    DEF_CONST(UndefinedType);       DEF_CONST(BilevelType);
    DEF_CONST(GrayscaleType);       DEF_CONST(GrayscaleMatteType);
    DEF_CONST(PaletteType);         DEF_CONST(PaletteMatteType);
    DEF_CONST(TrueColorType);       DEF_CONST(TrueColorMatteType);
    DEF_CONST(ColorSeparationType); DEF_CONST(ColorSeparationMatteType);
    DEF_CONST(OptimizeType);

    // InterlaceType constants
    DEF_CONST(UndefinedInterlace);  DEF_CONST(NoInterlace);
    DEF_CONST(LineInterlace);       DEF_CONST(PlaneInterlace);
    DEF_CONST(PartitionInterlace);

    // NoiseType constants
    DEF_CONST(UniformNoise);                DEF_CONST(GaussianNoise);
    DEF_CONST(MultiplicativeGaussianNoise); DEF_CONST(ImpulseNoise);
    DEF_CONST(LaplacianNoise);              DEF_CONST(PoissonNoise);

    // Opacity constants
    DEF_CONST(OpaqueOpacity);       DEF_CONST(TransparentOpacity);

    // Paint method constants
    DEF_CONST(PointMethod);         DEF_CONST(ReplaceMethod);
    DEF_CONST(FloodfillMethod);     DEF_CONST(FillToBorderMethod);
    DEF_CONST(ResetMethod);

    // RenderingIntent
    DEF_CONST(UndefinedIntent);     DEF_CONST(SaturationIntent);
    DEF_CONST(PerceptualIntent);    DEF_CONST(AbsoluteIntent);
    DEF_CONST(RelativeIntent);

    // ResolutionType constants
    DEF_CONST(UndefinedResolution); DEF_CONST(PixelsPerInchResolution);
    DEF_CONST(PixelsPerCentimeterResolution);

    // StretchType constants
    DEF_CONST(NormalStretch);           DEF_CONST(UltraCondensedStretch);
    DEF_CONST(ExtraCondensedStretch);   DEF_CONST(CondensedStretch);
    DEF_CONST(SemiCondensedStretch);    DEF_CONST(SemiExpandedStretch);
    DEF_CONST(ExpandedStretch);         DEF_CONST(ExtraExpandedStretch);
    DEF_CONST(UltraExpandedStretch);    DEF_CONST(AnyStretch);

    // StyleType constants
    DEF_CONST(NormalStyle);             DEF_CONST(ItalicStyle);
    DEF_CONST(ObliqueStyle);            DEF_CONST(AnyStyle);

    // WeightType constants
    DEF_CONST(AnyWeight);           DEF_CONST(NormalWeight);
    DEF_CONST(BoldWeight);          DEF_CONST(BolderWeight);
    DEF_CONST(LighterWeight);

    // Struct class constants - pass NULL as the structure name to
    // keep from polluting the Struct namespace. The only way to
    // use these classes is via the Magick:: namespace.

    // Magick::AffineMatrix
    Class_AffineMatrix = rb_struct_define(NULL, "sx", "rx", "ry", "sy", "tx", "ty", 0);
    rb_define_const(Module_Magick, "AffineMatrix", Class_AffineMatrix);

    // Magick::Pixel has 3 constructors: "new" "from_color", and "from_HSL".
    Class_Pixel = rb_struct_define(NULL, "red", "green", "blue", "opacity", 0);
    rb_define_singleton_method(Class_Pixel, "from_color", Pixel_from_color, 1);
    rb_define_singleton_method(Class_Pixel, "from_HSL", Pixel_from_HSL, 1);
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

    new_ID = rb_intern("new");
    push_ID = rb_intern("push");
    length_ID = rb_intern("length");
    cur_image_ID = rb_intern("cur_image");
    values_ID = rb_intern("values");
    _dummy_img__ID = rb_intern("_dummy_img_");
}
