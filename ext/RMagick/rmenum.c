/**************************************************************************//**
 * Enumeration methods.
 *
 * Copyright &copy; 2002 - 2009 by Timothy P. Hunter
 *
 * Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or
 *
 * @file     rmenum.c
 * @version  $Id: rmenum.c,v 1.9 2009/12/20 02:33:33 baror Exp $
 * @author   Tim Hunter
 ******************************************************************************/

#include "rmagick.h"


#define ENUMERATORS_CLASS_VAR "@@enumerators"


static VALUE Enum_type_values(VALUE);
static VALUE Enum_type_inspect(VALUE);





/**
 * Set up a subclass of Enum.
 *
 * No Ruby usage (internal function)
 *
 * @param tag the name of the subclass
 * @return the subclass
 */
VALUE
rm_define_enum_type(const char *tag)
{
    VALUE class;

    class = rb_define_class_under(Module_Magick, tag, Class_Enum);\

    rb_define_singleton_method(class, "values", Enum_type_values, 0);
    rb_define_method(class, "initialize", Enum_type_initialize, 2);
    rb_define_method(class, "inspect", Enum_type_inspect, 0);
    return class;
}


/**
 * Construct a new Enum subclass instance.
 *
 * No Ruby usage (internal function)
 *
 * @param class the subclass
 * @param sym the symbol
 * @param val the value for the symbol
 * @return a new instance of class
 */
VALUE
rm_enum_new(VALUE class, VALUE sym, VALUE val)
{
    VALUE argv[2];

    argv[0] = sym;
    argv[1] = val;
    return rb_obj_freeze(rb_class_new_instance(2, argv, class));
}


/**
 * Enum class alloc function.
 *
 * No Ruby usage (internal function)
 *
 * @param class the Ruby class to use
 * @return a new enumerator
 */
VALUE
Enum_alloc(VALUE class)
{
   MagickEnum *magick_enum;
   volatile VALUE enumr;

   enumr = Data_Make_Struct(class, MagickEnum, NULL, NULL, magick_enum);
   rb_obj_freeze(enumr);
   return enumr;
}


/**
 * "Case equal" operator for Enum.
 *
 * Ruby usage:
 *   - @verbatim Enum#=== @endverbatim
 *
 * Notes:
 *   - Yes, I know "case equal" is a misnomer.
 *
 * @param self this object
 * @param other the other object
 * @return true or false
 */
VALUE
Enum_case_eq(VALUE self, VALUE other)
{
    MagickEnum *this, *that;

    if (CLASS_OF(self) == CLASS_OF(other))
    {
        Data_Get_Struct(self, MagickEnum, this);
        Data_Get_Struct(other, MagickEnum, that);
        return this->val == that->val ? Qtrue : Qfalse;
    }

    return Qfalse;
}


/**
 * Initialize a new Enum instance.
 *
 * Ruby usage:
 *   - @verbatim Enum#initialize(sym,val) @endverbatim
 *
 * @param self this object
 * @param sym the symbol
 * @param val the value for the symbol
 * @return self
 */
VALUE
Enum_initialize(VALUE self, VALUE sym, VALUE val)
{
   MagickEnum *magick_enum;

   Data_Get_Struct(self, MagickEnum, magick_enum);
   magick_enum->id = rb_to_id(sym); /* convert symbol to ID */
   magick_enum->val = NUM2INT(val);

   return self;
}


/**
 * Return the value of an enum.
 *
 * Ruby usage:
 *   - @verbatim Enum#to_i @endverbatim
 *
 * @param self this object
 * @return this object's value
 */
VALUE
Enum_to_i(VALUE self)
{
   MagickEnum *magick_enum;

   Data_Get_Struct(self, MagickEnum, magick_enum);
   return INT2NUM(magick_enum->val);
}


/**
 * Support Comparable module in Enum.
 *
 * Ruby usage:
 *   - @verbatim Enum#<=> @endverbatim
 *
 * Notes:
 *   - Enums must be instances of the same class to be equal.
 *
 * @param self this object
 * @param other the other object
 * @return -1, 0, 1, or nil
 */
VALUE
Enum_spaceship(VALUE self, VALUE other)
{
    MagickEnum *this, *that;

    Data_Get_Struct(self, MagickEnum, this);
    Data_Get_Struct(other, MagickEnum, that);

    if (this->val > that->val)
    {
        return INT2FIX(1);
    }
    else if (this->val < that->val)
    {
        return INT2FIX(-1);
    }

    // Values are equal, check class.

    return rb_funcall(CLASS_OF(self), rm_ID_spaceship, 1, CLASS_OF(other));
}


/**
 * Return the name of an enum.
 *
 * Ruby usage:
 *   - @verbatim Enum#to_s @endverbatim
 *
 * @param self this object
 * @return the name
 */
VALUE
Enum_to_s(VALUE self)
{
   MagickEnum *magick_enum;

   Data_Get_Struct(self, MagickEnum, magick_enum);
   return rb_str_new2(rb_id2name(magick_enum->id));
}


/**
 * Initialize method for all Enum subclasses.
 *
 * Ruby usage:
 *   - @verbatim xxx#initialize(sym,val) @endverbatim
 *
 * @param self this object
 * @param sym the symbol
 * @param val the value of the symbol
 * @return self
 */
VALUE
Enum_type_initialize(VALUE self, VALUE sym, VALUE val)
{
    VALUE super_argv[2];
    volatile VALUE enumerators;

    super_argv[0] = sym;
    super_argv[1] = val;
    (void) rb_call_super(2, (const VALUE *)super_argv);

    if (rb_cvar_defined(CLASS_OF(self), rb_intern(ENUMERATORS_CLASS_VAR)) != Qtrue)
    {
        rb_cv_set(CLASS_OF(self), ENUMERATORS_CLASS_VAR, rb_ary_new());
    }

    enumerators = rb_cv_get(CLASS_OF(self), ENUMERATORS_CLASS_VAR);
    (void) rb_ary_push(enumerators, self);

    return self;
}


/**
 * Enum subclass #inspect.
 *
 * Ruby usage:
 *   - @verbatim xxx#inspect @endverbatim
 *
 * @param self this object
 * @return string representation of self
 */
static VALUE
Enum_type_inspect(VALUE self)
{
    char str[100];
    MagickEnum *magick_enum;

    Data_Get_Struct(self, MagickEnum, magick_enum);
    sprintf(str, "%.48s=%d", rb_id2name(magick_enum->id), magick_enum->val);

    return rb_str_new2(str);
}


/**
 * Behaves like #each if a block is present, otherwise like #to_a.
 *
 * Ruby usage:
 *   - @verbatim xxx.values @endverbatim
 *   - @verbatim xxx.values {|v| } @endverbatim
 *
 * Notes:
 *   - Defined for each Enum subclass
 *
 * @param class the subclass
 * @return iterator over values if given block, a copy of the values otherwise
 */
static VALUE
Enum_type_values(VALUE class)
{
    volatile VALUE enumerators, copy;
    volatile VALUE rv;
    int x;

    enumerators = rb_cv_get(class, ENUMERATORS_CLASS_VAR);

    if (rb_block_given_p())
    {
        for (x = 0; x < RARRAY_LEN(enumerators); x++)
        {
            (void) rb_yield(rb_ary_entry(enumerators, x));
        }
        rv = class;
    }
    else
    {
        copy = rb_ary_new2(RARRAY_LEN(enumerators));
        for (x = 0; x < RARRAY_LEN(enumerators); x++)
        {
            (void) rb_ary_push(copy, rb_ary_entry(enumerators, x));
        }
        rb_obj_freeze(copy);
        rv = copy;
    }

    return rv;
}


/**
 * Construct a ClassType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param cls the class type
 * @return a new enumerator
 */
VALUE
ClassType_new(ClassType cls)
{
    const char *name;

    switch(cls)
    {
        default:
        case UndefinedClass:
            name = "UndefineClass";
            break;
        case DirectClass:
            name = "DirectClass";
            break;
        case PseudoClass:
            name = "PseudoClass";
            break;
    }

    return rm_enum_new(Class_ClassType, ID2SYM(rb_intern(name)), INT2FIX(cls));
}


/**
 * Construct a ColorspaceType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param cs the ColorspaceType
 * @return a new ColorspaceType enumerator
 */
VALUE
ColorspaceType_new(ColorspaceType cs)
{
    const char *name;

    switch(cs)
    {
        default:
        case UndefinedColorspace:
            name = "UndefinedColorspace";
            break;
        case RGBColorspace:
            name = "RGBColorspace";
            break;
        case GRAYColorspace:
            name = "GRAYColorspace";
            break;
        case TransparentColorspace:
            name = "TransparentColorspace";
            break;
        case OHTAColorspace:
            name = "OHTAColorspace";
            break;
        case XYZColorspace:
            name = "XYZColorspace";
            break;
        case YCbCrColorspace:
            name = "YCbCrColorspace";
            break;
        case YCCColorspace:
            name = "YCCColorspace";
            break;
        case YIQColorspace:
            name = "YIQColorspace";
            break;
        case YPbPrColorspace:
            name = "YPbPrColorspace";
            break;
        case YUVColorspace:
            name = "YUVColorspace";
            break;
        case CMYKColorspace:
            name = "CMYKColorspace";
            break;
        case sRGBColorspace:
            name = "sRGBColorspace";
            break;
        case HSLColorspace:
            name = "HSLColorspace";
            break;
        case HWBColorspace:
            name = "HWBColorspace";
            break;
        case HSBColorspace:
            name = "HSBColorspace";
            break;
        case LABColorspace:
            name = "LABColorspace";
            break;
        case Rec601YCbCrColorspace:
            name = "Rec601YCbCrColorspace";
            break;
        case Rec601LumaColorspace:
            name = "Rec601LumaColorspace";
            break;
        case Rec709LumaColorspace:
            name = "Rec709LumaColorspace";
            break;
        case Rec709YCbCrColorspace:
            name = "Rec709YCbCrColorspace";
            break;
        case LogColorspace:
            name = "LogColorspace";
            break;
        case CMYColorspace:
            name = "CMYColorspace";
            break;
    }

    return rm_enum_new(Class_ColorspaceType, ID2SYM(rb_intern(name)), INT2FIX(cs));

}


/**
 * Return the name of a CompositeOperator enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param op the CompositeOperator
 * @return the name
 */
static const char *
CompositeOperator_name(CompositeOperator op)
{
    switch (op)
    {
        ENUM_TO_NAME(UndefinedCompositeOp)
        ENUM_TO_NAME(NoCompositeOp)
        ENUM_TO_NAME(AddCompositeOp)
        ENUM_TO_NAME(AtopCompositeOp)
#if defined(HAVE_ENUM_BLURCOMPOSITEOP)
        ENUM_TO_NAME(BlurCompositeOp)
#endif
        ENUM_TO_NAME(BumpmapCompositeOp)
        ENUM_TO_NAME(ChangeMaskCompositeOp)
        ENUM_TO_NAME(ClearCompositeOp)
        ENUM_TO_NAME(ColorBurnCompositeOp)
        ENUM_TO_NAME(BlendCompositeOp)
        ENUM_TO_NAME(ColorDodgeCompositeOp)
        ENUM_TO_NAME(ExclusionCompositeOp)
        ENUM_TO_NAME(HardLightCompositeOp)
        ENUM_TO_NAME(SoftLightCompositeOp)
        ENUM_TO_NAME(ColorizeCompositeOp)
        ENUM_TO_NAME(CopyBlueCompositeOp)
        ENUM_TO_NAME(CopyCompositeOp)
        ENUM_TO_NAME(CopyCyanCompositeOp)
        ENUM_TO_NAME(CopyMagentaCompositeOp)
        ENUM_TO_NAME(CopyYellowCompositeOp)
        ENUM_TO_NAME(CopyBlackCompositeOp)
        ENUM_TO_NAME(CopyGreenCompositeOp)
        ENUM_TO_NAME(CopyOpacityCompositeOp)
        ENUM_TO_NAME(CopyRedCompositeOp)
        ENUM_TO_NAME(DarkenCompositeOp)
#if defined(HAVE_ENUM_DISTORTCOMPOSITEOP)
        ENUM_TO_NAME(DistortCompositeOp)
#endif
        ENUM_TO_NAME(DivideCompositeOp)
        ENUM_TO_NAME(DstAtopCompositeOp)
        ENUM_TO_NAME(DstCompositeOp)
        ENUM_TO_NAME(DstInCompositeOp)
        ENUM_TO_NAME(DstOutCompositeOp)
        ENUM_TO_NAME(DstOverCompositeOp)
        ENUM_TO_NAME(DifferenceCompositeOp)
        ENUM_TO_NAME(DisplaceCompositeOp)
        ENUM_TO_NAME(DissolveCompositeOp)
        ENUM_TO_NAME(HueCompositeOp)
        ENUM_TO_NAME(InCompositeOp)
        ENUM_TO_NAME(LightenCompositeOp)
#if defined(HAVE_ENUM_LINEARBURNCOMPOSITEOP)
        ENUM_TO_NAME(LinearBurnCompositeOp)
#endif
#if defined(HAVE_ENUM_LINEARDODGECOMPOSITEOP)
        ENUM_TO_NAME(LinearDodgeCompositeOp)
#endif
        ENUM_TO_NAME(LinearLightCompositeOp)
        ENUM_TO_NAME(LuminizeCompositeOp)
#if defined(HAVE_ENUM_MATHEMATICSCOMPOSITEOP)
        ENUM_TO_NAME(MathematicsCompositeOp)
#endif
        ENUM_TO_NAME(MinusCompositeOp)
        ENUM_TO_NAME(ModulateCompositeOp)
        ENUM_TO_NAME(MultiplyCompositeOp)
        ENUM_TO_NAME(OutCompositeOp)
        ENUM_TO_NAME(OverCompositeOp)
        ENUM_TO_NAME(OverlayCompositeOp)
#if defined(HAVE_ENUM_PEGTOPLIGHTCOMPOSITEOP)
        ENUM_TO_NAME(PegtopLightCompositeOp)
#endif
#if defined(HAVE_ENUM_PINLIGHTCOMPOSITEOP)
        ENUM_TO_NAME(PinLightCompositeOp)
#endif
        ENUM_TO_NAME(PlusCompositeOp)
        ENUM_TO_NAME(ReplaceCompositeOp)
        ENUM_TO_NAME(SaturateCompositeOp)
        ENUM_TO_NAME(ScreenCompositeOp)
        ENUM_TO_NAME(SrcAtopCompositeOp)
        ENUM_TO_NAME(SrcCompositeOp)
        ENUM_TO_NAME(SrcInCompositeOp)
        ENUM_TO_NAME(SrcOutCompositeOp)
        ENUM_TO_NAME(SrcOverCompositeOp)
        ENUM_TO_NAME(SubtractCompositeOp)
        ENUM_TO_NAME(ThresholdCompositeOp)
#if defined(HAVE_ENUM_VIVIDLIGHTCOMPOSITEOP)
        ENUM_TO_NAME(VividLightCompositeOp)
#endif
        ENUM_TO_NAME(XorCompositeOp)
    }

    return "UndefinedCompositeOp";
}


/**
 * Construct a CompositeOperator enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param op the CompositeOperator
 * @return a new CompositeOperator enumerator
 */
VALUE
CompositeOperator_new(CompositeOperator op)
{
    const char *name = CompositeOperator_name(op);
    return rm_enum_new(Class_CompositeOperator, ID2SYM(rb_intern(name)), INT2FIX(op));
}


/**
 * Return the name of a CompressionType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param ct the CompressionType
 * @return the name
 */
static const char *
CompressionType_name(CompressionType ct)
{
    switch (ct)
    {
        ENUM_TO_NAME(UndefinedCompression)
        ENUM_TO_NAME(NoCompression)
#if defined(HAVE_ENUM_B44COMPRESSION)
        ENUM_TO_NAME(B44Compression)
#endif
#if defined(HAVE_ENUM_B44ACOMPRESSION)
        ENUM_TO_NAME(B44ACompression)
#endif
        ENUM_TO_NAME(BZipCompression)
#if defined(HAVE_ENUM_DXT1COMPRESSION)
        ENUM_TO_NAME(DXT1Compression)
#endif
#if defined(HAVE_ENUM_DXT3COMPRESSION)
        ENUM_TO_NAME(DXT3Compression)
#endif
#if defined(HAVE_ENUM_DXT5COMPRESSION)
        ENUM_TO_NAME(DXT5Compression)
#endif
        ENUM_TO_NAME(FaxCompression)
        ENUM_TO_NAME(Group4Compression)
        ENUM_TO_NAME(JPEGCompression)
        ENUM_TO_NAME(JPEG2000Compression)
        ENUM_TO_NAME(LosslessJPEGCompression)
        ENUM_TO_NAME(LZWCompression)
#if defined(HAVE_ENUM_PIZCOMPRESSION)
        ENUM_TO_NAME(PizCompression)
#endif
#if defined(HAVE_ENUM_PXR24COMPRESSION)
        ENUM_TO_NAME(Pxr24Compression)
#endif
        ENUM_TO_NAME(RLECompression)
        ENUM_TO_NAME(ZipCompression)
#if defined(HAVE_ENUM_ZIPSCOMPRESSION)
        ENUM_TO_NAME(ZipSCompression)
#endif
        }

    return "UndefinedCompression";
}


/**
 * Construct a CompressionType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param ct the CompressionType
 * @return a new CompressionType enumerator
 */
VALUE
CompressionType_new(CompressionType ct)
{
    const char *name = CompressionType_name(ct);
    return rm_enum_new(Class_CompressionType, ID2SYM(rb_intern(name)), INT2FIX(ct));
}


/**
 * Return the name of a DisposeType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the DisposeType
 * @return the name
 */
static const char *
DisposeType_name(DisposeType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedDispose)
        ENUM_TO_NAME(BackgroundDispose)
        ENUM_TO_NAME(NoneDispose)
        ENUM_TO_NAME(PreviousDispose)
    }

    return "UndefinedDispose";
}


/**
 * Construct a DisposeType enum object for the specified value..new.
 *
 * No Ruby usage (internal function)
 *
 * @param type the DisposeType
 * @return a new DisposeType enumerator
 */
VALUE
DisposeType_new(DisposeType type)
{
    const char *name = DisposeType_name(type);
    return rm_enum_new(Class_DisposeType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the name of a FilterTypes enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the FilterTypes
 * @return the name
 */
static const char *
FilterTypes_name(FilterTypes type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedFilter)
        ENUM_TO_NAME(PointFilter)
        ENUM_TO_NAME(BoxFilter)
        ENUM_TO_NAME(TriangleFilter)
        ENUM_TO_NAME(HermiteFilter)
        ENUM_TO_NAME(HanningFilter)
        ENUM_TO_NAME(HammingFilter)
        ENUM_TO_NAME(BlackmanFilter)
        ENUM_TO_NAME(GaussianFilter)
        ENUM_TO_NAME(QuadraticFilter)
        ENUM_TO_NAME(CubicFilter)
        ENUM_TO_NAME(CatromFilter)
        ENUM_TO_NAME(MitchellFilter)
        ENUM_TO_NAME(LanczosFilter)
        ENUM_TO_NAME(BesselFilter)
        ENUM_TO_NAME(SincFilter)
#if defined(HAVE_ENUM_KAISERFILTER)
        ENUM_TO_NAME(KaiserFilter)
#endif
#if defined(HAVE_ENUM_WELSHFILTER)
        ENUM_TO_NAME(WelshFilter)
#endif
#if defined(HAVE_ENUM_PARZENFILTER)
        ENUM_TO_NAME(ParzenFilter)
#endif
#if defined(HAVE_ENUM_LAGRANGEFILTER)
        ENUM_TO_NAME(LagrangeFilter)
#endif
#if defined(HAVE_ENUM_BOHMANFILTER)
        ENUM_TO_NAME(BohmanFilter)
#endif
#if defined(HAVE_ENUM_BARTLETTFILTER)
        ENUM_TO_NAME(BartlettFilter)
#endif
#if defined(HAVE_ENUM_SENTINELFILTER)
        // not a real filter name - defeat gcc warning message
        case SentinelFilter:
            break;
#endif
    }

    return "UndefinedFilter";
}


/**
 * Construct an FilterTypes enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param type the FilterTypes
 * @return a new FilterTypes enumerator
 */
VALUE
FilterTypes_new(FilterTypes type)
{
    const char *name = FilterTypes_name(type);
    return rm_enum_new(Class_FilterTypes, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the name of a EndianType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the EndianType
 * @return the name
 */
static const char *
EndianType_name(EndianType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedEndian)
        ENUM_TO_NAME(LSBEndian)
        ENUM_TO_NAME(MSBEndian)
    }
    return "UndefinedEndian";
}


/**
 * Construct an EndianType enum object.
 *
 * No Ruby usage (internal function)
 *
 * @param type the EndianType
 * @return a new EndianType enumerator
 */
VALUE
EndianType_new(EndianType type)
{
    const char *name = EndianType_name(type);
    return rm_enum_new(Class_EndianType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the name of a GravityType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the GravityType
 * @return the name
 */
static const char *
GravityType_name(GravityType type)
{
    switch(type)
    {
        ENUM_TO_NAME(ForgetGravity)
        ENUM_TO_NAME(NorthWestGravity)
        ENUM_TO_NAME(NorthGravity)
        ENUM_TO_NAME(NorthEastGravity)
        ENUM_TO_NAME(WestGravity)
        ENUM_TO_NAME(CenterGravity)
        ENUM_TO_NAME(EastGravity)
        ENUM_TO_NAME(SouthWestGravity)
        ENUM_TO_NAME(SouthGravity)
        ENUM_TO_NAME(SouthEastGravity)
        ENUM_TO_NAME(StaticGravity)
    }

    // Defeat "duplicate case value" error.
    return "UndefinedGravity";
}


/**
 * Construct an GravityType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param type the GravityType
 * @return a new GravityType enumerator
 */
VALUE
GravityType_new(GravityType type)
{
    const char *name = GravityType_name(type);
    return rm_enum_new(Class_GravityType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the name of a ImageType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the ImageType
 * @return the name
 */
static const char *
ImageType_name(ImageType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedType)
        ENUM_TO_NAME(BilevelType)
        ENUM_TO_NAME(GrayscaleType)
        ENUM_TO_NAME(GrayscaleMatteType)
        ENUM_TO_NAME(PaletteType)
        ENUM_TO_NAME(PaletteMatteType)
        ENUM_TO_NAME(TrueColorType)
        ENUM_TO_NAME(TrueColorMatteType)
        ENUM_TO_NAME(ColorSeparationType)
        ENUM_TO_NAME(ColorSeparationMatteType)
        ENUM_TO_NAME(OptimizeType)
        ENUM_TO_NAME(PaletteBilevelMatteType)
    }

    return "UndefinedType";
}


/**
 * Construct an ImageType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param type the ImageType
 * @return a new ImageType enumerator
 */
VALUE
ImageType_new(ImageType type)
{
    const char *name = ImageType_name(type);
    return rm_enum_new(Class_ImageType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the name of a InterlaceType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param interlace the InterlaceType
 * @return the name
 */
static const char *
InterlaceType_name(InterlaceType interlace)
{
    switch(interlace)
    {
        ENUM_TO_NAME(UndefinedInterlace)
        ENUM_TO_NAME(GIFInterlace)
        ENUM_TO_NAME(JPEGInterlace)
        ENUM_TO_NAME(PNGInterlace)
        ENUM_TO_NAME(NoInterlace)
        ENUM_TO_NAME(LineInterlace)
        ENUM_TO_NAME(PlaneInterlace)
        ENUM_TO_NAME(PartitionInterlace)
    }

    return "UndefinedInterlace";
}


/**
 * Construct an InterlaceType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param interlace the InterlaceType
 * @return a new InterlaceType enumerator
 */
VALUE
InterlaceType_new(InterlaceType interlace)
{
    const char *name = InterlaceType_name(interlace);
    return rm_enum_new(Class_InterlaceType, ID2SYM(rb_intern(name)), INT2FIX(interlace));
}


/**
 * Return the name of a InterpolatePixelMethod enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param interpolate the InterpolatePixelMethod
 * @return the name
 */
static const char *
InterpolatePixelMethod_name(InterpolatePixelMethod interpolate)
{
    switch(interpolate)
    {
        ENUM_TO_NAME(UndefinedInterpolatePixel)
        ENUM_TO_NAME(AverageInterpolatePixel)
        ENUM_TO_NAME(BicubicInterpolatePixel)
        ENUM_TO_NAME(BilinearInterpolatePixel)
        ENUM_TO_NAME(FilterInterpolatePixel)
        ENUM_TO_NAME(IntegerInterpolatePixel)
        ENUM_TO_NAME(MeshInterpolatePixel)
        ENUM_TO_NAME(NearestNeighborInterpolatePixel)
        ENUM_TO_NAME(SplineInterpolatePixel)
    }

    return "UndefinedInterpolatePixel";
}


/**
 * Construct an InterpolatePixelMethod enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param interpolate the InterpolatePixelMethod
 * @return a new InterpolatePixelMethod enumerator
 */
VALUE
InterpolatePixelMethod_new(InterpolatePixelMethod interpolate)
{
    const char *name = InterpolatePixelMethod_name(interpolate);
    return rm_enum_new(Class_InterpolatePixelMethod, ID2SYM(rb_intern(name)), INT2FIX(interpolate));
}


/**
 * Return the name of a MagickLayerMethod enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param method the MagickLayerMethod
 * @return the name
 */
static const char *
LAYERMETHODTYPE_NAME(LAYERMETHODTYPE method)
{
    switch(method)
    {
        ENUM_TO_NAME(UndefinedLayer)
        ENUM_TO_NAME(CompareAnyLayer)
        ENUM_TO_NAME(CompareClearLayer)
        ENUM_TO_NAME(CompareOverlayLayer)
        ENUM_TO_NAME(OptimizeLayer)
        ENUM_TO_NAME(OptimizePlusLayer)
        ENUM_TO_NAME(CoalesceLayer)
        ENUM_TO_NAME(DisposeLayer)
        ENUM_TO_NAME(OptimizeTransLayer)
        ENUM_TO_NAME(OptimizeImageLayer)
        ENUM_TO_NAME(RemoveDupsLayer)
        ENUM_TO_NAME(RemoveZeroLayer)
        ENUM_TO_NAME(CompositeLayer)
#if defined(HAVE_ENUM_MERGELAYER)
        ENUM_TO_NAME(MergeLayer)
#endif
#if defined(HAVE_ENUM_MOSAICLAYER)
        ENUM_TO_NAME(MosaicLayer)
#endif
#if defined(HAVE_ENUM_FLATTENLAYER)
        ENUM_TO_NAME(FlattenLayer)
#endif
#if defined(HAVE_ENUM_TRIMBOUNDSLAYER)
        ENUM_TO_NAME(TrimBoundsLayer)
#endif
    }

    return "UndefinedLayer";
}


/**
 * Construct an MagickLayerMethod enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param method the MagickLayerMethod
 * @return a new MagickLayerMethod enumerator
 */
VALUE
LAYERMETHODTYPE_NEW(LAYERMETHODTYPE method)
{
    const char *name = LAYERMETHODTYPE_NAME(method);
    return rm_enum_new(CLASS_LAYERMETHODTYPE, ID2SYM(rb_intern(name)), INT2FIX(method));
}


/**
 * Return the name of a OrientationType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the OreintationType
 * @return the name
 */
static const char *
OrientationType_name(OrientationType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedOrientation)
        ENUM_TO_NAME(TopLeftOrientation)
        ENUM_TO_NAME(TopRightOrientation)
        ENUM_TO_NAME(BottomRightOrientation)
        ENUM_TO_NAME(BottomLeftOrientation)
        ENUM_TO_NAME(LeftTopOrientation)
        ENUM_TO_NAME(RightTopOrientation)
        ENUM_TO_NAME(RightBottomOrientation)
        ENUM_TO_NAME(LeftBottomOrientation)
    }

    return "UndefinedOrientation";
}


/**
 * Construct an OrientationType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param type the OrientationType
 * @return a new OrientationType enumerator
 */
VALUE
OrientationType_new(OrientationType type)
{
    const char *name = OrientationType_name(type);
    return rm_enum_new(Class_OrientationType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the name of a RenderingIntent enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param intent the RenderingIntent
 * @return the name
 */
static const char *
RenderingIntent_name(RenderingIntent intent)
{
    switch(intent)
    {
        ENUM_TO_NAME(UndefinedIntent)
        ENUM_TO_NAME(SaturationIntent)
        ENUM_TO_NAME(PerceptualIntent)
        ENUM_TO_NAME(AbsoluteIntent)
        ENUM_TO_NAME(RelativeIntent)
    }

    return "UndefinedIntent";
}


/**
 * Construct an RenderingIntent enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param intent the RenderingIntent
 * @return a new RenderingIntent enumerator
 */
VALUE
RenderingIntent_new(RenderingIntent intent)
{
    const char *name = RenderingIntent_name(intent);
    return rm_enum_new(Class_RenderingIntent, ID2SYM(rb_intern(name)), INT2FIX(intent));
}


/**
 * Return the name of a ResolutionType enum as a string.
 *
 * No Ruby usage (internal function)
 *
 * @param type the ResolutionType
 * @return the name
 */
static const char *
ResolutionType_name(ResolutionType type)
{
    switch(type)
    {
        ENUM_TO_NAME(UndefinedResolution)
        ENUM_TO_NAME(PixelsPerInchResolution)
        ENUM_TO_NAME(PixelsPerCentimeterResolution)
    }

    return "UndefinedResolution";
}


/**
 * Construct an ResolutionType enum object for the specified value.
 *
 * No Ruby usage (internal function)
 *
 * @param type the ResolutionType
 * @return a new ResolutionType enumerator
 */
VALUE
ResolutionType_new(ResolutionType type)
{
    const char *name = ResolutionType_name(type);
    return rm_enum_new(Class_ResolutionType, ID2SYM(rb_intern(name)), INT2FIX(type));
}


/**
 * Return the string representation of a StorageType value.
 *
 * No Ruby usage (internal function)
 *
 * @param type the StorageType
 * @return the name
 */
const char *
StorageType_name(StorageType type)
{
    switch (type)
    {
        ENUM_TO_NAME(UndefinedPixel)
        ENUM_TO_NAME(CharPixel)
        ENUM_TO_NAME(DoublePixel)
        ENUM_TO_NAME(FloatPixel)
        ENUM_TO_NAME(IntegerPixel)
        ENUM_TO_NAME(LongPixel)
        ENUM_TO_NAME(QuantumPixel)
        ENUM_TO_NAME(ShortPixel)
    }

    return "UndefinedPixel";
}


/**
 * Return the string representation of a VirtualPixelMethod value.
 *
 * No Ruby usage (internal function)
 *
 * @param method the VirtualPixelMethod
 * @return the name
 */
static const char *
VirtualPixelMethod_name(VirtualPixelMethod method)
{
    switch (method)
    {
        ENUM_TO_NAME(UndefinedVirtualPixelMethod)
        ENUM_TO_NAME(EdgeVirtualPixelMethod)
        ENUM_TO_NAME(MirrorVirtualPixelMethod)
        ENUM_TO_NAME(TileVirtualPixelMethod)
        ENUM_TO_NAME(TransparentVirtualPixelMethod)
        ENUM_TO_NAME(BackgroundVirtualPixelMethod)
        ENUM_TO_NAME(DitherVirtualPixelMethod)
        ENUM_TO_NAME(RandomVirtualPixelMethod)
        ENUM_TO_NAME(ConstantVirtualPixelMethod)
        ENUM_TO_NAME(MaskVirtualPixelMethod)
        ENUM_TO_NAME(BlackVirtualPixelMethod)
        ENUM_TO_NAME(GrayVirtualPixelMethod)
        ENUM_TO_NAME(WhiteVirtualPixelMethod)
#if defined(HAVE_ENUM_HORIZONTALTILEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(HorizontalTileVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_VERTICALTILEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(VerticalTileVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_HORIZONTALTILEEDGEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(HorizontalTileEdgeVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_VERTICALTILEEDGEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(VerticalTileEdgeVirtualPixelMethod)
#endif
#if defined(HAVE_ENUM_CHECKERTILEVIRTUALPIXELMETHOD)
        ENUM_TO_NAME(CheckerTileVirtualPixelMethod)
#endif
    }

    return "UndefinedVirtualPixelMethod";
}


/**
 * Construct a VirtualPixelMethod enum for a specified VirtualPixelMethod value.
 *
 * No Ruby usage (internal function)
 *
 * @param style theVirtualPixelMethod
 * @return a new VirtualPixelMethod enumerator
 */
VALUE
VirtualPixelMethod_new(VirtualPixelMethod style)
{
    const char *name = VirtualPixelMethod_name(style);
    return rm_enum_new(Class_VirtualPixelMethod, ID2SYM(rb_intern(name)), INT2FIX(style));
}
