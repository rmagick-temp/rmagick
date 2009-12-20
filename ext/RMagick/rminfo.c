/**************************************************************************//**
 * Info class method definitions for RMagick.
 *
 * Copyright &copy; 2002 - 2009 by Timothy P. Hunter
 *
 * Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or
 *
 * @file     rminfo.c
 * @version  $Id: rminfo.c,v 1.79 2009/12/20 02:33:33 baror Exp $
 * @author   Tim Hunter
 ******************************************************************************/

#include "rmagick.h"





/**
 * Return the value of the specified option.
 *
 * Ruby usage:
 *   - @verbatim Info#get_option(key) @endverbatim
 *
 * @param self this object
 * @param key the option key
 * @return the value of key
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

/**
 * Set the specified option to this value. If the value is nil just unset any
 * current value.
 *
 * Ruby usage:
 *   - @verbatim Info#set_option(key,string) @endverbatim
 *
 * @param self this object
 * @param key the option key
 * @param string the value
 * @return self
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


/**
 * Set a color name as the value of the specified option
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Call QueryColorDatabase to validate color name.
 *
 * @param self this object
 * @param option the option
 * @param color the color name
 * @return self
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


/**
 * Get an Image::Info option floating-point value.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Convert the string value to a float
 *
 * @param self this object
 * @param option the option name
 * @return the Image::Info option
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


/**
 * Set an Image::Info option to a floating-point value.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - SetImageOption expects the value to be a string.
 *
 * @param self this object
 * @param option the option name
 * @param value the value
 * @return self
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


#if 0
/**
 * Convert a PixelPacket to a hex-format color name.
 *
 * No Ruby usage (internal function)
 *
 * @param pp the pixel packet
 * @param name pointer to the name
 * @return the name
 */
static char *pixel_packet_to_hexname(PixelPacket *pp, char *name)
{
    MagickPixelPacket mpp;

    GetMagickPixelPacket(NULL, &mpp);
    rm_set_magick_pixel_packet(pp, &mpp);
    (void) GetColorTuple(&mpp, MagickTrue, name);
    return name;
}
#endif


DEF_ATTR_ACCESSOR(Info, antialias, bool)

/** Maximum length of a format (@see Info_aref) */
#define MAX_FORMAT_LEN 60

/**
 * Get the value of an option set by Info[]=
 *
 * Ruby usage:
 *   - @verbatim Info[format, key] @endverbatim
 *   - @verbatim Info[key] @endverbatim
 *
 * Notes:
 *   - The 2 argument form is the original form. Added support for a single
 *     argument after ImageMagick started using Set/GetImageOption for options
 *     that aren't represented by fields in the ImageInfo structure.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the option value
 * @see Info_aset
 */
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


/**
 * Call SetImageOption
 *
 * Ruby usage:
 *   - @verbatim Info[format, key]= @endverbatim
 *   - @verbatim Info[key]= @endverbatim
 *
 * Notes:
 *   - Essentially the same function as Info_define but paired with Info_aref
 *   - If the value is nil it is equivalent to Info_undefine. 
 *   - The 2 argument form is the original form. Added support for a single
 *     argument after ImageMagick started using Set/GetImageOption for options
 *     that aren't represented by fields in the ImageInfo structure.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 * @see Info_aref
 * @see Info_define
 * @see Info_undefine
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


/**
 * Get the attenuate attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#attenuate @endverbatim
 *
 * @param self this object
 * @return the attenuate
 */
VALUE
Info_attenuate(VALUE self)
{
    return get_dbl_option(self, "attenuate");
}


/**
 * Set the attenuate attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#attenuate= @endverbatim
 *
 * @param self this object
 * @param value the attenuate
 * @return self
 */
VALUE
Info_attenuate_eq(VALUE self, VALUE value)
{
    return set_dbl_option(self, "attenuate", value);
}


/**
 * Get the authenticate attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#authenticate @endverbatim
 *
 * @param self this object
 * @return the authenticate
 */
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


/**
 * Set the authenticate attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#authenticate= @endverbatim
 *
 * @param self this object
 * @param passwd the authenticating password
 * @return self
 */
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


/**
 * Return the name of the background color as a String
 *
 * Ruby usage:
 *   - @verbatim Info#background_color @endverbatim
 *
 * @param self this object
 * @return the name of the background color
 * @see Image_background_color
 */
VALUE
Info_background_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return rm_pixelpacket_to_color_name_info(info, &info->background_color);
}


/**
 * Set the background color.
 *
 * Ruby usage:
 *   - @verbatim Info#background_color= @endverbatim
 *
 * Notes:
 *   - Color should be a string
 *
 * @param self this object
 * @param bc_arg the background color
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_background_color_eq(VALUE self, VALUE bc_arg)
{
    Info *info;
    //char colorname[MaxTextExtent];

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->background_color, bc_arg);
    //SetImageOption(info, "background", pixel_packet_to_hexname(&info->background_color, colorname));
    return self;
}

/**
 * Return the name of the border color as a String.
 *
 * Ruby usage:
 *   - @verbatim Info#border_color @endverbatim
 *
 * @param self this object
 * @return the border color
 * @see Image_border_color
 */
VALUE
Info_border_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return rm_pixelpacket_to_color_name_info(info, &info->border_color);
}

/**
 * set the border color
 *
 * Ruby usage:
 *   - @verbatim Info#border_color= @endverbatim
 *
 * Notes:
 *   - Color should be a string
 *
 * @param self this object
 * @param bc_arg the border color
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_border_color_eq(VALUE self, VALUE bc_arg)
{
    Info *info;
    //char colorname[MaxTextExtent];

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->border_color, bc_arg);
    //SetImageOption(info, "bordercolor", pixel_packet_to_hexname(&info->border_color, colorname));
    return self;
}



/**
 * Emulate the -caption option.
 *
 * Ruby usage:
 *   - @verbatim Info#caption @endverbatim
 *
 * @param self this object
 * @return the caption
 */
VALUE
Info_caption(VALUE self)
{
    return get_option(self, "caption");
}



/**
 * Emulate the -caption option.
 *
 * Ruby usage:
 *   - @verbatim Info#caption= @endverbatim
 *
 * @param self this object
 * @param caption the caption
 * @return self
 */
VALUE
Info_caption_eq(VALUE self, VALUE caption)
{
    return set_option(self, "caption", caption);
}


/**
 * Set the channels
 *
 * Ruby usage:
 *   - @verbatim Info#channel @endverbatim
 *   - @verbatim Info#channel(channel) @endverbatim
 *   - @verbatim Info#channel(channel, ...) @endverbatim
 *
 * Notes:
 *   - Default channel is AllChannels
 *   - Thanks to Douglas Sellers.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
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


/**
 * Get the colorspace type.
 *
 * Ruby usage:
 *   - @verbatim Info#colorspace @endverbatim
 *
 * @param self this object
 * @return the colorspace type
 */
VALUE
Info_colorspace(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return ColorspaceType_new(info->colorspace);
}

/**
 * Set the colorspace type
 *
 * Ruby usage:
 *   - @verbatim Info#colorspace= @endverbatim
 *
 * @param self this object
 * @param colorspace the colorspace type
 * @return self
 * @throw ArgumentError
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

/**
 * Get the compression type.
 *
 * Ruby usage:
 *   - @verbatim Info#compression @endverbatim
 *
 * @param self this object
 * @return the compression type
 */
VALUE
Info_compression(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return CompressionType_new(info->compression);
}

/**
 * Set the compression type
 *
 * Ruby usage:
 *   - @verbatim Info#compression= @endverbatim
 *
 * @param self this object
 * @param type the compression type
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_compression_eq(VALUE self, VALUE type)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(type, info->compression, CompressionType);
    return self;
}

/**
 * Call SetImageOption
 *
 * Ruby usage:
 *   - @verbatim Info#define(format, key) @endverbatim
 *   - @verbatim Info#define(format, key, value) @endverbatim
 *
 * Notes:
 *   - Default value is the empty string
 *   - This is the only method in Info that is not an attribute accessor.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
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

/**
 * Get the delay attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#delay @endverbatim
 *
 * Notes:
 *   - Convert from string to numeric
 *
 * @param self this object
 * @return the delay
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

/**
 * Will raise an exception if `arg' can't be converted to an int.
 *
 * No Ruby usage (internal function)
 *
 * @param arg the argument
 * @return arg
 */
static VALUE
arg_is_integer(VALUE arg)
{
    int d = NUM2INT(arg);
    d = d;      // satisfy icc
    return arg;
}

/**
 * Set the delay attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#delay= @endverbatim
 *
 * Notes:
 *   - Convert from numeric value to string.
 *
 * @param self this object
 * @param string the delay
 * @return self
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

/**
 * Get the density attribute
 *
 * Ruby usage:
 *   - @verbatim Info#density @endverbatim
 *
 * @param self this object
 * @return the density
 */
DEF_ATTR_READER(Info, density, str)

/**
 * Set the text rendering density
 *
 * Ruby usage:
 *   - @verbatim Info#density= @endverbatim
 *
 * Notes:
 *   - density should be a string, e.g., "72x72"
 *
 * @param self this object
 * @param density_arg the density
 * @return self
 * @throw ArgumentError
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

/**
 * Get the depth attribute
 *
 * Ruby usage:
 *   - @verbatim Info#depth @endverbatim
 *
 * @param self this object
 * @return the depth
 */
DEF_ATTR_READER(Info, depth, int)

/**
 * Set the depth (8, 16, 32).
 *
 * Ruby usage:
 *   - @verbatim Info#depth= @endverbatim
 *
 * @param self this object
 * @param depth the depth
 * @return self
 * @throw ArgumentError
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

/** A dispose option */
static struct
{
    const char *string; /**< the argument given by the user */
    const char *enum_name; /**< the enumerator name */
    DisposeType enumerator; /**< the enumerator itself */
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

/** Number of dispose options */
#define N_DISPOSE_OPTIONS (int)(sizeof(Dispose_Option)/sizeof(Dispose_Option[0]))


/**
 * Retrieve a dispose option string and convert it to a DisposeType enumerator.
 *
 * No Ruby usage (internal function)
 *
 * @param name the dispose string
 * @return the DisposeType enumerator
 */
DisposeType rm_dispose_to_enum(const char *name)
{
    DisposeType dispose = UndefinedDispose;
    int x;

    for (x = 0; x < N_DISPOSE_OPTIONS; x++)
    {
        if (strcmp(Dispose_Option[x].string, name) == 0)
        {
            dispose = Dispose_Option[x].enumerator;
            break;
        }
    }

    return dispose;
}


/**
 * Retrieve the dispose option string and convert it to a DisposeType
 * enumerator.
 *
 * Ruby usage:
 *   - @verbatim Info#dispose @endverbatim
 *
 * @param self this object
 * @return a DisposeType enumerator
 */
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

/**
 * Convert a DisposeType enumerator into the equivalent dispose option string.
 *
 * Ruby usage:
 *   - @verbatim Info#dispose= @endverbatim
 *
 * @param self this object
 * @param disp the DisposeType enumerator
 * @return self
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


/**
 * Get the endian attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#endian @endverbatim
 *
 * @param self this object
 * @return the endian (Magick::MSBEndian or Magick::LSBEndian)
 */
VALUE
Info_endian(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return EndianType_new(info->endian);
}


/**
 * Set the endian attribute.
 *
 * Ruby usage:
 *   - @verbatim Info#endian= @endverbatim
 *
 * @param self this object
 * @param endian the endian (Magick::MSBEndian or Magick::LSBEndian)
 * @return self
 */
VALUE
Info_endian_eq(VALUE self, VALUE endian)
{
    Info *info;
    EndianType type = UndefinedEndian;

    if (endian != Qnil)
    {
        VALUE_TO_ENUM(endian, type, EndianType);
    }

    Data_Get_Struct(self, Info, info);
    info->endian = type;
    return self;
}


/**
 * Get the extract string, e.g. "200x200+100+100"
 *
 * Ruby usage:
 *   - @verbatim Info#extract @endverbatim
 *
 * Notes:
 *   - Defined for ImageMagick 5.5.6 and later
 *
 * @param self this object
 * @return the extract string
 */
DEF_ATTR_READER(Info, extract, str)

/**
 * Set the extract string, e.g. "200x200+100+100"
 *
 * Ruby usage:
 *   - @verbatim Info#extract= @endverbatim
 *
 * Notes:
 *   - Defined for ImageMagick 5.5.6 and later
 *
 * @param self this object
 * @param extract_arg the extract string
 * @return self
 * @throw ArgumentError
 */
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


/**
 * Get the "filename".
 *
 * Ruby usage:
 *   - @verbatim Info#filename @endverbatim
 *
 * Notes:
 *   - Only used for Image_capture
 *
 * @param self this object
 * @return the filename ("" if filename not set)
 * @see Image_capture
 */
VALUE
Info_filename(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return rb_str_new2(info->filename);
}

/**
 * Set the "filename".
 *
 * Ruby usage:
 *   - @verbatim Info#filename= @endverbatim
 *
 * Notes:
 *   - Only used for Image_capture
 *
 * @param self this object
 * @param filename the filename
 * @return self
 * @see Image_capture
 */
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


/**
 * Return the fill color as a String.
 *
 * Ruby usage:
 *   - @verbatim Info#fill @endverbatim
 *
 * @param self this object
 * @return the fill color
 */
VALUE
Info_fill(VALUE self)
{
    return get_option(self, "fill");
}

/**
 * Set the fill color
 *
 * Ruby usage:
 *   - @verbatim Info#fill= @endverbatim
 *
 * @param self this object
 * @param color the fill color (as a String)
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_fill_eq(VALUE self, VALUE color)
{
    return set_color_option(self, "fill", color);
}


/**
 * Get the text font.
 *
 * Ruby usage:
 *   - @verbatim Info#font @endverbatim
 *
 * @param self this object
 * @return the font
 */
DEF_ATTR_READER(Info, font, str)

/**
 * Set the text font.
 *
 * Ruby usage:
 *   - @verbatim Info#font= @endverbatim
 *
 * @param self this object
 * @param font_arg the font (as a String)
 * @return self
 */
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

/**
 * Return the image encoding format.
 *
 * Ruby usage:
 *   - @verbatim Info#format @endverbatim
 *
 * @param self this object
 * @return the encoding format
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

/**
 * Set the image encoding format.
 *
 * Ruby usage:
 *   - @verbatim Info#format= @endverbatim
 *
 * @param self this object
 * @param magick the encoding format
 * @return self
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

/**
 * Get the fuzz.
 *
 * Ruby usage:
 *   - @verbatim Info#fuzz @endverbatim
 *
 * @param self this object
 * @return the fuzz
 * @see Image_fuzz
 */
DEF_ATTR_READER(Info, fuzz, dbl)

/**
 * Set the fuzz.
 *
 * Ruby usage:
 *   - @verbatim Info#fuzz=number @endverbatim
 *   - @verbatim Info#fuzz=NN% @endverbatim
 *
 * @param self this object
 * @param fuzz the fuzz
 * @return self
 * @see Image_fuzz_eq
 */
VALUE Info_fuzz_eq(VALUE self, VALUE fuzz)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    info->fuzz = rm_fuzz_to_dbl(fuzz);
    return self;
}

/** A gravity option */
static struct
{
    const char *string; /**< the argument given by the user */
    const char *enum_name; /**< the enumerator name */
    GravityType enumerator; /**< the enumerator itself */
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

/** Number of gravity options */
#define N_GRAVITY_OPTIONS (int)(sizeof(Gravity_Option)/sizeof(Gravity_Option[0]))


/**
 * Return the value of the gravity option as a GravityType enumerator.
 *
 * No Ruby usage (internal function)
 *
 * @param name the name of the gravity option
 * @return the enumerator for name
 */
GravityType rm_gravity_to_enum(const char *name)
{
    GravityType gravity = UndefinedGravity;
    int x;

    for (x = 0; x < N_GRAVITY_OPTIONS; x++)
    {
        if (strcmp(name, Gravity_Option[x].string) == 0)
        {
            gravity = Gravity_Option[x].enumerator;
            break;
        }
    }

    return gravity;
}


/**
 * Return the value of the gravity option as a GravityType enumerator.
 *
 * Ruby usage:
 *   - @verbatim Info#gravity @endverbatim
 *
 * @param self this object
 * @return the gravity enumerator
 */
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

/**
 * Convert a GravityType enum to a gravity option name and store in the Info
 * structure.
 *
 * Ruby usage:
 *   - @verbatim Info#gravity= @endverbatim
 *
 * @param self this object
 * @param grav the gravity enumerator
 * @return self
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

/**
 * Get the classification type.
 *
 * Ruby usage:
 *   - @verbatim Info#image_type @endverbatim
 *
 * @param self this object
 * @return the classification type
 */
VALUE
Info_image_type(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return ImageType_new(info->type);
}

/**
 * Set the classification type.
 *
 * Ruby usage:
 *   - @verbatim Info#image_type= @endverbatim
 *
 * @param self this object
 * @param type the classification type
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_image_type_eq(VALUE self, VALUE type)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(type, info->type, ImageType);
    return self;
}

/**
 * Get the interlace type.
 *
 * Ruby usage:
 *   - @verbatim Info#interlace @endverbatim
 *
 * @param self this object
 * @return the interlace type
 */
VALUE
Info_interlace(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return InterlaceType_new(info->interlace);
}

/**
 * Set the interlace type
 *
 * Ruby usage:
 *   - @verbatim Info#interlace= @endverbatim
 *
 * @param self this object
 * @param inter the interlace type
 * @return self
 * @throw ArgumentError
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

/**
 * Return the name of the matte color as a String.
 *
 * Ruby usage:
 *   - @verbatim Info#matte_color @endverbatim
 *
 * @param self this object
 * @return the name of the matte color
 * @see Image_matte_color
 */
VALUE
Info_matte_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return rm_pixelpacket_to_color_name_info(info, &info->matte_color);
}

/**
 * Set the matte color.
 *
 * Ruby usage:
 *   - @verbatim Info#matte_color= @endverbatim
 *
 * @param self this object
 * @param matte_arg the name of the matte as a String
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_matte_color_eq(VALUE self, VALUE matte_arg)
{
    Info *info;
    //char colorname[MaxTextExtent];

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->matte_color, matte_arg);
    //SetImageOption(info, "mattecolor", pixel_packet_to_hexname(&info->matte_color, colorname));
    return self;
}

/**
 * Establish a progress monitor.
 *
 * Ruby usage:
 *   - @verbatim Info#monitor= @endverbatim
 *
 * @param self this object
 * @param monitor the monitor
 * @return self
 * @see Image_monitor_eq
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

/**
 * Return the orientation attribute as an OrientationType enum value.
 *
 * Ruby usage:
 *   - @verbatim Info#orientation @endverbatim
 *
 * @param self this object
 * @return the orientation
 */
VALUE
Info_orientation(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return OrientationType_new(info->orientation);
}


/**
 * Set the Orientation type.
 *
 * Ruby usage:
 *   - @verbatim Info#Orientation= @endverbatim
 *
 * @param self this object
 * @param inter the orientation type as an OrientationType enum value
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_orientation_eq(VALUE self, VALUE inter)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(inter, info->orientation, OrientationType);
    return self;
}


/**
 * Return origin geometry.
 *
 * Ruby usage:
 *   - @verbatim Info#origin @endverbatim
 *
 * @param self this object
 * @return the origin geometry
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


/**
 * Set origin geometry. Argument may be a Geometry object as well as a geometry
 * string.
 *
 * Ruby usage:
 *   - @verbatim Info#origin=+-x+-y @endverbatim
 *
 * @param self this object
 * @param origin_arg the origin geometry
 * @return self
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


/**
 * Get the Postscript page geometry.
 *
 * Ruby usage:
 *   - @verbatim Info_page @endverbatim
 *
 * @param self this object
 * @return the page geometry
 */
VALUE
Info_page(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return info->page ? rb_str_new2(info->page) : Qnil;

}

/**
 * Store the Postscript page geometry. Argument may be a Geometry object as well
 * as a geometry string.
 *
 * Ruby usage:
 *   - @verbatim Info#page= @endverbatim
 *
 * @param self this object
 * @param page_arg the geometry
 * @return self
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

    return self;
}

DEF_ATTR_ACCESSOR(Info, pointsize, dbl)
DEF_ATTR_ACCESSOR(Info, quality, ulong)

/**
 * Get sampling factors used by JPEG or MPEG-2 encoder and YUV decoder/encoder.
 *
 * Ruby usage:
 *   - @verbatim Info#sampling_factor @endverbatim
 *
 * @param self this object
 * @return the sampling factors
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

/**
 * Set sampling factors used by JPEG or MPEG-2 encoder and YUV decoder/encoder.
 *
 * Ruby usage:
 *   - @verbatim Info#sampling_factor= @endverbatim
 *
 * @param self this object
 * @param sampling_factor the sampling factors
 * @return self
 */
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


/**
 * Get the scene number.
 *
 * Ruby usage:
 *   - @verbatim Info#scene @endverbatim
 *
 * @param self this object
 * @return the scene number
 */
VALUE
Info_scene(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return  ULONG2NUM(info->scene);
}


/**
 * Set the scene number.
 *
 * Ruby usage:
 *   - @verbatim Info#scene= @endverbatim
 *
 * @param self this object
 * @param scene the scene number
 * @return self
 */
VALUE
Info_scene_eq(VALUE self, VALUE scene)
{
    Info *info;
    char buf[25];

    Data_Get_Struct(self, Info, info);
    info->scene = NUM2ULONG(scene);

#if defined(HAVE_SNPRINTF)
    (void) snprintf(buf, sizeof(buf), "%-ld", info->scene);
#else
    (void) sprintf(buf, "%-l", info->scene);
#endif
    (void) SetImageOption(info, "scene", buf);

    return self;
}


/**
 * Get the server name.
 *
 * Ruby usage:
 *   - @verbatim Info#server_name @endverbatim
 *
 * @param self this object
 * @return the server name
 */
DEF_ATTR_READER(Info, server_name, str)


/**
 * Set the server name.
 *
 * Ruby usage:
 *   - @verbatim Info#server_name= @endverbatim
 *
 * @param self this object
 * @param server_arg the server name as a String
 * @return self
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

/**
 * Get ths size
 *
 * Ruby usage:
 *   - @verbatim Info#size @endverbatim
 *
 * @param self this object
 * @return the size as a Geometry object
 */
DEF_ATTR_READER(Info, size, str)

/**
 * Set the size (either as a Geometry object or a Geometry string, i.e.
 * WxH{+-}x{+-}y)
 *
 * Ruby usage:
 *   - @verbatim Info#size= @endverbatim
 *
 * @param self this object
 * @param size_arg the size
 * @return self
 * @throw ArgumentError
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


/**
 * Return the stroke color as a String.
 *
 * Ruby usage:
 *   - @verbatim Info#stroke @endverbatim
 *
 * @param self this object
 * @return the stroke color
 */
VALUE
Info_stroke(VALUE self)
{
    return get_option(self, "stroke");
}

/**
 * Set the stroke color
 *
 * Ruby usage:
 *   - @verbatim Info#stroke= @endverbatim
 *
 * @param self this object
 * @param color the stroke color as a String
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_stroke_eq(VALUE self, VALUE color)
{
    return set_color_option(self, "stroke", color);
}


/**
 * Support for caption: format.
 *
 * Ruby usage:
 *   - @verbatim Info#stroke_width @endverbatim
 *
 * Notes:
 *   - Supported in ImageMagick >= 6.3.2-6
 *
 * @param self this object
 * @return the stroke width
 */
VALUE
Info_stroke_width(VALUE self)
{
    return get_dbl_option(self, "strokewidth");
}


/**
 * Support for caption: format.
 *
 * Ruby usage:
 *   - @verbatim Info#stroke_width= @endverbatim
 *
 * Notes:
 *   - Supported in ImageMagick >= 6.3.2-6
 *
 * @param self this object
 * @param stroke_width the stroke width
 * @return self
 */
VALUE
Info_stroke_width_eq(VALUE self, VALUE stroke_width)
{
    return set_dbl_option(self, "strokewidth", stroke_width);
}


/**
 * Set name of texture to tile onto the image background.
 *
 * Ruby usage:
 *   - @verbatim Image::Info#texture= @endverbatim
 *
 * @param self this object
 * @param texture the name of the texture image
 * @return self
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


/**
 * info.tile_offset = [+/-]x[+/-]y.
 *
 * Ruby usage:
 *   - @verbatim Image::Info#tile_offset= @endverbatim
 *
 * @param self this object
 * @param offset the offset
 * @return self
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


/**
 * Return the name of the transparent color as a String.
 *
 * Ruby usage:
 *   - @verbatim Info#transparent_color @endverbatim
 *
 * @param self this object
 * @return the name of the transparent color
 * @see Image_transparent_color
 */
VALUE
Info_transparent_color(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return rm_pixelpacket_to_color_name_info(info, &info->transparent_color);
}


/**
 * Set the transparent color.
 *
 * Ruby usage:
 *   - @verbatim Info#transparent_color= @endverbatim
 *
 * @param self this object
 * @param tc_arg the transparent color as a String
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_transparent_color_eq(VALUE self, VALUE tc_arg)
{
    Info *info;
    //char colorname[MaxTextExtent];

    Data_Get_Struct(self, Info, info);
    Color_to_PixelPacket(&info->transparent_color, tc_arg);
    //SetImageOption(info, "transparent", pixel_packet_to_hexname(&info->transparent_color, colorname));
    return self;
}


/**
 * Return tile_offset attribute values.
 *
 * Ruby usage:
 *   - @verbatim Image::Info#tile_offset @endverbatim
 *
 * @param self this object
 * @return the tile offset
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


/**
 * Undefine image option.
 *
 * Ruby usage:
 *   - @verbatim Info#undefine(format,key) @endverbatim
 *
 * @param self this object
 * @param format the format
 * @param key the key
 * @return self
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


/**
 * Return the undercolor color as a String.
 *
 * Ruby usage:
 *   - @verbatim Info#undercolor @endverbatim
 *
 * @param self this object
 * @return the undercolor
 */
VALUE
Info_undercolor(VALUE self)
{
    return get_option(self, "undercolor");
}

/**
 * Set the undercolor color.
 *
 * Ruby usage:
 *   - @verbatim Info#undercolor= @endverbatim
 *
 * @param self this object
 * @param color the undercolor color as a String
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_undercolor_eq(VALUE self, VALUE color)
{
    return set_color_option(self, "undercolor", color);
}

/**
 * Get the resolution type.
 *
 * Ruby usage:
 *   - @verbatim Info#units @endverbatim
 *
 * @param self this object
 * @return the resolution type
 */
VALUE
Info_units(VALUE self)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    return ResolutionType_new(info->units);
}

/**
 * Set the resolution type
 *
 * Ruby usage:
 *   - @verbatim Info#units= @endverbatim
 *
 * @param self this object
 * @param units the resolution type
 * @return self
 * @throw ArgumentError
 */
VALUE
Info_units_eq(VALUE self, VALUE units)
{
    Info *info;

    Data_Get_Struct(self, Info, info);
    VALUE_TO_ENUM(units, info->units, ResolutionType);
    return self;
}

/**
 * Get FlashPix viewing parameters.
 *
 * Ruby usage:
 *   - @verbatim Info#view @endverbatim
 *
 * @param self this object.
 * @return the viewing parameters
 */
DEF_ATTR_READER(Info, view, str)

/**
 * Set FlashPix viewing parameters.
 *
 * Ruby usage:
 *   - @verbatim Info#view= @endverbatim
 *
 * @param self this object
 * @param view_arg the viewing parameters
 * @return self
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


/**
 * If there is a texture image, delete it before destroying the ImageInfo
 * structure.
 *
 * No Ruby usage (internal function)
 *
 * @param infoptr pointer to the Info object
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


/**
 * Create an ImageInfo object.
 *
 * No Ruby usage (internal function)
 *
 * @param class the Ruby class to use
 * @return a new ImageInfo object
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


/**
 * Provide a Info.new method for internal use.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Takes no parameters, but runs the parm block if present
 *
 * @return a new ImageInfo object
 */
VALUE
rm_info_new(void)
{
    volatile VALUE info_obj;

    info_obj = Info_alloc(Class_Info);
    return Info_initialize(info_obj);
}


/**
 * If an initializer block is present, run it.
 *
 * Ruby usage:
 *   - @verbatim Info#initialize @endverbatim
 *
 * @param self this object
 * @return self
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

