/* $Id: rminfo.c,v 1.36 2006/01/18 00:23:03 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2006 by Timothy P. Hunter
| Name:     rminfo.c
| Author:   Tim Hunter
| Purpose:  Info class method definitions for RMagick.
\============================================================================*/

#include "rmagick.h"

static VALUE get_option(VALUE, char *);
static VALUE set_option(VALUE, char *, VALUE);


DEF_ATTR_ACCESSOR(Info, antialias, bool)

/*
 * Method:  value = Info[format, key]
 * Purpose: get the value of an option set by Info[]=
*/
#define MAX_FORMAT_LEN 60

VALUE
Info_aref(VALUE self, VALUE format, VALUE key)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    char *format_p, *key_p;
    long format_l, key_l;
    const char *value;
    char fkey[MaxTextExtent];

    format_p = STRING_PTR_LEN(format, format_l);
    key_p = STRING_PTR_LEN(key, key_l);

    if (format_l > MAX_FORMAT_LEN || format_l + key_l > MaxTextExtent-1)
    {
        rb_raise(rb_eArgError, "can't reference %.60s:%.1024s - too long", format_p, key_p);
    }

    sprintf(fkey, "%.60s:%.*s", STRING_PTR(format), (int)(MaxTextExtent-61), STRING_PTR(key));

    Data_Get_Struct(self, Info, info);
    value = GetImageOption(info, fkey);
    if (!value)
    {
        return Qnil;
    }

    return rb_str_new2(value);

#elif defined(HAVE_ADDDEFINITIONS)
    Info *info;
    const char *value;

    Data_Get_Struct(self, Info, info);
    value = AccessDefinition(info, STRING_PTR(format), STRING_PTR(key));

    if (!value)
    {
        return Qnil;
    }

    return rb_str_new2(value);
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}


/*
    Method:     Info[format, key] = value
    Purpose:    Call AddDefinitions (GM) or SetImageOption (IM)
    Note:       Essentially the same function as Info#define but paired
                with Info#[]=
*/
VALUE
Info_aset(VALUE self, VALUE format, VALUE key, VALUE value)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    char *format_p, *key_p, *value_p = "";
    long format_l, key_l;
    char ckey[MaxTextExtent];
    unsigned int okay;


    Data_Get_Struct(self, Info, info);

    format_p = STRING_PTR_LEN(format, format_l);
    key_p = STRING_PTR_LEN(key, key_l);

    /* Allow any argument that supports to_s */
    value = rb_funcall(value, ID_to_s, 0);
    value_p = STRING_PTR(value);

    if (format_l > MAX_FORMAT_LEN || format_l+key_l > MaxTextExtent-1)
    {
        rb_raise(rb_eArgError, "%.60s:%.1024s not defined - too long", format_p, key_p);
    }

    (void) sprintf(ckey, "%.60s:%.*s", format_p, (int)(sizeof(ckey)-MAX_FORMAT_LEN), key_p);

    okay = SetImageOption(info, ckey, value_p);
    if (!okay)
    {
        rb_warn("%.60s:%.1024s not defined - SetImageOption failed.", format_p, key_p);
        return Qnil;
    }

    return self;

#elif defined(HAVE_ADDDEFINITIONS)
    Info *info;
    char *format_p, *key_p, *value_p = NULL;
    long format_l, key_l, value_l = 0;
    unsigned int okay;
    ExceptionInfo exception;
    char definitions[MaxTextExtent*2];/* Make this buffer longer than the buffer used   */
                                      /* for SetImageOptions since AddDefinitions cats  */
                                      /* the value onto the format:key pair.            */

    Data_Get_Struct(self, Info, info);

    format_p = STRING_PTR_LEN(format, format_l);
    key_p = STRING_PTR_LEN(key, key_l);
    value = rb_funcall(value, ID_to_s, 0);
    value_p = STRING_PTR_LEN(value, value_l);

    if ((3 + format_l + key_l + value_l) > sizeof(definitions))
    {
        rb_raise(rb_eArgError, "%.60s:%.1024s not defined - too long", format_p, key_p);
    }
    (void)sprintf(definitions, "%s:%s=", format_p, key_p);
    if (value_l > 0)
    {
        strcat(definitions, value_p);
    }

    GetExceptionInfo(&exception);
    okay = AddDefinitions(info, definitions, &exception);
    HANDLE_ERROR
    if (!okay)
    {
        rb_warn("%.60s:%.1024s not defined - AddDefinitions failed.", format_p, key_p);
        return Qnil;
    }

    return self;

#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}


VALUE
Info_authenticate(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    if (info->authenticate)
    {
        return rb_str_new2(info->authenticate);
    }
    else
    {
        return Qnil;
    }
}


VALUE
Info_authenticate_eq(VALUE self, VALUE passwd)
{
    Info *info;
    char *passwd_p = NULL;
    long passwd_len = 0;

    Data_Get_Struct(self, Info, info);

    if (!NIL_P(passwd))
    {
        passwd_p = STRING_PTR_LEN(passwd, passwd_len);
    }

    if (info->authenticate)
    {
        magick_free(info->authenticate);
        info->authenticate = NULL;
    }
    if (passwd_len > 0)
    {
        magick_clone_string(&info->authenticate, passwd_p);
    }

    return self;
}


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

/*
    Method:     Info#colorspace
    Purpose:    Get the colorspace type
*/
VALUE
Info_colorspace(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return ColorspaceType_new(info->colorspace);
}

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
    VALUE_TO_ENUM(colorspace, info->colorspace, ColorspaceType);
    return self;
}

OPTION_ATTR_ACCESSOR(comment, Comment)

/*
    Method:  Info#compression
    Purpose: Get the compression type
    Notes:
*/
VALUE
Info_compression(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return CompressionType_new(info->compression);
}

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
    VALUE_TO_ENUM(type, info->compression, CompressionType);
    return self;
}

/*
    Method:     Info#define(format, key[, value])
    Purpose:    Call AddDefinitions (GM) or SetImageOption (IM)
    Note:       The only method in Info that is not an
                attribute accessor.
*/
VALUE
Info_define(int argc, VALUE *argv, VALUE self)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    char *format, *key, *value = "";
    long format_l, key_l;
    char ckey[100];
    unsigned int okay;
    volatile VALUE fmt_arg;

    Data_Get_Struct(self, Info, info);

    switch(argc)
    {
        case 3:
            /* Allow any argument that supports to_s */
            fmt_arg = rb_funcall(argv[2], ID_to_s, 0);
            value = STRING_PTR(fmt_arg);
        case 2:
            key = STRING_PTR_LEN(argv[1], key_l);
            format = STRING_PTR_LEN(argv[0], format_l);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
    }

    if (2 + format_l + key_l > sizeof(ckey))
    {
        rb_raise(rb_eArgError, "%.20s:%.20s not defined - format or key too long", format, key);
    }
    (void) sprintf(ckey, "%s:%s", format, key);

    okay = SetImageOption(info, ckey, value);
    if (!okay)
    {
        rb_warn("%.20s=\"%.78s\" not defined - SetImageOption failed.", ckey, value);
        return Qnil;
    }

    return self;

#elif defined(HAVE_ADDDEFINITIONS)
    Info *info;
    char *format, *key, *value = NULL;
    long format_l, key_l, value_l = 0;
    unsigned int okay;
    volatile VALUE fmt_arg;
    ExceptionInfo exception;
    char definitions[200];      /* Make this buffer longer than the buffer used   */
                                /* for SetImageOptions since AddDefinitions cats  */
                                /* the value onto the format:key pair.            */

    Data_Get_Struct(self, Info, info);

    switch(argc)
    {
        case 3:
            /* Allow any argument that supports to_s */
            fmt_arg = rb_funcall(argv[2], ID_to_s, 0);
            value = STRING_PTR_LEN(fmt_arg, value_l);
            /* Fall through */
        case 2:
            key = STRING_PTR_LEN(argv[1], key_l);
            format = STRING_PTR_LEN(argv[0], format_l);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
            break;
    }


    if ((3 + format_l + key_l + value_l) > sizeof(definitions))
    {
        rb_raise(rb_eArgError, "%.20s:%.20s not defined - too long", format, key);
    }
    (void)sprintf(definitions, "%s:%s=", format, key);
    if (value)
    {
        strcat(definitions, value);
    }

    GetExceptionInfo(&exception);
    okay = AddDefinitions(info, definitions, &exception);
    HANDLE_ERROR
    if (!okay)
    {
        rb_warn("%.*s not defined - AddDefinitions failed.", sizeof(definitions), definitions);
        return Qnil;
    }

    return self;

#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

/*
    Method:     Info#delay
    Purpose:    Get the delay attribute
    Notes:      Convert from string to numeric
*/
VALUE
Info_delay(VALUE self)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    const char *delay;
    char *p;
    long d;

    Data_Get_Struct(self, Info, info);

    delay = GetImageOption(info, "delay");
    if (delay)
    {
        d = strtol(delay, &p, 10);
        if (*p != '\0')
        {
            rb_raise(rb_eRangeError, "failed to convert %s to Numeric", delay);
        }
        return LONG2NUM(d);
    }
    return Qnil;
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

/*
 * Will raise an exception if `arg' can't be converted to an int.
*/
#if defined(HAVE_SETIMAGEOPTION)
static VALUE
arg_is_integer(VALUE arg)
{
    double d;
    d = NUM2INT(arg);
    d = d;      // satisfy icc
    return arg;
}
#endif

/*
    Method:     Info#delay=
    Purpose:    Set the delay attribute
    Notes:      Convert from numeric value to string.
*/
VALUE
Info_delay_eq(VALUE self, VALUE string)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    int delay;
    int not_num;
    char dstr[20];

    Data_Get_Struct(self, Info, info);

    if (NIL_P(string))
    {
        (void) RemoveImageOption(info, "delay");
    }
    else
    {
        not_num = 0;
        rb_protect(arg_is_integer, string, &not_num);
        if (not_num)
        {
            rb_raise(rb_eTypeError, "failed to convert %s into Integer", rb_class2name(CLASS_OF(string)));
        }
        delay = NUM2INT(string);
        sprintf(dstr, "%d", delay);
        (void) SetImageOption(info, "delay", dstr);
    }
    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

DEF_ATTR_READER(Info, density, str)

/*
    Method:     Info#density=<aString>
    Purpose:    Set the text rendering density, e.g. "72x72"
    Raise:      ArgumentError
*/
VALUE
Info_density_eq(VALUE self, VALUE density_arg)
{
    Info *info;
    volatile VALUE density;
    char *dens;

    Data_Get_Struct(self, Info, info);

    if(NIL_P(density_arg))
    {
        magick_free(info->density);
        info->density = NULL;
        return self;
    }

    density = rb_funcall(density_arg, ID_to_s, 0);
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

/*
    Method:     Info#dispose
    Purpose:    Retrieve a dispose option string and convert it to
                a DisposeType enumerator
*/
#if defined(HAVE_SETIMAGEOPTION)
static struct
{
    char *string;
    char *enum_name;
    DisposeType enumerator;
} Dispose_Option[] = {
    { "Background", "BackgroundDispose", BackgroundDispose },
    { "None",       "NoneDispose",       NoneDispose },
    { "Previous",   "PreviousDispose",   PreviousDispose },
    { "Undefined",  "UndefinedDispose",  UndefinedDispose },
    { "0",          "UndefinedDispose",  UndefinedDispose },
    { "1",          "NoneDispose",       NoneDispose },
    { "2",          "BackgroundDispose", BackgroundDispose },
    { "3",          "PreviousDispose",   PreviousDispose },
    };
#define N_DISPOSE_OPTIONS (sizeof(Dispose_Option)/sizeof(Dispose_Option[0]))
#endif

VALUE
Info_dispose(VALUE self)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    int x;
    ID dispose_id;
    const char *dispose;

    Data_Get_Struct(self, Info, info);

    dispose_id = rb_intern("UndefinedDispose");

    // Map the dispose option string to a DisposeType enumerator.
    dispose=GetImageOption(info, "dispose");
    if (dispose)
    {
        for (x = 0; x < N_DISPOSE_OPTIONS; x++)
        {
            if (strcmp(dispose, Dispose_Option[x].string) == 0)
            {
                dispose_id = rb_intern(Dispose_Option[x].enum_name);
                break;
            }
        }
    }

    return rb_const_get(Module_Magick, dispose_id);
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

/*
    Method:     Info#dispose=
    Purpose:    Convert a DisposeType enumerator into the equivalent
                dispose option string
*/
VALUE
Info_dispose_eq(VALUE self, VALUE disp)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    DisposeType dispose;
    char *option;
    int x;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(disp))
    {
        (void) RemoveImageOption(info, "dispose");
        return self;
    }

    VALUE_TO_ENUM(disp, dispose, DisposeType);
    option = "Undefined";

    for(x = 0; x < N_DISPOSE_OPTIONS; x++)
    {
        if (dispose == Dispose_Option[x].enumerator)
        {
            option = Dispose_Option[x].string;
            break;
        }
    }

    (void) SetImageOption(info, "dispose", option);
    return self;

#else
    rm_not_implemented();
    return (VALUE)0;
#endif
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
Info_extract_eq(VALUE self, VALUE extract_arg)
{
    Info *info;
    char *extr;
    volatile VALUE extract;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(extract_arg))
    {
        magick_free(info->extract);
        info->extract = NULL;
        return self;
    }

    extract = rb_funcall(extract_arg, ID_to_s, 0);
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
    rm_not_implemented();
    return (VALUE)0;
}
VALUE
Info_extract_eq(VALUE self, VALUE extr)
{
    rm_not_implemented();
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
Info_tile_eq(VALUE self, VALUE tile_arg)
{
    Info *info;
    char *til;
    volatile VALUE tile;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(tile_arg))
    {
        magick_free(info->tile);
        info->tile = NULL;
        return self;
    }

    tile = rb_funcall(tile_arg, ID_to_s, 0);
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
    Method:     Info#fill
    Purpose:    return the fill (a.k.a pen) color as a String
    Note:       Compare with Image#fill!
*/
VALUE
Info_fill(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return PixelPacket_to_Color_Name_Info(info, &info->pen);
}

/*
    Method:     Info#fill=<aString>
    Purpose:    set the fill (a.k.a. pen) color
    Raises:     ArgumentError
*/
VALUE
Info_fill_eq(VALUE self, VALUE color)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->pen, color);
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

DEF_ATTR_READER(Info, fuzz, dbl)

/*
    Method:     Info#fuzz=number
                Info#fuzz=NN%
    Notes:      See Image#fuzz
*/
VALUE Info_fuzz_eq(VALUE self, VALUE fuzz)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    info->fuzz = rm_fuzz_to_dbl(fuzz);
    return self;
}

/*
    Method:     Info#gravity
    Purpose:    Return the value of the gravity option as a GravityType enumerator
*/

#if defined(HAVE_SETIMAGEOPTION)
static struct
{
    char *string;
    char *enum_name;
    GravityType enumerator;
} Gravity_Option[] = {
    { "Undefined",  "UndefinedGravity", UndefinedGravity },
    { "None",       "UndefinedGravity", UndefinedGravity },
    { "Center",     "CenterGravity",    CenterGravity },
    { "East",       "EastGravity",      EastGravity},
    { "Forget",     "ForgetGravity",    ForgetGravity },
    { "NorthEast",  "NorthEastGravity", NorthEastGravity },
    { "North",      "NorthGravity",     NorthGravity },
    { "NorthWest",  "NorthWestGravity", NorthWestGravity },
    { "SouthEast",  "SouthEastGravity", SouthEastGravity },
    { "South",      "SouthGravity",     SouthGravity },
    { "SouthWest",  "SouthWestGravity", SouthWestGravity },
    { "West",       "WestGravity",      WestGravity },
    { "Static",     "StaticGravity",    StaticGravity }
    };
#define N_GRAVITY_OPTIONS (sizeof(Gravity_Option)/sizeof(Gravity_Option[0]))
#endif

VALUE Info_gravity(VALUE self)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    const char *gravity;
    int x;
    ID gravity_id;

    Data_Get_Struct(self, Info, info);

    gravity_id = rb_intern("UndefinedGravity");

    // Map the gravity option string to a GravityType enumerator.
    gravity=GetImageOption(info, "gravity");
    if (gravity)
    {
        for (x = 0; x < N_GRAVITY_OPTIONS; x++)
        {
            if (strcmp(gravity, Gravity_Option[x].string) == 0)
            {
                gravity_id = rb_intern(Gravity_Option[x].enum_name);
                break;
            }
        }
    }

    return rb_const_get(Module_Magick, gravity_id);

#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

/*
    Method:     Info#gravity=
    Purpose:    Convert a GravityType enum to a gravity option name and
                store in the Info structure
*/
VALUE
Info_gravity_eq(VALUE self, VALUE grav)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    GravityType gravity;
    char *option;
    int x;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(grav))
    {
        (void) RemoveImageOption(info, "gravity");
        return self;
    }

    VALUE_TO_ENUM(grav, gravity, GravityType);
    option = "Undefined";

    for(x = 0; x < N_GRAVITY_OPTIONS; x++)
    {
        if (gravity == Gravity_Option[x].enumerator)
        {
            option = Gravity_Option[x].string;
            break;
        }
    }

    (void) SetImageOption(info, "gravity", option);
    return self;

#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}


DEF_ATTR_ACCESSOR(Info, group, long)

/*
    Method:     Info#image_type
    Purpose:    Get the classification type
    Raises:     ArgumentError
*/
VALUE
Info_image_type(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return ImageType_new(info->type);
}

/*
    Method:     Info#image_type=
    Purpose:    Set the classification type
    Raises:     ArgumentError
*/
VALUE
Info_image_type_eq(VALUE self, VALUE type)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(type, info->type, ImageType);
    return self;
}

/*
    Method:  Info#interlace
    Purpose: Get the interlace type
    Notes:
*/
VALUE
Info_interlace(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return InterlaceType_new(info->interlace);
}

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
    VALUE_TO_ENUM(inter, info->interlace, InterlaceType);
    return self;
}

OPTION_ATTR_ACCESSOR(label, Label)

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

/*
    Method:     Info#monitor=
    Purpose:    Establish a progress monitor
    Notes:      See Image_monitor_eq
*/
VALUE
Info_monitor_eq(VALUE self, VALUE monitor)
{
#if defined(HAVE_SETIMAGEPROGRESSMONITOR)
    Info *info;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(monitor))
    {
        info->progress_monitor = NULL;
    }
    else
    {
        (void) SetImageInfoProgressMonitor(info, rm_progress_monitor, (void *)monitor);
    }


    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
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

/*
    Method:     Info#orientation
    Purpose:    Return the orientation attribute as an OrientationType enum value.
*/
VALUE
Info_orientation(VALUE self)
{
#if defined(HAVE_IMAGEINFO_ORIENTATION)
    Info *info;

    Data_Get_Struct(self, Info, info);
    return OrientationType_new(info->orientation);
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}


/*
    Method:     Info#Orientation=
    Purpose:    Set the Orientation type
    Raises:     ArgumentError
*/
VALUE
Info_orientation_eq(VALUE self, VALUE inter)
{
#if defined(HAVE_IMAGEINFO_ORIENTATION)
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(inter, info->orientation, OrientationType);
    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

VALUE
Info_page(VALUE self)
{
    Info *info;
    const char *page;

    Data_Get_Struct(self, Info, info);
#if defined(HAVE_SETIMAGEOPTION)

    page = GetImageOption(info, "page");
#else
    page = (const char *)info->page;
#endif
    return page ? rb_str_new2(page) : Qnil;

}

/*
    Method:     Info#page=<aString> or <aGeometry>
    Purpose:    store the Postscript page geometry
*/
VALUE
Info_page_eq(VALUE self, VALUE page_arg)
{
    Info *info;
    volatile VALUE geom_str;
    char *geometry;

    Data_Get_Struct(self, Info, info);
    if (NIL_P(page_arg))
    {
        magick_free(info->page);
        info->page = NULL;
        return self;
    }
    geom_str = rb_funcall(page_arg, ID_to_s, 0);
    geometry=GetPageGeometry(STRING_PTR(geom_str));
    if (*geometry == '\0')
    {
        magick_free(info->page);
        info->page = NULL;
        return self;
    }
    magick_clone_string(&info->page, geometry);
#if defined(HAVE_SETIMAGEOPTION)
    (void) SetImageOption(info, "page", STRING_PTR(geom_str));
#endif

    return self;
}

DEF_ATTR_ACCESSOR(Info, pointsize, dbl)
DEF_ATTR_ACCESSOR(Info, quality, long)

/*
    Method:     Info#sampling_factor, #sampling_factor=
    Purpose:    get/set sampling factors used by JPEG or MPEG-2 encoder
                and YUV decoder/encoder
*/
VALUE
Info_sampling_factor(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    if (info->sampling_factor)
    {
        return rb_str_new2(info->sampling_factor);
    }
    else
    {
        return Qnil;
    }
}

VALUE
Info_sampling_factor_eq(VALUE self, VALUE sampling_factor)
{
    Info *info;
    char *sampling_factor_p = NULL;
    long sampling_factor_len = 0;

    Data_Get_Struct(self, Info, info);

    if (!NIL_P(sampling_factor))
    {
        sampling_factor_p = STRING_PTR_LEN(sampling_factor, sampling_factor_len);
    }

    if (info->sampling_factor)
    {
        magick_free(info->sampling_factor);
        info->sampling_factor = NULL;
    }
    if (sampling_factor_len > 0)
    {
        magick_clone_string(&info->sampling_factor, sampling_factor_p);
    }

    return self;
}

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
                Info#size=<aGeometry>
    Purpose:    Set the size (a Geometry string, i.e. WxH{+-}x{+-}y)
    Raises:     ArgumentError
*/
VALUE
Info_size_eq(VALUE self, VALUE size_arg)
{
    Info *info;
    volatile VALUE size;
    char *sz;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(size_arg))
    {
        magick_free(info->size);
        info->size = NULL;
        return self;
    }

    size = rb_funcall(size_arg, ID_to_s, 0);
    sz = STRING_PTR(size);
    if (!IsGeometry(sz))
    {
        rb_raise(rb_eArgError, "invalid size geometry: %s", sz);
    }

    magick_clone_string(&info->size, sz);

    return self;
}


/*
    Method:     Info#undefine
    Purpose:    Undefine image option
*/

VALUE
Info_undefine(VALUE self, VALUE format, VALUE key)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    char *format_p, *key_p;
    long format_l, key_l;
    char fkey[MaxTextExtent];

    format_p = STRING_PTR_LEN(format, format_l);
    key_p = STRING_PTR_LEN(key, key_l);

    if (format_l > MAX_FORMAT_LEN || format_l + key_l > MaxTextExtent)
    {
        rb_raise(rb_eArgError, "can't undefine %.60s:%.1024s - too long", format_p, key_p);
    }

    sprintf(fkey, "%.60s:%.*s", format_p, (int)(MaxTextExtent-61), key_p);

    Data_Get_Struct(self, Info, info);
    /* Depending on the IM version, RemoveImageOption returns either */
    /* char * or MagickBooleanType. Ignore the return value.         */
    (void) RemoveImageOption(info, fkey);
    return self;

#elif defined(HAVE_ADDDEFINITIONS)
    Info *info;
    unsigned int okay;
    char *format_p, *key_p;
    long format_l, key_l;
    char fkey[MaxTextExtent];

    format_p = STRING_PTR_LEN(format, format_l);
    key_p = STRING_PTR_LEN(key, key_l);

    if (format_l > MAX_FORMAT_LEN || format_l + key_l > MaxTextExtent)
    {
        rb_raise(rb_eArgError, "can't undefine %.60s:%.1024s - too long", format_p, key_p);
    }

    sprintf(fkey, "%.60s:%.*s", format_p, MaxTextExtent-61, key_p);

    Data_Get_Struct(self, Info, info);
    okay = RemoveDefinitions(info, fkey);
    if (!okay)
    {
        rb_raise(rb_eArgError, "no such key: `%s'", fkey);
    }
    return self;

#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

/*
    Method:  Info#units
    Purpose: Get the resolution type
*/
VALUE
Info_units(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return ResolutionType_new(info->units);
}

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
    VALUE_TO_ENUM(units, info->units, ResolutionType);
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

/*
    Method:     Info.new
    Purpose:    Create an Info object by calling CloneInfo
*/
#if !defined(HAVE_RB_DEFINE_ALLOC_FUNC)
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


/*
    Method:     Info#get_option
    Purpose:    Return the value of the specified option
*/
static VALUE
get_option(VALUE self, char *key)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    const char *value;

    Data_Get_Struct(self, Info, info);

    value = GetImageOption(info, key);
    if (value)
    {
        return rb_str_new2(value);
    }
    return Qnil;
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

/*
    Method:     Info#set_option
    Purpose:    Set the specified option to this value.
                If the value is nil just unset any current value
*/
static VALUE
set_option(VALUE self, char *key, VALUE string)
{
#if defined(HAVE_SETIMAGEOPTION)
    Info *info;
    char *value;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(string))
    {
        (void) RemoveImageOption(info, key);
    }
    else
    {
        value = STRING_PTR(string);
        (void) SetImageOption(info, key, value);
    }
    return self;
#else
    rm_not_implemented();
    return (VALUE)0;
#endif
}

