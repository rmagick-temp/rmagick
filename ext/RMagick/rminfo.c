/* $Id: rminfo.c,v 1.72 2008/08/26 22:36:15 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2008 by Timothy P. Hunter
| Name:     rminfo.c
| Author:   Tim Hunter
| Purpose:  Info class method definitions for RMagick.
\============================================================================*/

#include "rmagick.h"





/*
    Method:     Info#get_option
    Purpose:    Return the value of the specified option
*/
static VALUE
get_option(VALUE self, const char *key)
{
    Info *info;
    const char *value;

    Data_Get_Struct(self, Info, info);

    value = GetImageOption(info, key);
    if (value)
    {
        return rb_str_new2(value);
    }
    return Qnil;
}

/*
    Method:     Info#set_option
    Purpose:    Set the specified option to this value.
                If the value is nil just unset any current value
*/
static VALUE
set_option(VALUE self, const char *key, VALUE string)
{
    Info *info;
    char *value;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(string))
    {
        (void) RemoveImageOption(info, key);
    }
    else
    {
        value = StringValuePtr(string);
        (void) SetImageOption(info, key, value);
    }
    return self;
}


/*
    Static:     set_color_option
    Purpose:    Set a color name as the value of the specified option
    Note:       Call QueryColorDatabase to validate color name
*/
static VALUE set_color_option(VALUE self, const char *option, VALUE color)
{
    Info *info;
    char *name;
    PixelPacket pp;
    ExceptionInfo exception;
    MagickBooleanType okay;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(color))
    {
        (void) RemoveImageOption(info, option);
    }
    else
    {
        GetExceptionInfo(&exception);
        name = StringValuePtr(color);
        okay = QueryColorDatabase(name, &pp, &exception);
        (void) DestroyExceptionInfo(&exception);
        if (!okay)
        {
            rb_raise(rb_eArgError, "invalid color name `%s'", name);
        }

        (void) RemoveImageOption(info, option);
        (void) SetImageOption(info, option, name);
    }

    return self;
}


/*
    Static:     get_dbl_option(obj, option)
    Purpose:    Get an Image::Info option floating-point value
    Notes:      Convert the string value to a float
*/
static VALUE get_dbl_option(VALUE self, const char *option)
{
    Info *info;
    const char *value;
    double d;
    long n;

    Data_Get_Struct(self, Info, info);

    value = GetImageOption(info, option);
    if (!value)
    {
        return Qnil;
    }

    d = atof(value);
    n = (long) floor(d);
    return d == (double)n ? LONG2NUM(n) : rb_float_new(d);
}


/*
    Static:     set_dbl_option(obj, option, value)
    Purpose:    Set an Image::Info option to a floating-point value
    Notes:      SetImageOption expects the value to be a string.
*/
static VALUE set_dbl_option(VALUE self, const char *option, VALUE value)
{
    Info *info;
    char buff[50];
    double d;
    long n;
    int len;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(value))
    {
        (void) RemoveImageOption(info, option);
    }
    else
    {
        d = NUM2DBL(value);
        n = floor(d);
        if (d == n)
        {
            len = sprintf(buff, "%-10ld", n);
        }
        else
        {
            len = sprintf(buff, "%-10.2f", d);
        }
        memset(buff+len, '\0', sizeof(buff)-len);
        (void) RemoveImageOption(info, option);
        (void) SetImageOption(info, option, buff);
    }

    return self;
}


DEF_ATTR_ACCESSOR(Info, antialias, bool)

/*
   Method:  value = Info[format, key]
            value = Info[key]
   Purpose: get the value of an option set by Info[]=
            The 2 argument form is the original form. Added support for a
            single argument after ImageMagick started using Set/GetImageOption
            for options that aren't represented by fields in the ImageInfo
            structure.
*/
#define MAX_FORMAT_LEN 60

VALUE
Info_aref(int argc, VALUE *argv, VALUE self)
{
    Info *info;
    char *format_p, *key_p;
    long format_l, key_l;
    const char *value;
    char fkey[MaxTextExtent];

    switch (argc)
    {
        case 2:
            format_p = rm_str2cstr(argv[0], &format_l);
            key_p = rm_str2cstr(argv[1], &key_l);
            if (format_l > MAX_FORMAT_LEN || format_l + key_l > MaxTextExtent-1)
            {
                rb_raise(rb_eArgError, "can't reference %.60s:%.1024s - too long", format_p, key_p);
            }

            sprintf(fkey, "%.60s:%.*s", format_p, (int)(MaxTextExtent-61), key_p);
            break;

        case 1:
            strncpy(fkey, StringValuePtr(argv[0]), sizeof(fkey)-1);
            fkey[sizeof(fkey)-1] = '\0';
            break;

        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 or 2)", argc);
            break;

    }

    Data_Get_Struct(self, Info, info);
    value = GetImageOption(info, fkey);
    if (!value)
    {
        return Qnil;
    }

    return rb_str_new2(value);
}


/*
    Method:     Info[format, key] = value
    Purpose:    Call SetImageOption
    Note:       Essentially the same function as Info#define but paired with Info#[]=
                If the value is nil it is equivalent to #undefine.

                The 2 argument form is the original form. Added support for a
                single argument after ImageMagick started using Set/GetImageOption
                for options that aren't represented by fields in the ImageInfo
                structure.
*/
VALUE
Info_aset(int argc, VALUE *argv, VALUE self)
{
    Info *info;
    volatile VALUE value;
    char *format_p, *key_p, *value_p = NULL;
    long format_l, key_l;
    char ckey[MaxTextExtent];
    unsigned int okay;


    Data_Get_Struct(self, Info, info);

    switch (argc)
    {
        case 3:
            format_p = rm_str2cstr(argv[0], &format_l);
            key_p = rm_str2cstr(argv[1], &key_l);

            if (format_l > MAX_FORMAT_LEN || format_l+key_l > MaxTextExtent-1)
            {
                rb_raise(rb_eArgError, "%.60s:%.1024s not defined - too long", format_p, key_p);
            }

            (void) sprintf(ckey, "%.60s:%.*s", format_p, (int)(sizeof(ckey)-MAX_FORMAT_LEN), key_p);

            value = argv[2];
            break;

        case 2:
            strncpy(ckey, StringValuePtr(argv[0]), sizeof(ckey)-1);
            ckey[sizeof(ckey)-1] = '\0';

            value = argv[1];
            break;

        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
            break;
    }

    if (NIL_P(value))
    {
        (void) RemoveImageOption(info, ckey);
    }
    else
    {
        /* Allow any argument that supports to_s */
        value = rm_to_s(value);
        value_p = StringValuePtr(value);

        (void) RemoveImageOption(info, ckey);
        okay = SetImageOption(info, ckey, value_p);
        if (!okay)
        {
            rb_warn("`%s' not defined - SetImageOption failed.", ckey);
            return Qnil;
        }
    }


    return self;
}


VALUE
Info_attenuate(VALUE self)
{
    return get_dbl_option(self, "attenuate");
}


VALUE
Info_attenuate_eq(VALUE self, VALUE value)
{
    return set_dbl_option(self, "attenuate", value);
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
    long passwd_l = 0;

    Data_Get_Struct(self, Info, info);

    if (!NIL_P(passwd))
    {
        passwd_p = rm_str2cstr(passwd, &passwd_l);
    }

    if (info->authenticate)
    {
        magick_free(info->authenticate);
        info->authenticate = NULL;
    }
    if (passwd_l > 0)
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
    Method:     Info#caption=<aString>
    Purpose:    emulate the -caption option
*/
VALUE
Info_caption(VALUE self)
{
    return get_option(self, "caption");
}



VALUE
Info_caption_eq(VALUE self, VALUE caption)
{
    return set_option(self, "caption", caption);
}


/*
    Method:     Info#channel(channel [, channel...])
    Purpose:    Set the channels
    Thanks:     Douglas Sellers
*/
VALUE
Info_channel(int argc, VALUE *argv, VALUE self)
{
    Info *info;
    ChannelType channels;

    channels = extract_channels(&argc, argv);

    // Ensure all arguments consumed.
    if (argc > 0)
    {
        raise_ChannelType_error(argv[argc-1]);
    }

    Data_Get_Struct(self, Info, info);

    info->channel = channels;
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
    Purpose:    Call SetImageOption
    Note:       The only method in Info that is not an
                attribute accessor.
*/
VALUE
Info_define(int argc, VALUE *argv, VALUE self)
{
    Info *info;
    char *format, *key;
    const char *value = "";
    long format_l, key_l;
    char ckey[100];
    unsigned int okay;
    volatile VALUE fmt_arg;

    Data_Get_Struct(self, Info, info);

    switch (argc)
    {
        case 3:
            /* Allow any argument that supports to_s */
            fmt_arg = rb_String(argv[2]);
            value = (const char *)StringValuePtr(fmt_arg);
        case 2:
            key = rm_str2cstr(argv[1], &key_l);
            format = rm_str2cstr(argv[0], &format_l);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 2 or 3)", argc);
    }

    if (2 + format_l + key_l > (long)sizeof(ckey))
    {
        rb_raise(rb_eArgError, "%.20s:%.20s not defined - format or key too long", format, key);
    }
    (void) sprintf(ckey, "%s:%s", format, key);

    (void) RemoveImageOption(info, ckey);
    okay = SetImageOption(info, ckey, value);
    if (!okay)
    {
        rb_warn("%.20s=\"%.78s\" not defined - SetImageOption failed.", ckey, value);
        return Qnil;
    }

    return self;
}

/*
    Method:     Info#delay
    Purpose:    Get the delay attribute
    Notes:      Convert from string to numeric
*/
VALUE
Info_delay(VALUE self)
{
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
}

/*
 * Will raise an exception if `arg' can't be converted to an int.
*/
static VALUE
arg_is_integer(VALUE arg)
{
    int d = NUM2INT(arg);
    d = d;      // satisfy icc
    return arg;
}

/*
    Method:     Info#delay=
    Purpose:    Set the delay attribute
    Notes:      Convert from numeric value to string.
*/
VALUE
Info_delay_eq(VALUE self, VALUE string)
{
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
        (void) rb_protect(arg_is_integer, string, &not_num);
        if (not_num)
        {
            rb_raise(rb_eTypeError, "failed to convert %s into Integer", rb_class2name(CLASS_OF(string)));
        }
        delay = NUM2INT(string);
        sprintf(dstr, "%d", delay);
        (void) RemoveImageOption(info, "delay");
        (void) SetImageOption(info, "delay", dstr);
    }
    return self;
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

    if (NIL_P(density_arg))
    {
        magick_free(info->density);
        info->density = NULL;
        return self;
    }

    density = rm_to_s(density_arg);
    dens = StringValuePtr(density);
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
    unsigned long d;

    Data_Get_Struct(self, Info, info);
    d = NUM2ULONG(depth);
    switch (d)
    {
        case 8:                     // always okay
#if QuantumDepth == 16 || QuantumDepth == 32 || QuantumDepth == 64
        case 16:
#if QuantumDepth == 32 || QuantumDepth == 64
        case 32:
#if QuantumDepth == 64
        case 64:
#endif
#endif
#endif
            break;
        default:
            rb_raise(rb_eArgError, "invalid depth (%lu)", d);
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
static struct
{
    const char *string;
    const char *enum_name;
    DisposeType enumerator;
} Dispose_Option[] = {
    { "Background", "BackgroundDispose", BackgroundDispose},
    { "None",       "NoneDispose",       NoneDispose},
    { "Previous",   "PreviousDispose",   PreviousDispose},
    { "Undefined",  "UndefinedDispose",  UndefinedDispose},
    { "0",          "UndefinedDispose",  UndefinedDispose},
    { "1",          "NoneDispose",       NoneDispose},
    { "2",          "BackgroundDispose", BackgroundDispose},
    { "3",          "PreviousDispose",   PreviousDispose},
};
#define N_DISPOSE_OPTIONS (int)(sizeof(Dispose_Option)/sizeof(Dispose_Option[0]))

VALUE
Info_dispose(VALUE self)
{
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
}

/*
    Method:     Info#dispose=
    Purpose:    Convert a DisposeType enumerator into the equivalent
                dispose option string
*/
VALUE
Info_dispose_eq(VALUE self, VALUE disp)
{
    Info *info;
    DisposeType dispose;
    const char *option;
    int x;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(disp))
    {
        (void) RemoveImageOption(info, "dispose");
        return self;
    }

    VALUE_TO_ENUM(disp, dispose, DisposeType);
    option = "Undefined";

    for (x = 0; x < N_DISPOSE_OPTIONS; x++)
    {
        if (dispose == Dispose_Option[x].enumerator)
        {
            option = Dispose_Option[x].string;
            break;
        }
    }

    (void) SetImageOption(info, "dispose", option);
    return self;
}

DEF_ATTR_ACCESSOR(Info, dither, bool)


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

    extract = rm_to_s(extract_arg);
    extr = StringValuePtr(extract);
    if (!IsGeometry(extr))
    {
        rb_raise(rb_eArgError, "invalid extract geometry: %s", extr);
    }

    magick_clone_string(&info->extract, extr);

    return self;
}


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
    if (NIL_P(filename) || StringValuePtr(filename) == NULL)
    {
        info->filename[0] = '\0';
    }
    else
    {
        // Otherwise copy in filename
        fname = StringValuePtr(filename);
        strncpy(info->filename, fname, MaxTextExtent);
    }
    return self;
}


/*
    Method:     Info#fill
    Purpose:    return the fill color as a String
*/
VALUE
Info_fill(VALUE self)
{
    return get_option(self, "fill");
}

/*
    Method:     Info#fill=<aString>
    Purpose:    set the fill color
    Raises:     ArgumentError
*/
VALUE
Info_fill_eq(VALUE self, VALUE color)
{
    return set_color_option(self, "fill", color);
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
    if (NIL_P(font_arg) || StringValuePtr(font_arg) == NULL)
    {
        magick_free(info->font);
        info->font = NULL;
    }
    else
    {
        font = StringValuePtr(font_arg);
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
        (void) DestroyExceptionInfo(&exception);

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

    mgk = StringValuePtr(magick);
    m = GetMagickInfo(mgk, &exception);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

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

static struct
{
    const char *string;
    const char *enum_name;
    GravityType enumerator;
} Gravity_Option[] = {
    { "Undefined",  "UndefinedGravity", UndefinedGravity},
    { "None",       "UndefinedGravity", UndefinedGravity},
    { "Center",     "CenterGravity",    CenterGravity},
    { "East",       "EastGravity",      EastGravity},
    { "Forget",     "ForgetGravity",    ForgetGravity},
    { "NorthEast",  "NorthEastGravity", NorthEastGravity},
    { "North",      "NorthGravity",     NorthGravity},
    { "NorthWest",  "NorthWestGravity", NorthWestGravity},
    { "SouthEast",  "SouthEastGravity", SouthEastGravity},
    { "South",      "SouthGravity",     SouthGravity},
    { "SouthWest",  "SouthWestGravity", SouthWestGravity},
    { "West",       "WestGravity",      WestGravity},
    { "Static",     "StaticGravity",    StaticGravity}
};
#define N_GRAVITY_OPTIONS (int)(sizeof(Gravity_Option)/sizeof(Gravity_Option[0]))

VALUE Info_gravity(VALUE self)
{
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
}

/*
    Method:     Info#gravity=
    Purpose:    Convert a GravityType enum to a gravity option name and
                store in the Info structure
*/
VALUE
Info_gravity_eq(VALUE self, VALUE grav)
{
    Info *info;
    GravityType gravity;
    const char *option;
    int x;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(grav))
    {
        (void) RemoveImageOption(info, "gravity");
        return self;
    }

    VALUE_TO_ENUM(grav, gravity, GravityType);
    option = "Undefined";

    for (x = 0; x < N_GRAVITY_OPTIONS; x++)
    {
        if (gravity == Gravity_Option[x].enumerator)
        {
            option = Gravity_Option[x].string;
            break;
        }
    }

    (void) SetImageOption(info, "gravity", option);
    return self;
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
}




DEF_ATTR_ACCESSOR(Info, monochrome, bool)

DEF_ATTR_ACCESSOR(Info, number_scenes, ulong)

/*
    Method:     Info#orientation
    Purpose:    Return the orientation attribute as an OrientationType enum value.
*/
VALUE
Info_orientation(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return OrientationType_new(info->orientation);
}


/*
    Method:     Info#Orientation=
    Purpose:    Set the Orientation type
    Raises:     ArgumentError
*/
VALUE
Info_orientation_eq(VALUE self, VALUE inter)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(inter, info->orientation, OrientationType);
    return self;
}


/*
    Method:     Info#origin
    Purpose:    Return origin geometry
*/
VALUE
Info_origin(VALUE self)
{
    Info *info;
    const char *origin;

    Data_Get_Struct(self, Info, info);

    origin = GetImageOption(info, "origin");
    return origin ? rb_str_new2(origin) : Qnil;
}


/*
    Method:     Info#origin=+-x+-y
    Purpose:    Set origin geometry. Argument may be a Geometry object as well
                as a geometry string.
*/
VALUE
Info_origin_eq(VALUE self, VALUE origin_arg)
{
    Info *info;
    volatile VALUE origin_str;
    char *origin;

    Data_Get_Struct(self, Info, info);

    if (NIL_P(origin_arg))
    {
        (void) RemoveImageOption(info, "origin");
        return self;
    }

    origin_str = rm_to_s(origin_arg);
    origin = GetPageGeometry(StringValuePtr(origin_str));

    if (IsGeometry(origin) == MagickFalse)
    {
        rb_raise(rb_eArgError, "invalid origin geometry: %s", origin);
    }

    (void) SetImageOption(info, "origin", origin);
    return self;
}


VALUE
Info_page(VALUE self)
{
    Info *info;
    const char *page;

    Data_Get_Struct(self, Info, info);
    page = GetImageOption(info, "page");
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
    geom_str = rm_to_s(page_arg);
    geometry=GetPageGeometry(StringValuePtr(geom_str));
    if (*geometry == '\0')
    {
        magick_free(info->page);
        info->page = NULL;
        return self;
    }
    magick_clone_string(&info->page, geometry);
    (void) SetImageOption(info, "page", StringValuePtr(geom_str));

    return self;
}

DEF_ATTR_ACCESSOR(Info, pointsize, dbl)
DEF_ATTR_ACCESSOR(Info, quality, ulong)

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
        sampling_factor_p = rm_str2cstr(sampling_factor, &sampling_factor_len);
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

DEF_ATTR_ACCESSOR(Info, scene, ulong)
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
    if (NIL_P(server_arg) || StringValuePtr(server_arg) == NULL)
    {
        magick_free(info->server_name);
        info->server_name = NULL;
    }
    else
    {
        server = StringValuePtr(server_arg);
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

    size = rm_to_s(size_arg);
    sz = StringValuePtr(size);
    if (!IsGeometry(sz))
    {
        rb_raise(rb_eArgError, "invalid size geometry: %s", sz);
    }

    magick_clone_string(&info->size, sz);

    return self;
}


/*
    Method:     Info#stroke
    Purpose:    return the stroke color as a String
*/
VALUE
Info_stroke(VALUE self)
{
    return get_option(self, "stroke");
}

/*
    Method:     Info#stroke=<aString>
    Purpose:    set the stroke color
    Raises:     ArgumentError
*/
VALUE
Info_stroke_eq(VALUE self, VALUE color)
{
    return set_color_option(self, "stroke", color);
}


/*
    Method:     Info#stroke_width
    Purpose:    Support for caption: format
    Notes:      Supported >= 6.3.2-6
*/
VALUE
Info_stroke_width(VALUE self)
{
    return get_dbl_option(self, "strokewidth");
}


/*
    Method:     Info#stroke_width=
    Purpose:    Support for caption: format
    Notes:      Supported >= 6.3.2-6
*/
VALUE
Info_stroke_width_eq(VALUE self, VALUE stroke_width)
{
    return set_dbl_option(self, "strokewidth", stroke_width);
}


/*
    Method:     Image::Info#texture=texture_image
    Purpose:    Set name of texture to tile onto the image background
*/
VALUE
Info_texture_eq(VALUE self, VALUE texture)
{
    Info *info;
    Image *image;
    char name[MaxTextExtent];

    Data_Get_Struct(self, Info, info);

    // Delete any existing texture file
    if (info->texture)
    {
        rm_delete_temp_image(info->texture);
        magick_free(info->texture);
        info->texture = NULL;
    }

    // If argument is nil we're done
    if (texture == Qnil)
    {
        return self;
    }

    // Create a temp copy of the texture and store its name in the texture field
    image = rm_check_destroyed(texture);
    rm_write_temp_image(image, name);

    magick_clone_string(&info->texture, name);

    return self;
}


/*
    Method:     Image::Info#tile_offset= geometry
    Purpose:    info.tile_offset = [+/-]x[+/-]y
*/
VALUE
Info_tile_offset_eq(VALUE self, VALUE offset)
{
    Info *info;
    volatile VALUE offset_str;
    char *tile_offset;

    offset_str = rm_to_s(offset);
    tile_offset = StringValuePtr(offset_str);
    if (!IsGeometry(tile_offset))
    {
        rb_raise(rb_eArgError, "invalid tile offset geometry: %s", tile_offset);
    }

    Data_Get_Struct(self, Info, info);

    (void) DeleteImageOption(info, "tile-offset");
    (void) SetImageOption(info, "tile-offset", tile_offset);
    return self;
}


/*
    Method:     Image::Info#tile_offset
    Purpose:    returns tile_offset attribute values
*/
VALUE
Info_tile_offset(VALUE self)
{
    Info *info;
    const char *tile_offset;

    Data_Get_Struct(self, Info, info);

    tile_offset = GetImageOption(info, "tile-offset");

    if (!tile_offset)
    {
        return Qnil;
    }

    return rb_str_new2(tile_offset);
}


/*
    Method:     Info#undefine
    Purpose:    Undefine image option
*/

VALUE
Info_undefine(VALUE self, VALUE format, VALUE key)
{
    Info *info;
    char *format_p, *key_p;
    long format_l, key_l;
    char fkey[MaxTextExtent];

    format_p = rm_str2cstr(format, &format_l);
    key_p = rm_str2cstr(key, &key_l);

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
}


/*
    Method:     Info#undercolor
    Purpose:    return the undercolor color as a String
*/
VALUE
Info_undercolor(VALUE self)
{
    return get_option(self, "undercolor");
}

/*
    Method:     Info#undercolor=<aString>
    Purpose:    set the undercolor color
    Raises:     ArgumentError
*/
VALUE
Info_undercolor_eq(VALUE self, VALUE color)
{
    return set_color_option(self, "undercolor", color);
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

    if (NIL_P(view_arg) || StringValuePtr(view_arg) == NULL)
    {
        magick_free(info->view);
        info->view = NULL;
    }
    else
    {
        view = StringValuePtr(view_arg);
        magick_clone_string(&info->view, view);
    }
    return self;
}


/*
    Static:     destroy_Info
    Purpose:    if there is a texture image, delete it before destroying
                the ImageInfo structure
*/
static void
destroy_Info(void *infoptr)
{
    Info *info = (Info *)infoptr;

    if (info->texture)
    {
        rm_delete_temp_image(info->texture);
        magick_free(info->texture);
        info->texture = NULL;
    }

    (void) DestroyImageInfo(info);
}


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
    info_obj = Data_Wrap_Struct(class, NULL, destroy_Info, info);
    return info_obj;
}


/*
    Extern:     rm_info_new
    Purpose:    provide a Info.new method for internal use
    Notes:      takes no parameters, but runs the parm block if present
*/
VALUE
rm_info_new(void)
{
    volatile VALUE info_obj;

    info_obj = Info_alloc(Class_Info);
    return Info_initialize(info_obj);
}


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
        (void) rb_obj_instance_eval(0, NULL, self);
    }
    return self;
}

