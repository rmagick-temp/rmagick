/* $Id: rmagick.h,v 1.55 2004/06/15 00:01:50 rmagick Exp $ */
/*=============================================================================
|               Copyright (C) 2004 by Timothy P. Hunter
| Name:     rmagick.h
| Purpose:  RMagick declarations and definitions
| Author:   Tim Hunter
\============================================================================*/

#ifndef _RMAGICK_H_
#define _RMAGICK_H_

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include "ruby.h"
#include "intern.h"
#include "rubyio.h"
#include "magick/api.h"



// Undef ImageMagick's versions of these symbols
#undef PACKAGE_VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_TARNAME
#include "rmagick_config.h"

// Define a pair of macros that make it easier to code
// 1.6 and 1.8 alternatives. Code enclosed in RUBY18()
// is present when compiled for 1.8.0 and later. Code
// enclosed in RUBY16 is present for 1.6 versions.
#if RUBY_VERSION >= 0x180
#define RUBY18(d) d
#define RUBY16(d)
#else
#define RUBY18(d)
#define RUBY16(d) d
#endif

#if !defined(ULONG2NUM)
#define ULONG2NUM(v) UINT2NUM(v)
#endif


// Define a version of StringValuePtr that works in both 1.6 and 1.8.
#if !defined(StringValuePtr)
#define STRING_PTR(v) rm_string_value_ptr(&(v))
#else
#define STRING_PTR(v) StringValuePtr(v)
#endif

// Safe replacement for rb_str2cstr
#define STRING_PTR_LEN(v,l) rm_string_value_ptr_len(&(v), &(l))

#undef DegreesToRadians     // defined in ImageMagick.h in 6.0.2
#define DegreesToRadians(x) ((x)*3.14159265358979323846/180.0)

typedef ImageInfo Info; // Make type name match class name
typedef PixelPacket Pixel;

// Montage
typedef struct
{
    CompositeOperator compose;
    MontageInfo *info;
} Montage;

// Draw
typedef struct
{
    DrawInfo *info;         // the DrawInfo struct
    VALUE primitives;       // the primitive string
    VALUE tmpfile_ary;
} Draw;             // make the type match the class name

// Enum
typedef struct
{
   ID id;
   int val;
} MagickEnum;

#undef False    // defined in deprecate.h in 6.0.2
#undef True     // defined in deprecate.h in 6.0.2
typedef enum
{
    False = 0,
    True = 1
}
boolean;

typedef enum {
    AnyWeight,
    NormalWeight,
    BoldWeight,
    BolderWeight,
    LighterWeight
} WeightType;

// Draw#text_anchor AnchorType argument
typedef enum {
    StartAnchor = 1,
    MiddleAnchor = 2,
    EndAnchor = 3
} AnchorType;


typedef struct
{
    unsigned char id;   // Dumped image id = 0xd1
    unsigned char mj;   // Major format number = 1
    unsigned char mi;   // Minor format number = 0
    unsigned char len;  // Length of image magick string
    char magick[MaxTextExtent]; // magick string
} DumpedImage;

#define DUMPED_IMAGE_ID      0xd1
#define DUMPED_IMAGE_MAJOR_VERS 1
#define DUMPED_IMAGE_MINOR_VERS 0

#define MAGICK_LOC "magick_location"     // instance variable name in ImageMagickError class

#define MAX_GEOM_STR 51                 // max length of a geometry string

#if defined(HAVE_QUANTUMOPERATORREGIONIMAGE) || defined(HAVE_EVALUATEIMAGECHANNEL)
/*
 * Both ImageMagick and GraphicsMagick define an enum type for quantum-level
 * expressions, but they're different types. The QuantumExpressionOperator
 * type is an adapter type that can be mapped to either one.
*/
typedef enum _QuantumExpressionOperator
{
    UndefinedQuantumOperator,
    AddQuantumOperator,
    AndQuantumOperator,
    DivideQuantumOperator,
    LShiftQuantumOperator,
    MultiplyQuantumOperator,
    OrQuantumOperator,
    RShiftQuantumOperator,
    SubtractQuantumOperator,
    XorQuantumOperator
} QuantumExpressionOperator ;
#endif


/*
    ImageMagick used simply size_t and off_t in 5.5.1, then defined the
    Extended(Un)SignedIntegralType from 5.5.2 thru 5.5.7. The 5.5.8 release
    deprecates these types and uses Magick(Un)SignedType instead.
    GraphicsMagick 1.1. introduced the magick_(u)int64_t type.

    Here, if we don't already have magick_(u)int64_t, define them.
*/
#if !defined(HAVE_MAGICK_INT64_T)
#if defined(HAVE_MAGICKOFFSETTYPE)
typedef MagickOffsetType magick_int64_t;
#elif defined(HAVE_EXTENDEDSIGNEDINTEGRALTYPE)
typedef ExtendedSignedIntegralType magick_int64_t;
#else
typedef off_t magick_int64_t;
#endif
#endif

#if !defined(HAVE_MAGICK_UINT64_T)
#if defined(HAVE_MAGICKSIZETYPE)
typedef MagickSizeType magick_uint64_t;
#elif defined(HAVE_EXTENDEDUNSIGNEDINTEGRALTYPE)
typedef ExtendedUnsignedIntegralType magick_uint64_t;
#else
typedef size_t magick_uint64_t;
#endif
#endif

// This implements the "omitted storage class model" for external variables.
// (Ref: Harbison & Steele.) The rmmain.c file defines MAIN, which causes
// the single defining declarations to be generated. No other source files
// define MAIN and therefore generate referencing declarations.
#undef EXTERN
#if defined(MAIN)
#define EXTERN
#else
#define EXTERN extern
#endif

/*
*   RMagick Module and Class VALUEs
*/
EXTERN VALUE Module_Magick;
EXTERN VALUE Class_ImageList;
EXTERN VALUE Class_Info;
EXTERN VALUE Class_Draw;
EXTERN VALUE Class_Image;
EXTERN VALUE Class_Montage;
EXTERN VALUE Class_ImageMagickError;
EXTERN VALUE Class_GradientFill;
EXTERN VALUE Class_TextureFill;
EXTERN VALUE Class_AffineMatrix;
EXTERN VALUE Class_Chromaticity;
EXTERN VALUE Class_Color;
EXTERN VALUE Class_Font;
EXTERN VALUE Class_Geometry;
EXTERN VALUE Class_GeometryValue;   // Defined in RMagick.rb
EXTERN VALUE Class_Pixel;
EXTERN VALUE Class_Point;
EXTERN VALUE Class_Primary;
EXTERN VALUE Class_Rectangle;
EXTERN VALUE Class_Segment;
EXTERN VALUE Class_TypeMetric;
#if defined(HAVE_COMPAREIMAGECHANNELS)
EXTERN VALUE Class_MetricType;
#endif
#if defined(HAVE_QUANTUMOPERATORREGIONIMAGE) || defined(HAVE_EVALUATEIMAGECHANNEL)
EXTERN VALUE Class_QuantumExpressionOperator;
#endif
#if defined(HAVE_GETIMAGESTATISTICS)
EXTERN VALUE Class_Statistics;
EXTERN VALUE Class_StatisticsChannel;
#endif

// Enum classes
EXTERN VALUE Class_Enum;
EXTERN VALUE Class_AlignType;
EXTERN VALUE Class_AnchorType;
EXTERN VALUE Class_ChannelType;
EXTERN VALUE Class_ClassType;
EXTERN VALUE Class_ColorspaceType;
EXTERN VALUE Class_ComplianceType;
EXTERN VALUE Class_CompositeOperator;
EXTERN VALUE Class_CompressionType;
EXTERN VALUE Class_DecorationType;
EXTERN VALUE Class_DisposeType;
EXTERN VALUE Class_EndianType;
EXTERN VALUE Class_FilterTypes;
EXTERN VALUE Class_GravityType;
EXTERN VALUE Class_ImageType;
EXTERN VALUE Class_InterlaceType;
EXTERN VALUE Class_NoiseType;
EXTERN VALUE Class_PaintMethod;
EXTERN VALUE Class_PreviewType;
EXTERN VALUE Class_RenderingIntent;
EXTERN VALUE Class_ResolutionType;
EXTERN VALUE Class_StretchType;
EXTERN VALUE Class_StyleType;
EXTERN VALUE Class_WeightType;

/*
*   Commonly-used IDs
*/
EXTERN ID ID__dummy_img_;       // "_dummy_img_"
EXTERN ID ID_changed;           // "changed"
EXTERN ID ID_call;              // "call"
EXTERN ID ID_cur_image;         // "cur_image"
EXTERN ID ID_dup;               // "dup"
EXTERN ID ID_flag;              // "flag"
EXTERN ID ID_from_s;            // "from_s"
EXTERN ID ID_Geometry;          // "Geometry"
EXTERN ID ID_GeometryValue;     // "GeometryValue"
EXTERN ID ID_height;            // "height"
EXTERN ID ID_initialize_copy;   // "initialize_copy"
EXTERN ID ID_length;            // "length"
EXTERN ID ID_notify_observers;  // "notify_observers"
EXTERN ID ID_new;               // "new"
EXTERN ID ID_push;              // "push"
EXTERN ID ID_to_s;              // "to_s"
EXTERN ID ID_values;            // "values"
EXTERN ID ID_width;             // "width"
EXTERN ID ID_x;                 // "x"
EXTERN ID ID_y;                 // "y"


#if defined(HAVE_GETNEXTIMAGEINLIST)
#define GET_NEXT_IMAGE(a) GetNextImageInList(a)
#else
#define GET_NEXT_IMAGE(a) (a)->next
#endif

#if defined(HAVE_GETLOCALEEXCEPTIONMESSAGE)
#define GET_MSG(s,t) GetLocaleExceptionMessage((s), (t))
#else
#define GET_MSG(s,t) t
#endif


#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define Q(N) Q2(N)
#define Q2(N) #N

/*
   Handle warnings & errors

   For ExceptionInfo structures in auto storage. Destroys the
   ExceptionInfo structure (releasing any IM storage) before
   calling rb_raise.
*/
#define HANDLE_ERROR handle_error(&exception);
// For ExceptionInfo structures in images.
#define HANDLE_IMG_ERROR(img) handle_error(&((img)->exception));

/*
    Map the QuantumDepth to a StorageType.
*/
#if QuantumDepth == 8
#define FIX_STG_TYPE CharPixel
#elif QuantumDepth == 16
#define FIX_STG_TYPE ShortPixel
#else   // QuantumDepth == 32
#define FIX_STG_TYPE LongPixel
#endif


/*
    Call rb_define_method for an attribute accessor method
*/
#define DCL_ATTR_READER(class, attr) \
    rb_define_method(Class_##class, #attr, class##_##attr, 0);
#define DCL_ATTR_WRITER(class, attr) \
    rb_define_method(Class_##class, #attr "=", class##_##attr##_eq, 1);
#define DCL_ATTR_ACCESSOR(class, attr) \
    DCL_ATTR_READER(class, attr) \
    DCL_ATTR_WRITER(class, attr)

/*
    Define simple attribute accessor methods (boolean, int, string, and double types)
*/
#define C_bool_to_R_bool(attr) (attr) ? Qtrue : Qfalse
#define R_bool_to_C_bool(attr) RTEST(attr)
#define C_int_to_R_int(attr) INT2FIX(attr)
#define R_int_to_C_int(attr) NUM2INT(attr)
#define C_long_to_R_long(attr) INT2NUM(attr)
#define R_long_to_C_long(attr) NUM2LONG(attr)
#define C_ulong_to_R_ulong(attr) UINT2NUM(attr)
#define R_ulong_to_C_ulong(attr) NUM2ULONG(attr)
#define C_str_to_R_str(attr) attr ? rb_str_new2(attr) : Qnil
#define C_dbl_to_R_dbl(attr) rb_float_new(attr)
#define R_dbl_to_C_dbl(attr) NUM2DBL(attr)

#define DEF_ATTR_READER(class, attr, type) \
    VALUE class##_##attr(VALUE self)\
    {\
        class *ptr;\
        Data_Get_Struct(self, class, ptr);\
        return C_##type##_to_R_##type(ptr->attr);\
    }

// Use when attribute name is different from the field name
#define DEF_ATTR_READERF(class, attr, field, type) \
    VALUE class##_##attr(VALUE self)\
    {\
        class *ptr;\
        Data_Get_Struct(self, class, ptr);\
        return C_##type##_to_R_##type(ptr->field);\
    }
#define DEF_ATTR_WRITER(class, attr, type) \
    VALUE class##_##attr##_eq(VALUE self, VALUE val)\
    {\
        class *ptr;\
        rm_check_frozen(self);\
        Data_Get_Struct(self, class, ptr);\
        ptr->attr = R_##type##_to_C_##type(val);\
        return self;\
    }
#define DEF_ATTR_ACCESSOR(class, attr, type)\
    DEF_ATTR_READER(class, attr, type)\
    DEF_ATTR_WRITER(class, attr, type)

/*
 *  Declare attribute accessors
 */
#define ATTR_READER(class, attr) \
    extern VALUE class##_##attr(VALUE);
#define ATTR_WRITER(class, attr) \
    extern VALUE class##_##attr##_eq(VALUE, VALUE);
#define ATTR_ACCESSOR(class, attr) \
    ATTR_READER(class, attr)\
    ATTR_WRITER(class, attr)

/*
 *  Enum constants - define a subclass of Enum for the specified enumeration.
 *  Define an instance of the subclass for each member in the enumeration.
 *  Initialize each instance with its name and value.
 */
#define DEF_ENUM(type) {\
   VALUE _cls, _enum;\
   _cls =  Class_##type = rb_define_class_under(Module_Magick, #type, Class_Enum);
#define ENUM_VAL(val)\
   _enum = rm_enum_new(_cls, ID2SYM(rb_intern(#val)), INT2FIX(val));\
   rb_define_const(Module_Magick, #val, _enum);
#define END_ENUM }

//  Define a Magick module constant
#define DEF_CONST(constant) rb_define_const(Module_Magick, #constant, INT2FIX(constant))


// Convert a Ruby enum constant back to a C enum member.
#define VALUE_TO_ENUM(value, e, type) \
   do {\
   MagickEnum *magick_enum;\
   if (CLASS_OF(value) != Class_##type)\
       rb_raise(rb_eTypeError, "wrong enumeration type - expected %s, got %s"\
                   , rb_class2name(Class_##type),rb_class2name(CLASS_OF(value)));\
   Data_Get_Struct(value, MagickEnum, magick_enum);\
   e = (type)(magick_enum->val);\
   } while(0)



// Method, external function declarations. These declarations are
// grouped by the source file in which the methods are defined.

// We don't need any "extern/no extern" stuff here. An external function
// declaration can refer to a function defined in another source file or
// the same source file.

// rmdraw.c
ATTR_WRITER(Draw, affine)
ATTR_WRITER(Draw, align)
ATTR_WRITER(Draw, decorate)
ATTR_WRITER(Draw, density)
ATTR_WRITER(Draw, encoding)
ATTR_WRITER(Draw, fill)
ATTR_WRITER(Draw, font)
ATTR_WRITER(Draw, font_family)
ATTR_WRITER(Draw, font_stretch)
ATTR_WRITER(Draw, font_style)
ATTR_WRITER(Draw, font_weight)
ATTR_WRITER(Draw, gravity)
ATTR_WRITER(Draw, pointsize)
ATTR_WRITER(Draw, rotation)
ATTR_WRITER(Draw, stroke)
ATTR_WRITER(Draw, text_antialias)
ATTR_WRITER(Draw, undercolor)
extern VALUE Draw_annotate(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Draw_clone(VALUE);
extern VALUE Draw_composite(int, VALUE *, VALUE);
extern VALUE Draw_draw(VALUE, VALUE);
extern VALUE Draw_dup(VALUE);
extern VALUE Draw_get_type_metrics(int, VALUE *, VALUE);
extern VALUE Draw_init_copy(VALUE, VALUE);
extern VALUE Draw_initialize(VALUE);
extern VALUE Draw_inspect(VALUE);
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
extern VALUE Draw_alloc(VALUE);
#else
extern VALUE Draw_new(VALUE);
#endif
extern VALUE Draw_primitive(VALUE, VALUE);

ATTR_WRITER(Montage, background_color)
ATTR_WRITER(Montage, border_color)
ATTR_WRITER(Montage, border_width)
ATTR_WRITER(Montage, compose)
ATTR_WRITER(Montage, filename)
ATTR_WRITER(Montage, fill)
ATTR_WRITER(Montage, font)
ATTR_WRITER(Montage, frame)
ATTR_WRITER(Montage, geometry)
ATTR_WRITER(Montage, gravity)
ATTR_WRITER(Montage, matte_color)
ATTR_WRITER(Montage, pointsize)
ATTR_WRITER(Montage, shadow)
ATTR_WRITER(Montage, stroke)
ATTR_WRITER(Montage, texture)
ATTR_WRITER(Montage, tile)
ATTR_WRITER(Montage, title)
extern VALUE Montage_initialize(VALUE);
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
extern VALUE Montage_alloc(VALUE);
#else
extern VALUE Montage_new(VALUE);
#endif
extern VALUE rm_montage_new(void);


// rmmain.c
extern VALUE rm_montage_new(void);


 // rmilist.c
extern VALUE ImageList_animate(int, VALUE *, VALUE);
extern VALUE ImageList_append(VALUE, VALUE);
extern VALUE ImageList_average(VALUE);
extern VALUE ImageList_coalesce(VALUE);
extern VALUE ImageList_deconstruct(VALUE);
extern VALUE ImageList_display(VALUE);
extern VALUE ImageList_flatten_images(VALUE);
extern VALUE ImageList_map(VALUE, VALUE, VALUE);
extern VALUE ImageList_montage(VALUE);
extern VALUE ImageList_morph(VALUE, VALUE);
extern VALUE ImageList_mosaic(VALUE);
extern VALUE ImageList_quantize(int, VALUE*, VALUE);
extern VALUE ImageList_to_blob(VALUE);
extern VALUE ImageList_write(VALUE, VALUE);

extern VALUE rm_imagelist_new(void);
extern int rm_imagelist_length(VALUE);
extern VALUE rm_imagelist_push(VALUE, VALUE);


// rminfo.c
ATTR_ACCESSOR(Info, antialias)
ATTR_ACCESSOR(Info, background_color)
ATTR_ACCESSOR(Info, border_color)
ATTR_ACCESSOR(Info, colorspace)
ATTR_ACCESSOR(Info, compression)
ATTR_ACCESSOR(Info, density)
ATTR_ACCESSOR(Info, depth)
ATTR_ACCESSOR(Info, dither)
ATTR_ACCESSOR(Info, extract)
ATTR_ACCESSOR(Info, filename)
ATTR_ACCESSOR(Info, font)
ATTR_ACCESSOR(Info, format)
ATTR_ACCESSOR(Info, fuzz)
ATTR_ACCESSOR(Info, group)
ATTR_ACCESSOR(Info, image_type)
ATTR_ACCESSOR(Info, interlace)
ATTR_ACCESSOR(Info, matte_color)
ATTR_ACCESSOR(Info, monochrome)
ATTR_ACCESSOR(Info, number_scenes)
ATTR_ACCESSOR(Info, page)
ATTR_ACCESSOR(Info, pen)
// ATTR_ACCESSOR(Info, ping) obsolete
// ATTR_ACCESSOR(Info, pointsize)
ATTR_ACCESSOR(Info, quality)
ATTR_ACCESSOR(Info, scene)
ATTR_ACCESSOR(Info, server_name)
ATTR_ACCESSOR(Info, subimage)
ATTR_ACCESSOR(Info, subrange)
ATTR_ACCESSOR(Info, tile)
ATTR_ACCESSOR(Info, size)
ATTR_ACCESSOR(Info, units)
ATTR_ACCESSOR(Info, view)
//ATTR_ACCESSOR(Info, verbose)

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
extern VALUE Info_alloc(VALUE);
#else
extern VALUE Info_new(VALUE);
#endif

extern VALUE Info_define(int, VALUE *, VALUE);
extern VALUE Info_initialize(VALUE);
extern VALUE rm_info_new(void);


// rmimage.c
ATTR_ACCESSOR(Image, background_color)
ATTR_READER(Image, base_columns)
ATTR_READER(Image, base_filename)
ATTR_READER(Image, base_rows)
ATTR_ACCESSOR(Image, blur)
ATTR_ACCESSOR(Image, border_color)
ATTR_READER(Image, bounding_box)
ATTR_ACCESSOR(Image, chromaticity)
ATTR_WRITER(Image, clip_mask)
ATTR_ACCESSOR(Image, color_profile)
ATTR_READER(Image, colors)
ATTR_ACCESSOR(Image, colorspace)
ATTR_READER(Image, columns)
ATTR_ACCESSOR(Image, compose)
ATTR_ACCESSOR(Image, compression)
ATTR_ACCESSOR(Image, delay)
ATTR_ACCESSOR(Image, density)
ATTR_READER(Image, depth)
ATTR_READER(Image, directory)
ATTR_ACCESSOR(Image, dispose)
ATTR_ACCESSOR(Image, endian)
ATTR_ACCESSOR(Image, extract_info)
ATTR_READER(Image, filename)
ATTR_READER(Image, filesize)
ATTR_ACCESSOR(Image, filter)
ATTR_ACCESSOR(Image, format)
ATTR_ACCESSOR(Image, fuzz)
ATTR_ACCESSOR(Image, gamma)
ATTR_ACCESSOR(Image, geometry)
ATTR_ACCESSOR(Image, image_type)
ATTR_ACCESSOR(Image, interlace)
ATTR_ACCESSOR(Image, iptc_profile)
ATTR_ACCESSOR(Image, iterations)
ATTR_ACCESSOR(Image, matte)
ATTR_ACCESSOR(Image, matte_color)
ATTR_READER(Image, mean_error_per_pixel)
ATTR_READER(Image, mime_type)
ATTR_ACCESSOR(Image, montage)
ATTR_READER(Image, normalized_mean_error)
ATTR_READER(Image, normalized_maximum_error)
ATTR_READER(Image, number_colors)
ATTR_ACCESSOR(Image, offset)
ATTR_WRITER(Image, opacity)
ATTR_ACCESSOR(Image, page)
ATTR_READER(Image, quantum_depth)
ATTR_ACCESSOR(Image, rendering_intent)
ATTR_READER(Image, rows)
ATTR_READER(Image, scene)
ATTR_ACCESSOR(Image, start_loop)
ATTR_ACCESSOR(Image, class_type)
ATTR_ACCESSOR(Image, tile_info)
ATTR_READER(Image, total_colors)
ATTR_ACCESSOR(Image, units)
ATTR_ACCESSOR(Image, x_resolution)
ATTR_ACCESSOR(Image, y_resolution)

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
extern VALUE Image_alloc(VALUE);
extern VALUE Image_initialize(int, VALUE *, VALUE);
#else
extern VALUE Image_new(int, VALUE *, VALUE);
extern VALUE Image_initialize(VALUE, VALUE, VALUE, VALUE, VALUE);
#endif

extern VALUE Image_adaptive_threshold(int, VALUE *, VALUE);
extern VALUE Image_add_noise(VALUE, VALUE);
extern VALUE Image_affine_transform(VALUE, VALUE);
extern VALUE Image_aref(VALUE, VALUE);
extern VALUE Image_aset(VALUE, VALUE, VALUE);
extern VALUE Image_properties(VALUE);
extern VALUE Image_bilevel_channel(int, VALUE *, VALUE);
extern VALUE Image_black_threshold(int, VALUE *, VALUE);
extern VALUE Image_blur_image(int, VALUE *, VALUE);
extern VALUE Image_border(VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_border_bang(VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_capture(int, VALUE *, VALUE);
extern VALUE Image_change_geometry(VALUE, VALUE);
extern VALUE Image_changed_q(VALUE);
extern VALUE Image_channel(VALUE, VALUE);
extern VALUE Image_channel_compare(int, VALUE *, VALUE);
extern VALUE Image_channel_depth(int, VALUE *, VALUE);
extern VALUE Image_channel_extrema(int, VALUE *, VALUE);
extern VALUE Image_channel_mean(int, VALUE *, VALUE);
extern VALUE Image_channel_threshold(int, VALUE *, VALUE);
extern VALUE Image_charcoal(int, VALUE *, VALUE);
extern VALUE Image_chop(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_clone(VALUE);
extern VALUE Image_color_flood_fill(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_color_histogram(VALUE);
extern VALUE Image_colorize(int, VALUE *, VALUE);
extern VALUE Image_colormap(int, VALUE *, VALUE);
extern VALUE Image_composite(int, VALUE *, VALUE);
extern VALUE Image_composite_bang(int, VALUE *, VALUE);
extern VALUE Image_composite_affine(VALUE, VALUE, VALUE);
extern VALUE Image_compress_colormap_bang(VALUE);
extern VALUE Image_constitute(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_contrast(int, VALUE *, VALUE);
extern VALUE Image_convolve(VALUE, VALUE, VALUE);
extern VALUE Image_copy(VALUE);
extern VALUE Image_crop(int, VALUE *, VALUE);
extern VALUE Image_crop_bang(int, VALUE *, VALUE);
extern VALUE Image_cycle_colormap(VALUE, VALUE);
extern VALUE Image_despeckle(VALUE);
extern VALUE Image_difference(VALUE, VALUE);
extern VALUE Image_dispatch(int, VALUE *, VALUE);
extern VALUE Image_display(VALUE);
extern VALUE Image__dump(VALUE, VALUE);
extern VALUE Image_dup(VALUE);
extern VALUE Image_each_profile(VALUE);
extern VALUE Image_edge(int, VALUE *, VALUE);
extern VALUE Image_emboss(int, VALUE *, VALUE);
extern VALUE Image_enhance(VALUE);
extern VALUE Image_equalize(VALUE);
extern VALUE Image_erase_bang(VALUE);
extern VALUE Image_export_pixels(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_flip(VALUE);
extern VALUE Image_flip_bang(VALUE);
extern VALUE Image_flop(VALUE);
extern VALUE Image_flop_bang(VALUE);
extern VALUE Image_frame(int, VALUE *, VALUE);
extern VALUE Image_from_blob(VALUE, VALUE);
extern VALUE Image_gamma_channel(int, VALUE *, VALUE);
extern VALUE Image_gamma_correct(int, VALUE *, VALUE);
extern VALUE Image_gaussian_blur(int, VALUE *, VALUE);
extern VALUE Image_gaussian_blur_channel(int, VALUE *, VALUE);
extern VALUE Image_get_pixels(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_gray_q(VALUE);
extern VALUE Image_grayscale_pseudo_class(int, VALUE *, VALUE);
extern VALUE Image_implode(int, VALUE *, VALUE);
extern VALUE Image_import_pixels(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_init_copy(VALUE, VALUE);
extern VALUE Image_inspect(VALUE);
extern VALUE Image_level(int, VALUE *, VALUE);
extern VALUE Image_level_channel(int, VALUE *, VALUE);
extern VALUE Image__load(VALUE, VALUE);
extern VALUE Image_magnify(VALUE);
extern VALUE Image_magnify_bang(VALUE);
extern VALUE Image_map(int, VALUE *, VALUE);
extern VALUE Image_matte_flood_fill(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_median_filter(int, VALUE *, VALUE);
extern VALUE Image_minify(VALUE);
extern VALUE Image_minify_bang(VALUE);
extern VALUE Image_modulate(int, VALUE *, VALUE);
extern VALUE Image_monochrome_q(VALUE);
extern VALUE Image_motion_blur(VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_negate(int, VALUE *, VALUE);
extern VALUE Image_negate_channel(int, VALUE *, VALUE);
extern VALUE Image_normalize(VALUE);
extern VALUE Image_oil_paint(int, VALUE *, VALUE);
extern VALUE Image_opaque(VALUE, VALUE, VALUE);
extern VALUE Image_opaque_q(VALUE);
extern VALUE Image_ordered_dither(VALUE);
extern VALUE Image_palette_q(VALUE);
extern VALUE Image_ping(VALUE, VALUE);
extern VALUE Image_pixel_color(int, VALUE *, VALUE);
// VALUE Image_plasma(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_posterize(int, VALUE *, VALUE);
extern VALUE Image_preview(VALUE, VALUE);
extern VALUE Image_profile_bang(VALUE, VALUE, VALUE);
extern VALUE Image_quantize(int, VALUE *, VALUE);
extern VALUE Image_quantization_error(VALUE);
extern VALUE Image_quantum_operator(int, VALUE *, VALUE);
extern VALUE Image_radial_blur(VALUE, VALUE);
extern VALUE Image_raise(int, VALUE *, VALUE);
extern VALUE Image_random_channel_threshold(VALUE, VALUE, VALUE);
extern VALUE Image_random_threshold_channel(int, VALUE *, VALUE);
extern VALUE Image_read(VALUE, VALUE);
extern VALUE Image_reduce_noise(VALUE, VALUE);
extern VALUE Image_resize(int, VALUE *, VALUE);
extern VALUE Image_resize_bang(int, VALUE *, VALUE);
extern VALUE Image_roll(VALUE, VALUE, VALUE);
extern VALUE Image_rotate(VALUE, VALUE);
extern VALUE Image_rotate_bang(VALUE, VALUE);
extern VALUE Image_sample(int, VALUE *, VALUE);
extern VALUE Image_sample_bang(int, VALUE *, VALUE);
extern VALUE Image_scale(int, VALUE *, VALUE);
extern VALUE Image_scale_bang(int, VALUE *, VALUE);
extern VALUE Image_segment(int, VALUE *, VALUE);
extern VALUE Image_shade(int, VALUE *, VALUE);
extern VALUE Image_sharpen(int, VALUE *, VALUE);
extern VALUE Image_shave(VALUE, VALUE, VALUE);
extern VALUE Image_shave_bang(VALUE, VALUE, VALUE);
extern VALUE Image_shear(VALUE, VALUE, VALUE);
extern VALUE Image_signature(VALUE);
extern VALUE Image_solarize(int, VALUE *, VALUE);
extern VALUE Image_spaceship(VALUE, VALUE);
extern VALUE Image_spread(int, VALUE *, VALUE);
extern VALUE Image_statistics(VALUE);
extern VALUE Image_stegano(VALUE, VALUE, VALUE);
extern VALUE Image_stereo(VALUE, VALUE);
extern VALUE Image_store_pixels(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_strip_bang(VALUE);
extern VALUE Image_swirl(VALUE, VALUE);
extern VALUE Image_texture_flood_fill(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_threshold(VALUE, VALUE);
extern VALUE Image_thumbnail(int, VALUE *, VALUE);
extern VALUE Image_thumbnail_bang(int, VALUE *, VALUE);
extern VALUE Image_tint(int, VALUE *, VALUE);
extern VALUE Image_to_blob(VALUE);
extern VALUE Image_to_color(VALUE, VALUE);
extern VALUE Image_transparent(int, VALUE *, VALUE);
extern VALUE Image_trim(VALUE);
extern VALUE Image_trim_bang(VALUE);
extern VALUE Image_unsharp_mask(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_wave(int, VALUE *, VALUE);
extern VALUE Image_white_threshold(int, VALUE *, VALUE);
extern VALUE Image_write(VALUE, VALUE);

extern VALUE rm_image_new(Image *);


// rmfill.c
#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
VALUE GradientFill_alloc(VALUE);
#else
VALUE GradientFill_new(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
#endif

extern VALUE GradientFill_initialize(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE GradientFill_fill(VALUE, VALUE);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
VALUE TextureFill_alloc(VALUE);
#else
VALUE TextureFill_new(VALUE, VALUE);
#endif

extern VALUE TextureFill_initialize(VALUE, VALUE);
extern VALUE TextureFill_fill(VALUE, VALUE);


// rmutil.c
extern VALUE   AffineMatrix_from_AffineMatrix(AffineMatrix *);
extern VALUE   ChromaticityInfo_to_s(VALUE);
extern VALUE   ChromaticityInfo_new(ChromaticityInfo *);
extern void    Color_to_PixelPacket(PixelPacket *, VALUE);
extern VALUE   Color_to_s(VALUE);
extern VALUE   Color_from_ColorInfo(const ColorInfo *);
extern VALUE   ClassType_new(ClassType);
extern VALUE   ColorspaceType_new(ColorspaceType);
extern VALUE   CompressionType_new(CompressionType);
extern VALUE   EndianType_new(EndianType);
extern VALUE   FilterTypes_new(FilterTypes);
extern VALUE   Font_to_s(VALUE);
extern VALUE   ImageList_cur_image(VALUE);
extern VALUE   ImageMagickError_initialize(int, VALUE *, VALUE);
extern VALUE   ImageType_new(ImageType);
extern VALUE   InterlaceType_new(InterlaceType);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
extern VALUE   Pixel_alloc(VALUE);
#else
extern VALUE   Pixel_new(int, VALUE *, VALUE);
#endif
ATTR_ACCESSOR(Pixel, red)
ATTR_ACCESSOR(Pixel, green)
ATTR_ACCESSOR(Pixel, blue)
ATTR_ACCESSOR(Pixel, opacity)
extern VALUE   Pixel_case_eq(VALUE, VALUE);
extern VALUE   Pixel_clone(VALUE);
extern VALUE   Pixel_dup(VALUE);
extern VALUE   Pixel_fcmp(int, VALUE *, VALUE);
extern VALUE   Pixel_from_color(VALUE, VALUE);
extern VALUE   Pixel_from_HSL(VALUE, VALUE);
extern VALUE   Pixel_initialize(int, VALUE *, VALUE);
extern VALUE   Pixel_init_copy(VALUE, VALUE);
extern VALUE   Pixel_intensity(VALUE);
extern VALUE   Pixel_spaceship(VALUE, VALUE);
extern VALUE   Pixel_to_color(int, VALUE *, VALUE);
extern VALUE   Pixel_to_HSL(VALUE);
extern VALUE   Pixel_to_s(VALUE);
extern VALUE   PixelPacket_to_Color_Name(Image *, PixelPacket *);
extern VALUE   PixelPacket_to_Color_Name_Info(Info *, PixelPacket *);
extern VALUE   Pixel_from_PixelPacket(PixelPacket *);

extern void    Point_to_PointInfo(PointInfo *, VALUE);
extern VALUE   PointInfo_to_Point(PointInfo *);
extern VALUE   PrimaryInfo_to_s(VALUE);
extern VALUE   PrimaryInfo_from_PrimaryInfo(PrimaryInfo *);
extern VALUE   RectangleInfo_to_s(VALUE);
extern VALUE   Rectangle_from_RectangleInfo(RectangleInfo *);
extern VALUE   RenderingIntent_new(RenderingIntent);
extern VALUE   ResolutionType_new(ResolutionType);
extern VALUE   SegmentInfo_to_s(VALUE);
extern VALUE   Segment_from_SegmentInfo(SegmentInfo *);
extern void    AffineMatrix_to_AffineMatrix(AffineMatrix *, VALUE);
extern void    ChromaticityInfo_to_ChromaticityInfo(ChromaticityInfo *, VALUE);
extern void    Color_to_ColorInfo(ColorInfo *, VALUE);
extern void    PrimaryInfo_to_PrimaryInfo(PrimaryInfo *, VALUE);
extern void    Rectangle_to_RectangleInfo(RectangleInfo *, VALUE);
extern void    Segment_to_SegmentInfo(SegmentInfo *, VALUE);
extern void    Font_to_TypeInfo(TypeInfo *, VALUE);
extern void    TypeMetric_to_TypeMetric(TypeMetric *, VALUE);
extern VALUE   Font_from_TypeInfo(TypeInfo *);
extern VALUE   TypeMetric_to_s(VALUE);
extern VALUE   TypeMetric_from_TypeMetric(TypeMetric *);
#if defined(HAVE_GETIMAGESTATISTICS)
extern VALUE   Statistics_new(ImageStatistics *);
#endif

extern VALUE   rm_enum_new(VALUE, VALUE, VALUE);
extern VALUE   rm_no_freeze(VALUE);

#if defined(HAVE_RB_DEFINE_ALLOC_FUNC)
extern VALUE Enum_alloc(VALUE);
#else
extern VALUE Enum_new(VALUE, VALUE, VALUE);
#endif
extern VALUE   Enum_initialize(VALUE, VALUE, VALUE);
extern VALUE   Enum_to_s(VALUE);
extern VALUE   Enum_to_i(VALUE);
extern VALUE   Enum_spaceship(VALUE, VALUE);
extern VALUE   Enum_case_eq(VALUE, VALUE);

#if !defined(StringValuePtr)
extern char *rm_string_value_ptr(volatile VALUE *);
#endif
extern char *rm_string_value_ptr_len(volatile VALUE *, long *);
extern void rm_check_frozen(VALUE);
extern VALUE rm_obj_to_s(VALUE);
double rm_fuzz_to_dbl(VALUE fuzz);

extern void *magick_malloc(const size_t);
extern void magick_free(void *);
extern void *magick_realloc(void *, size_t);
extern void magick_clone_string(char **, const char *);
extern void write_temp_image(Image *, char *);
extern void delete_temp_image(char *);
extern void not_implemented(void);
extern void handle_error(ExceptionInfo *);
extern void handle_all_errors(Image *);
extern void attr_write(VALUE, VALUE);
extern void get_geometry(VALUE, long *, long *, unsigned long *, unsigned long *, int *);
extern Image *toseq(VALUE);
extern void unseq(Image *);

#endif

