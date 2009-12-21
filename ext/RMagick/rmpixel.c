/**************************************************************************//**
 * Contains Pixel class methods.
 *
 * Copyright &copy; 2002 - 2009 by Timothy P. Hunter
 *
 * Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or
 *
 * @file     rmpixel.c
 * @version  $Id: rmpixel.c,v 1.7 2009/12/21 10:34:58 baror Exp $
 * @author   Tim Hunter
 ******************************************************************************/

#include "rmagick.h"




static void Color_Name_to_PixelPacket(PixelPacket *, VALUE);




/**
 * Free the storage associated with a Pixel object.
 *
 * No Ruby usage (internal function)
 *
 * @param pixel the Pixel object to destroy
 */
void
destroy_Pixel(Pixel *pixel)
{
    xfree(pixel);
}


/**
 * Get Pixel red attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#red @endverbatim
 *
 * @param self this object
 * @return the red value
 */
DEF_ATTR_READER(Pixel, red, int)

/**
 * Get Pixel green attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#green @endverbatim
 *
 * @param self this object
 * @return the green value
 */
DEF_ATTR_READER(Pixel, green, int)

/**
 * Get Pixel blue attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#blue @endverbatim
 *
 * @param self this object
 * @return the blue value
 */
DEF_ATTR_READER(Pixel, blue, int)

/**
 * Get Pixel opacity attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#opacity @endverbatim
 *
 * @param self this object
 * @return the opacity value
 */
DEF_ATTR_READER(Pixel, opacity, int)

/**
 * Set Pixel red attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#red= @endverbatim
 *
 * Notes:
 *   - Pixel is Observable. Setters call changed, notify_observers
 *   - Setters return their argument values for backward compatibility to when
 *     Pixel was a Struct class.
 *
 * @param self this object
 * @param v the red value
 * @return self
 */
DEF_PIXEL_CHANNEL_WRITER(red)

/**
 * Set Pixel green attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#green= @endverbatim
 *
 * Notes:
 *   - Pixel is Observable. Setters call changed, notify_observers
 *   - Setters return their argument values for backward compatibility to when
 *     Pixel was a Struct class.
 *
 * @param self this object
 * @param v the green value
 * @return self
 */
DEF_PIXEL_CHANNEL_WRITER(green)

/**
 * Set Pixel blue attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#blue= @endverbatim
 *
 * Notes:
 *   - Pixel is Observable. Setters call changed, notify_observers
 *   - Setters return their argument values for backward compatibility to when
 *     Pixel was a Struct class.
 *
 * @param self this object
 * @param v the blue value
 * @return self
 */
DEF_PIXEL_CHANNEL_WRITER(blue)

/**
 * Set Pixel opacity attribute.
 *
 * Ruby usage:
 *   - @verbatim Pixel#opacity= @endverbatim
 *
 * Notes:
 *   - Pixel is Observable. Setters call changed, notify_observers
 *   - Setters return their argument values for backward compatibility to when
 *     Pixel was a Struct class.
 *
 * @param self this object
 * @param v the opacity value
 * @return self
 */
DEF_PIXEL_CHANNEL_WRITER(opacity)


/*
 * Get/set Pixel CMYK attributes.
 */
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(cyan, red)
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(magenta, green)
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(yellow, blue)
DEF_PIXEL_CMYK_CHANNEL_ACCESSOR(black, opacity)


/**
 * Raise ArgumentError if the color name cannot be converted to a string via
 * rb_str_to_str.
 *
 * No Ruby usage (internal function)
 *
 * @param arg the argument to convert
 * @return 0
 * @throw ArgumentError
 */
static VALUE
color_arg_rescue(VALUE arg)
{
    rb_raise(rb_eTypeError, "argument must be color name or pixel (%s given)",
            rb_class2name(CLASS_OF(arg)));
    return (VALUE)0;
}


/**
 * Convert either a String color name or a Magick::Pixel to a PixelPacket.
 *
 * No Ruby usage (internal function)
 *
 * @param pp the PixelPacket to modify
 * @param color the color name or Magick::Pixel
 */
void
Color_to_PixelPacket(PixelPacket *pp, VALUE color)
{
    Pixel *pixel;

    // Allow color name or Pixel
    if (CLASS_OF(color) == Class_Pixel)
    {
        Data_Get_Struct(color, Pixel, pixel);
        *pp = *pixel;
    }
    else
    {
        // require 'to_str' here instead of just 'to_s'.
        color = rb_rescue(rb_str_to_str, color, color_arg_rescue, color);
        Color_Name_to_PixelPacket(pp, color);
    }
}


/**
 * Convert a color name to a PixelPacket
 *
 * No Ruby usage (internal function)
 *
 * @param color the PixelPacket to modify
 * @param name_arg the coor name
 * @throw ArgumentError
 */
static void
Color_Name_to_PixelPacket(PixelPacket *color, VALUE name_arg)
{
    MagickBooleanType okay;
    char *name;
    ExceptionInfo exception;

    GetExceptionInfo(&exception);
    name = StringValuePtr(name_arg);
    okay = QueryColorDatabase(name, color, &exception);
    (void) DestroyExceptionInfo(&exception);
    if (!okay)
    {
        rb_raise(rb_eArgError, "invalid color name %s", name);
    }
}



/**
 * Allocate a Pixel object.
 *
 * No Ruby usage (internal function)
 *
 * @param class the Ruby class to use
 * @return a new Magick::Pixel object
 */
VALUE
Pixel_alloc(VALUE class)
{
    Pixel *pixel;

    pixel = ALLOC(Pixel);
    memset(pixel, '\0', sizeof(Pixel));
    return Data_Wrap_Struct(class, NULL, destroy_Pixel, pixel);
}


/**
 * "Case equal" operator for Pixel.
 *
 * Ruby usage:
 *   - @verbatim Pixel#=== @endverbatim
 *
 * @param self this object
 * @param other the other object
 * @return true or false
 */

VALUE
Pixel_case_eq(VALUE self, VALUE other)
{
    Pixel *this, *that;

    if (CLASS_OF(self) == CLASS_OF(other))
    {
        Data_Get_Struct(self, Pixel, this);
        Data_Get_Struct(other, Pixel, that);
        return (this->red == that->red
            && this->blue == that->blue
            && this->green == that->green
            && this->opacity == that->opacity) ? Qtrue : Qfalse;
    }

    return Qfalse;
}


/**
 * Clone a Pixel.
 *
 * Ruby usage:
 *   - @verbatim Pixel#clone @endverbatim
 *
 * @param self this object
 * @return a clone
 * @see Pixel_dup
 * @see Pixel_init_copy
 */
VALUE
Pixel_clone(VALUE self)
{
    volatile VALUE clone;

    clone = Pixel_dup(self);
    if (OBJ_FROZEN(self))
    {
        OBJ_FREEZE(clone);
    }

    return clone;
}


/**
 * Duplicate a Pixel.
 *
 * Ruby usage:
 *   - @verbatim Pixel#dup @endverbatim
 *
 * @param self this object
 * @return a clone
 * @see Pixel_clone
 * @see Pixel_init_copy
 */
VALUE
Pixel_dup(VALUE self)
{
    Pixel *pixel;
    volatile VALUE dup;

    pixel = ALLOC(Pixel);
    memset(pixel, '\0', sizeof(Pixel));
    dup = Data_Wrap_Struct(CLASS_OF(self), NULL, destroy_Pixel, pixel);
    if (rb_obj_tainted(self))
    {
        (void) rb_obj_taint(dup);
    }
    return rb_funcall(dup, rm_ID_initialize_copy, 1, self);
}


/**
 * For use with Hash.
 *
 * Ruby usage:
 *   - @verbatim Pixel#eql? @endverbatim
 *
 * @param self this object
 * @param other the other object
 * @return true if hash to the same value, otherwise false
 */
VALUE
Pixel_eql_q(VALUE self, VALUE other)
{
    return NUM2INT(Pixel_spaceship(self, other)) == 0 ? Qtrue : Qfalse;
}


/**
 * Compare pixel values for equality.
 *
 * Ruby usage:
 *   - @verbatim Pixel#fcmp(other, fuzz, colorspace) @endverbatim
 *
 * Notes:
 *   - Default fuzz is 0.0
 *   - Default colorspace is RGBColorspace
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return true if equal, otherwise false
 */
VALUE
Pixel_fcmp(int argc, VALUE *argv, VALUE self)
{
    Image *image;
    Info *info;

    Pixel *this, *that;
    ColorspaceType colorspace = RGBColorspace;
    double fuzz = 0.0;
    unsigned int equal;

    switch (argc)
    {
        case 3:
            VALUE_TO_ENUM(argv[2], colorspace, ColorspaceType);
        case 2:
            fuzz = NUM2DBL(argv[1]);
        case 1:
            // Allow 1 argument
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 1 to 3)", argc);
            break;
    }

    Data_Get_Struct(self, Pixel, this);
    Data_Get_Struct(argv[0], Pixel, that);

    // The IsColorSimilar function expects to get the
    // colorspace and fuzz parameters from an Image structure.

    info = CloneImageInfo(NULL);
    if (!info)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    image = AcquireImage(info);

    // Delete Info now in case we have to raise an exception
    (void) DestroyImageInfo(info);

    if (!image)
    {
        rb_raise(rb_eNoMemError, "not enough memory to continue");
    }

    image->colorspace = colorspace;
    image->fuzz = fuzz;

    equal = IsColorSimilar(image, this, that);
    (void) DestroyImage(image);

    return equal ? Qtrue : Qfalse;
}


/**
 * Construct an Magick::Pixel corresponding to the given color name.
 *
 * Ruby usage:
 *   - @verbatim Magick::Pixel.from_color(string) @endverbatim
 *
 * Notes:
 *   - The "inverse" is Image_to_color, b/c the conversion of a pixel to a
 *     color name requires both a color depth and if the opacity value has
 *     meaning (i.e. whether image->matte == True or not).
 *
 * @param class the Ruby class to use
 * @param name the color name
 * @return a new Magic::Pixel object
 * @see Image_to_color
 * @see Pixel_to_color
 */
VALUE
Pixel_from_color(VALUE class, VALUE name)
{
    PixelPacket pp;
    ExceptionInfo exception;
    MagickBooleanType okay;

    class = class;      // defeat "never referenced" message from icc

    GetExceptionInfo(&exception);
    okay = QueryColorDatabase(StringValuePtr(name), &pp, &exception);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    if (!okay)
    {
        rb_raise(rb_eArgError, "invalid color name: %s", StringValuePtr(name));
    }

    return Pixel_from_PixelPacket(&pp);
}


/**
 * Construct an RGB pixel.
 *
 * Ruby usage:
 *   - @verbatim Pixel#from_hsla(hue, saturation, lightness) @endverbatim
 *   - @verbatim Pixel#from_hsla(hue, saturation, lightness, alpha) @endverbatim
 *
 * Notes:
 *   - Default alpha is 1.0
 *   - 0 <= hue < 360 OR "0%" <= hue < "100%"
 *   - 0 <= saturation <= 255 OR "0%" <= saturation <= "100%"
 *   - 0 <= lightness <= 255 OR "0%" <= lightness <= "100%"
 *   - 0 <= alpha <= 1 (0 is transparent, 1 is opaque) OR "0%" <= alpha <= "100%"
 *   - Replaces brain-dead Pixel_from_HSL.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param class the Ruby class to use
 * @return a new Magick::Pixel object
 */
VALUE
Pixel_from_hsla(int argc, VALUE *argv, VALUE class)
{
    double h, s, l, a = 1.0;
    MagickPixelPacket pp;
    ExceptionInfo exception;
    char name[50];
    MagickBooleanType alpha = MagickFalse;

    class = class;          // defeat "unused parameter" message.

    switch (argc)
    {
        case 4:
            a = rm_percentage(argv[3],1.0);
            alpha = MagickTrue;
        case 3:
            // saturation and lightness are out of 255 in new ImageMagicks and
            // out of 100 in old ImageMagicks. Compromise: always use %.
            l = rm_percentage(argv[2],255.0); 
            s = rm_percentage(argv[1],255.0);
            h = rm_percentage(argv[0],360.0);
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 3 or 4)", argc);
            break;
    }

    if (alpha && (a < 0.0 || a > 1.0))
    {
        rb_raise(rb_eRangeError, "alpha %g out of range [0.0, 1.0]", a);
    }
    if (l < 0.0 || l > 255.0)
    {
        rb_raise(rb_eRangeError, "lightness %g out of range [0.0, 255.0]", l);
    }
    if (s < 0.0 || s > 255.0)
    {
        rb_raise(rb_eRangeError, "saturation %g out of range [0.0, 255.0]", s);
    }
    if (h < 0.0 || h >= 360.0)
    {
        rb_raise(rb_eRangeError, "hue %g out of range [0.0, 360.0)", h);
    }

    // Ugly way of checking for change in ImageMagick 6.5.6-5 to see whether
    // saturation/lightness should be out of 255 or out of 100.
    if(MagickLibVersion < 0x656 ||
        (MagickLibVersion == 0x656 && strcmp(MagickLibSubversion,"-5") <= 0) )
    {
      s = s/2.55;
      l = l/2.55;
    }

    memset(name, 0, sizeof(name));
    if (alpha)
    {
        sprintf(name, "hsla(%-2.1f,%-2.1f,%-2.1f,%-2.1f)", h, s, l, a);
    }
    else
    {
        sprintf(name, "hsl(%-2.1f,%-2.1f,%-2.1f)", h, s, l);
    }

    GetExceptionInfo(&exception);

    (void) QueryMagickColor(name, &pp, &exception);
    CHECK_EXCEPTION()

    (void) DestroyExceptionInfo(&exception);

    return Pixel_from_MagickPixelPacket(&pp);
}


/**
 * Construct an RGB pixel from the array [hue, saturation, luminosity].
 *
 * Ruby usage:
 *   - @verbatim Pixel.from_HSL  @endverbatim
 *
 * @param class the Ruby class to use
 * @param hsl the array
 * @return a new Magick::Pixel object
 * @deprecated This method has been deprecated. Please use Pixel_from_hsla.
 */
VALUE
Pixel_from_HSL(VALUE class, VALUE hsl)
{
    PixelPacket rgb;
    double hue, saturation, luminosity;

    class = class;      // defeat "never referenced" message from icc
    memset(&rgb, 0, sizeof(rgb));

    hsl = rb_Array(hsl);    // Ensure array
    if (RARRAY_LEN(hsl) < 3)
    {
        rb_raise(rb_eArgError, "array argument must have at least 3 elements");
    }

    hue        = NUM2DBL(rb_ary_entry(hsl, 0));
    saturation = NUM2DBL(rb_ary_entry(hsl, 1));
    luminosity = NUM2DBL(rb_ary_entry(hsl, 2));

    rb_warning("Pixel#from_HSL is deprecated; use from_hsla");
    ConvertHSLToRGB(hue, saturation, luminosity,
                 &rgb.red, &rgb.green, &rgb.blue);
    return Pixel_from_PixelPacket(&rgb);
}


/**
 * Create a Magick::Pixel object from a MagickPixelPacket structure.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Bypasses normal Pixel.new, Pixel#initialize methods
 *
 * @param pp the MagickPixelPacket
 * @return a new Magick::Pixel object
 */
VALUE
Pixel_from_MagickPixelPacket(const MagickPixelPacket *pp)
{
    Pixel *pixel;

    pixel          = ALLOC(Pixel);
    pixel->red     = ROUND_TO_QUANTUM(pp->red);
    pixel->green   = ROUND_TO_QUANTUM(pp->green);
    pixel->blue    = ROUND_TO_QUANTUM(pp->blue);
    pixel->opacity = ROUND_TO_QUANTUM(pp->opacity);

    return Data_Wrap_Struct(Class_Pixel, NULL, destroy_Pixel, pixel);
}


/**
 * Create a Magick::Pixel object from a PixelPacket structure.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Bypasses normal Pixel.new, Pixel#initialize methods
 *
 * @param pp the PixelPacket
 * @return a new Magick::Pixel object
 */
VALUE
Pixel_from_PixelPacket(const PixelPacket *pp)
{
    Pixel *pixel;

    pixel = ALLOC(Pixel);
    *pixel = *pp;
    return Data_Wrap_Struct(Class_Pixel, NULL, destroy_Pixel, pixel);
}


/**
 * Ruby usage:
 *   - @verbatim Pixel#hash @endverbatim
 *
 * Notes:
 *   - INT2FIX left-shifts 1 bit. Sacrifice 1 bit from the opacity attribute to
 *     the FIXNUM_FLAG.
 *
 * @param self this object
 * @return the hash of self
 */
VALUE
Pixel_hash(VALUE self)
{
    Pixel *pixel;
    unsigned int hash;

    Data_Get_Struct(self, Pixel, pixel);

    hash  = ScaleQuantumToChar(pixel->red)   << 24;
    hash += ScaleQuantumToChar(pixel->green) << 16;
    hash += ScaleQuantumToChar(pixel->blue)  << 8;
    hash += ScaleQuantumToChar(pixel->opacity);
    hash >>= 1;

    return INT2FIX(hash);

}


/**
 * Initialize clone, dup methods.
 *
 * Ruby usage:
 *   - @verbatim Pixel#initialize_copy @endverbatim
 *
 * @param self this object
 * @param orig the original Pixel
 * @return self
 * @see Pixel_clone
 * @see Pixel_dup
 */
VALUE
Pixel_init_copy(VALUE self, VALUE orig)
{
    Pixel *copy, *original;

    Data_Get_Struct(orig, Pixel, original);
    Data_Get_Struct(self, Pixel, copy);

    *copy = *original;

    return self;
}


/**
 * Ruby usage:
 *   - @verbatim Pixel#initialize @endverbatim
 *   - @verbatim Pixel#initialize(red) @endverbatim
 *   - @verbatim Pixel#initialize(red,green) @endverbatim
 *   - @verbatim Pixel#initialize(red,green,blue) @endverbatim
 *   - @verbatim Pixel#initialize(red,green,blue,opacity) @endverbatim
 *
 * Notes:
 *   - Default red is 0.0
 *   - Default green is 0.0
 *   - Default blue is 0.0
 *   - Default opacity is 0.0
 *   - For backward compatibility, arguments may be nil.
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return self
 */
VALUE
Pixel_initialize(int argc, VALUE *argv, VALUE self)
{
    Pixel *pixel;

    Data_Get_Struct(self, Pixel, pixel);

    switch(argc)
    {
        case 4:
            if (argv[3] != Qnil)
            {
                pixel->opacity = APP2QUANTUM(argv[3]);
            }
        case 3:
            if (argv[2] != Qnil)
            {
                pixel->blue = APP2QUANTUM(argv[2]);
            }
        case 2:
            if (argv[1] != Qnil)
            {
                pixel->green = APP2QUANTUM(argv[1]);
            }
        case 1:
            if (argv[0] != Qnil)
            {
                pixel->red = APP2QUANTUM(argv[0]);
            }
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 4)", argc);
    }

    return self;
}


/**
 * Return the "intensity" of a pixel.
 *
 * Ruby usage:
 *   - @verbatim Pixel#intensity @endverbatim
 *
 * @param self this object
 * @return the intensity
 */
VALUE
Pixel_intensity(VALUE self)
{
    Pixel *pixel;
    Quantum intensity;

    Data_Get_Struct(self, Pixel, pixel);

    intensity = ROUND_TO_QUANTUM((0.299*pixel->red)
                                + (0.587*pixel->green)
                                + (0.114*pixel->blue));

    return QUANTUM2NUM((unsigned long) intensity);
}


/**
 * Support Marshal.dump.
 *
 * Ruby usage:
 *   - @verbatim Pixel#marshal_dump @endverbatim
 *
 * @param self this object
 * @return a string representing the dumped pixel
 */
VALUE
Pixel_marshal_dump(VALUE self)
{
    Pixel *pixel;
    volatile VALUE dpixel;

    Data_Get_Struct(self, Pixel, pixel);
    dpixel = rb_hash_new();
    rb_hash_aset(dpixel, CSTR2SYM("red"), QUANTUM2NUM(pixel->red));
    rb_hash_aset(dpixel, CSTR2SYM("green"), QUANTUM2NUM(pixel->green));
    rb_hash_aset(dpixel, CSTR2SYM("blue"), QUANTUM2NUM(pixel->blue));
    rb_hash_aset(dpixel, CSTR2SYM("opacity"), QUANTUM2NUM(pixel->opacity));
    return dpixel;
}


/**
 * Support Marshal.load.
 *
 * Ruby usage:
 *   - @verbatim Pixel#marshal_load @endverbatim
 *
 * @param self this object
 * @param dpixel the dumped pixel
 * @return self
 */
VALUE
Pixel_marshal_load(VALUE self, VALUE dpixel)
{
    Pixel *pixel;

    Data_Get_Struct(self, Pixel, pixel);
    pixel->red = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("red")));
    pixel->green = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("green")));
    pixel->blue = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("blue")));
    pixel->opacity = NUM2QUANTUM(rb_hash_aref(dpixel, CSTR2SYM("opacity")));
    return self;
}


/**
 * Support Comparable mixin.
 *
 * Ruby usage:
 *   - @verbatim Pixel#<=> @endverbatim
 *
 * @param self this object
 * @param other the other Pixel
 * @return -1, 0, 1
 */
VALUE
Pixel_spaceship(VALUE self, VALUE other)
{
    Pixel *this, *that;

    Data_Get_Struct(self, Pixel, this);
    Data_Get_Struct(other, Pixel, that);

    if (this->red != that->red)
    {
        return INT2NUM((this->red - that->red)/abs(this->red - that->red));
    }
    else if(this->green != that->green)
    {
        return INT2NUM((this->green - that->green)/abs(this->green - that->green));
    }
    else if(this->blue != that->blue)
    {
        return INT2NUM((this->blue - that->blue)/abs(this->blue - that->blue));
    }
    else if(this->opacity != that->opacity)
    {
        return INT2NUM((this->opacity - that->opacity)/abs(this->opacity - that->opacity));
    }

    // Values are equal, check class.

    return rb_funcall(CLASS_OF(self), rb_intern("<=>"), 1, CLASS_OF(other));

}


/**
 * Return [hue, saturation, lightness, alpha] in the same ranges as
 * Pixel_from_hsla.
 * 
 *
 * Ruby usage:
 *   - @verbatim Pixel#to_hsla @endverbatim
 *
 * Notes:
 *   - Replace brain-dead Pixel_to_HSL.
 *
 * @param self this object
 * @return an array with hsla data
 * @see Pixel_from_hsla
 */
VALUE
Pixel_to_hsla(VALUE self)
{
    double hue, sat, lum, alpha;
    Pixel *pixel;
    volatile VALUE hsla;

    Data_Get_Struct(self, Pixel, pixel);

    ConvertRGBToHSL(pixel->red, pixel->green, pixel->blue, &hue, &sat, &lum);
    hue *= 360.0;
    sat *= 255.0;
    lum *= 255.0;

    if (pixel->opacity == OpaqueOpacity)
    {
        alpha = 1.0;
    }
    else if (pixel->opacity == TransparentOpacity)
    {
        alpha = 0.0;
    }
    else
    {
        alpha = ROUND_TO_QUANTUM(QuantumRange - (pixel->opacity / QuantumRange));
    }

    hsla = rb_ary_new3(4, rb_float_new(hue), rb_float_new(sat), rb_float_new(lum), rb_float_new(alpha));
    return hsla;
}

/**
 * Convert an RGB pixel to the array [hue, saturation, luminosity].
 *
 * Ruby usage:
 *   - @verbatim Pixel#to_HSL @endverbatim
 *
 * @param self this object
 * @return an array with hsl data
 * @deprecated This method has been deprecated. Please use Pixel_to_hsla.
 */
VALUE
Pixel_to_HSL(VALUE self)
{
    Pixel *pixel;
    double hue, saturation, luminosity;
    volatile VALUE hsl;

    Data_Get_Struct(self, Pixel, pixel);

    rb_warning("Pixel#to_HSL is deprecated; use to_hsla");
    ConvertRGBToHSL(pixel->red, pixel->green, pixel->blue, &hue, &saturation, &luminosity);

    hsl = rb_ary_new3(3, rb_float_new(hue), rb_float_new(saturation),
                      rb_float_new(luminosity));

    return hsl;
}


/**
 * Return the color name corresponding to the pixel values.
 *
 * Ruby usage:
 *   - @verbatim Magick::Pixel#to_color @endverbatim
 *   - @verbatim Magick::Pixel#to_color(compliance) @endverbatim
 *   - @verbatim Magick::Pixel#to_color(compliance, matte) @endverbatim
 *   - @verbatim Magick::Pixel#to_color(compliance, matte, depth) @endverbatim
 *   - @verbatim Magick::Pixel#to_color(compliance, matte, depth, hex) @endverbatim
 *
 * Notes:
 *   - Default compliance is AllCompliance
 *   - Default matte is false
 *   - Default depth is QuantumDepth
 *   - Default hex is false
 *   - The conversion respects the value of the 'opacity' field in the Pixel
 *
 * @param argc number of input arguments
 * @param argv array of input arguments
 * @param self this object
 * @return the color name as a String
 */
VALUE
Pixel_to_color(int argc, VALUE *argv, VALUE self)
{
    Info *info;
    Image *image;
    Pixel *pixel;
    MagickPixelPacket mpp;
    MagickBooleanType hex = MagickFalse;
    char name[MaxTextExtent];
    ExceptionInfo exception;
    ComplianceType compliance = AllCompliance;
    unsigned int matte = MagickFalse;
    unsigned int depth = QuantumDepth;

    switch (argc)
    {
        case 4:
            hex = RTEST(argv[3]);
        case 3:
            depth = NUM2UINT(argv[2]);

            // Ensure depth is appropriate for the way xMagick was compiled.
            switch (depth)
            {
                case 8:
#if QuantumDepth == 16 || QuantumDepth == 32
                case 16:
#endif
#if QuantumDepth == 32
                case 32:
#endif
                    break;
                default:
                    rb_raise(rb_eArgError, "invalid depth (%d)", depth);
                    break;
            }
       case 2:
            matte = RTEST(argv[1]);
        case 1:
            VALUE_TO_ENUM(argv[0], compliance, ComplianceType);
        case 0:
            break;
        default:
            rb_raise(rb_eArgError, "wrong number of arguments (%d for 0 to 2)", argc);
    }

    Data_Get_Struct(self, Pixel, pixel);

    info = CloneImageInfo(NULL);
    image = AcquireImage(info);
    image->depth = depth;
    image->matte = matte;
    (void) DestroyImageInfo(info);

    GetMagickPixelPacket(image, &mpp);
    rm_set_magick_pixel_packet(pixel, &mpp);

    GetExceptionInfo(&exception);

#if defined(HAVE_NEW_QUERYMAGICKCOLORNAME)
    // Support for hex-format color names moved out of QueryMagickColorname
    // in 6.4.1-9. The 'hex' argument was removed as well.
    if (hex)
    {
        if (compliance == XPMCompliance)
        {
            mpp.matte = MagickFalse;
            mpp.depth = (unsigned long) min(1.0 * image->depth, 16.0);
        }
        (void) GetColorTuple(&mpp, MagickTrue, name);
    }
    else
    {
        (void) QueryMagickColorname(image, &mpp, compliance, name, &exception);
    }
#else
    (void) QueryMagickColorname(image, &mpp, compliance, hex, name, &exception);
#endif
    (void) DestroyImage(image);
    CHECK_EXCEPTION()
    (void) DestroyExceptionInfo(&exception);

    // Always return a string, even if it's ""
    return rb_str_new2(name);
}


/**
 * Create a string representation of a Magick::Pixel.
 *
 * Ruby usage:
 *   - @verbatim Magick::Pixel#to_s @endverbatim
 *
 * @param self this object
 * @return the string
 */
VALUE
Pixel_to_s(VALUE self)
{
    Pixel *pixel;
    char buff[100];

    Data_Get_Struct(self, Pixel, pixel);
    sprintf(buff, "red=" QuantumFormat ", green=" QuantumFormat ", blue=" QuantumFormat ", opacity=" QuantumFormat
          , pixel->red, pixel->green, pixel->blue, pixel->opacity);
    return rb_str_new2(buff);
}


/**
 * Convert a PixelPacket to a MagickPixelPacket.
 *
 * No Ruby usage (internal function)
 *
 * Notes:
 *   - Same code as the private function SetMagickPixelPacket in ImageMagick.
 *
 * @param pixel the pixel
 * @param pp the MagickPixelPacket to be modified
 */
void
rm_set_magick_pixel_packet(Pixel *pixel, MagickPixelPacket *pp)
{
    pp->red     = (MagickRealType) pixel->red;
    pp->green   = (MagickRealType) pixel->green;
    pp->blue    = (MagickRealType) pixel->blue;
    pp->opacity = (MagickRealType) pixel->opacity;
    pp->index   = (MagickRealType) 0.0;
}

