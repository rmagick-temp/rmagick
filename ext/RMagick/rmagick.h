/* $Id: rmagick.h,v 1.4 2003/07/19 01:47:03 tim Exp $ */
/*=============================================================================
|               Copyright (C) 2003 by Timothy P. Hunter
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
#include "magick/image.h"       // Get QuantumLeap definition


// Undef ImageMagick's versions of these symbols
#undef PACKAGE_VERSION
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_TARNAME
#include "rmagick_config.h"

#ifndef HAVE_GETNEXTIMAGEINLIST
#define GetNextImageInList(a) (a)->next
#endif

#if defined(HAVE_GETLOCALEEXCEPTIONMESSAGE)
#define GETLOCALEEXCEPTIONMESSAGE(s, t) GetLocaleExceptionMessage(s, t)
#else
#define GETLOCALEEXCEPTIONMESSAGE(s, t) t
#endif


#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

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

#if RUBY_VERSION < 0x168
#define ULONG2NUM(v) UINT2NUM(v)
#endif


// Define a version of StringValuePtr that works in both 1.6 and 1.8.
#if RUBY_VERSION < 0x180
#define STRING_PTR(v) rm_string_value_ptr(&(v))
#else
#define STRING_PTR(v) StringValuePtr(v)
#endif

// Safe replacement for rb_str2cstr
#define STRING_PTR_LEN(v,l) rm_string_value_ptr_len(&(v), &(l))

// Define range of acceptable ImageMagick version numbers

#ifndef GRAPHICSMAGICK
#define MIN_LIBVER 0x0551
#define MAX_LIBVER 0x0558
#endif

#define Q(a) Q2(a)     // Create quoted macro symbol
#define Q2(a) #a

#define DegreesToRadians(x) ((x)*3.14159265358979323846/180.0)

RUBY18(typedef long Strlen_t;)
RUBY16(typedef int Strlen_t;)

typedef ImageInfo Info; // Make type name match class name

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

typedef enum
{
    False = 0,
    True = 1
}
boolean;

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

#ifndef HAVE_EXTENDEDSIGNEDINTEGRALTYPE
typedef off_t ExtendedSignedIntegralType;
#endif

#ifndef HAVE_EXTENDEDUNSIGNEDINTEGRALTYPE
typedef size_t ExtendedUnsignedIntegralType;
#endif

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
EXTERN VALUE Class_Pixel;
EXTERN VALUE Class_Point;
EXTERN VALUE Class_Primary;
EXTERN VALUE Class_Rectangle;
EXTERN VALUE Class_Segment;
EXTERN VALUE Class_TypeMetric;

/*
*   Commonly-used IDs
*/
EXTERN ID new_ID;          // "new"
EXTERN ID push_ID;         // "push"
EXTERN ID length_ID;       // "length"
EXTERN ID values_ID;       // "values"
EXTERN ID cur_image_ID;    // "cur_image"
EXTERN ID call_ID;         // "call"

/*
*   ImageMagick exceptions that have severity >= 400 are errors, < 400 are warnings.
*/
#define ERROR_SEVERITY 400

/*
*   Handle warnings & errors
*/
// For ExceptionInfo structures in auto storage. Destroys the
// ExceptionInfo structure (releasing any IM storage) before
// calling rb_raise.
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
        Data_Get_Struct(self, class, ptr);\
        ptr->attr = R_##type##_to_C_##type(val);\
        return self;\
    }
#define DEF_ATTR_ACCESSOR(class, attr, type)\
    DEF_ATTR_READER(class, attr, type)\
    DEF_ATTR_WRITER(class, attr, type)

/*
 *  Define an Magick module constant from an ImageMagick enumeration value
 */
#define DEF_CONST(constant) rb_define_const(Module_Magick, #constant, INT2FIX(constant))

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
*   Method, external function declarations
*/

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

RUBY16(extern VALUE Info_new(VALUE);)
RUBY18(extern VALUE Info_alloc(VALUE);)

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

RUBY16(extern VALUE Image_new(int, VALUE *, VALUE);)
RUBY16(extern VALUE Image_initialize(VALUE, VALUE, VALUE, VALUE, VALUE);)

RUBY18(extern VALUE Image_alloc(VALUE);)
RUBY18(extern VALUE Image_initialize(int, VALUE *, VALUE);)

extern VALUE Image_adaptive_threshold(int, VALUE *, VALUE);
extern VALUE Image_add_noise(VALUE, VALUE);
extern VALUE Image_affine_transform(VALUE, VALUE);
extern VALUE Image_aref(VALUE, VALUE);
extern VALUE Image_aset(VALUE, VALUE, VALUE);
extern VALUE Image_properties(VALUE);
extern VALUE Image_black_threshold(int, VALUE *, VALUE);
extern VALUE Image_blur_image(int, VALUE *, VALUE);
extern VALUE Image_border(VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_capture(int, VALUE *, VALUE);
extern VALUE Image_changed_q(VALUE);
extern VALUE Image_channel(VALUE, VALUE);
extern VALUE Image_channel_threshold(int, VALUE *, VALUE);
extern VALUE Image_charcoal(int, VALUE *, VALUE);
extern VALUE Image_chop(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_color_flood_fill(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_colorize(int, VALUE *, VALUE);
extern VALUE Image_colormap(int, VALUE *, VALUE);
extern VALUE Image_composite(int, VALUE *, VALUE);
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
extern VALUE Image_gamma_correct(int, VALUE *, VALUE);
extern VALUE Image_gaussian_blur(int, VALUE *, VALUE);
extern VALUE Image_get_pixels(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_gray_q(VALUE);
extern VALUE Image_implode(int, VALUE *, VALUE);
extern VALUE Image_import_pixels(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
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
extern VALUE Image_normalize(VALUE);
extern VALUE Image_oil_paint(int, VALUE *, VALUE);
extern VALUE Image_opaque(VALUE, VALUE, VALUE);
extern VALUE Image_opaque_q(VALUE);
extern VALUE Image_ordered_dither(VALUE);
extern VALUE Image_palette_q(VALUE);
extern VALUE Image_ping(VALUE, VALUE);
extern VALUE Image_pixel_color(int, VALUE *, VALUE);
// VALUE Image_plasma(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_profile_bang(VALUE, VALUE, VALUE);
extern VALUE Image_quantize(int, VALUE *, VALUE);
extern VALUE Image_quantization_error(VALUE);
extern VALUE Image_raise(int, VALUE *, VALUE);
extern VALUE Image_random_channel_threshold(VALUE, VALUE, VALUE);
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
extern VALUE Image_stegano(VALUE, VALUE, VALUE);
extern VALUE Image_stereo(VALUE, VALUE);
extern VALUE Image_store_pixels(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_strip_bang(VALUE);
extern VALUE Image_swirl(VALUE, VALUE);
extern VALUE Image_texture_flood_fill(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_threshold(VALUE, VALUE);
extern VALUE Image_thumbnail(int, VALUE *, VALUE);
extern VALUE Image_thumbnail_bang(int, VALUE *, VALUE);
extern VALUE Image_to_blob(VALUE);
extern VALUE Image_to_color(VALUE, VALUE);
extern VALUE Image_transparent(int, VALUE *, VALUE);
extern VALUE Image_unsharp_mask(VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE Image_wave(int, VALUE *, VALUE);
extern VALUE Image_white_threshold(int, VALUE *, VALUE);
extern VALUE Image_write(VALUE, VALUE);

extern VALUE rm_image_new(Image *);

// rmfill.c

RUBY16(VALUE GradientFill_new(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);)
RUBY18(VALUE GradientFill_alloc(VALUE));

extern VALUE GradientFill_initialize(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
extern VALUE GradientFill_fill(VALUE, VALUE);

RUBY16(VALUE TextureFill_new(VALUE, VALUE);)
RUBY18(VALUE TextureFill_alloc(VALUE));

extern VALUE TextureFill_initialize(VALUE, VALUE);
extern VALUE TextureFill_fill(VALUE, VALUE);

// rmutil.c
extern VALUE   ImageList_cur_image(VALUE);
extern VALUE   PixelPacket_to_Color_Name(Image *, PixelPacket *);
extern VALUE   PixelPacket_to_Color_Name_Info(Info *, PixelPacket *);
extern VALUE   AffineMatrix_to_Struct(AffineMatrix *);
extern void    Struct_to_AffineMatrix(AffineMatrix *, VALUE);
extern VALUE   ColorInfo_to_Struct(const ColorInfo *);
extern void    Struct_to_ColorInfo(ColorInfo *, VALUE);
extern VALUE   Color_to_s(VALUE);
extern VALUE   PixelPacket_to_Struct(PixelPacket *);
extern void    Struct_to_PixelPacket(PixelPacket *, VALUE);
extern void    Color_to_PixelPacket(PixelPacket *, VALUE);
extern VALUE   Pixel_from_color(VALUE, VALUE);
extern VALUE   Pixel_to_color(int, VALUE *, VALUE);
extern VALUE   Pixel_to_HSL(VALUE);
extern VALUE   Pixel_from_HSL(VALUE, VALUE);
extern VALUE   Pixel_to_s(VALUE);
extern VALUE   PrimaryInfo_to_Struct(PrimaryInfo *);
extern void    Struct_to_PrimaryInfo(PrimaryInfo *, VALUE);
extern VALUE   PrimaryInfo_to_s(VALUE);
extern VALUE   ChromaticityInfo_to_Struct(ChromaticityInfo *);
extern void    Struct_to_ChromaticityInfo(ChromaticityInfo *, VALUE);
extern VALUE   ChromaticityInfo_to_s(VALUE);
extern void    Struct_to_RectangleInfo(RectangleInfo *, VALUE);
extern VALUE   RectangleInfo_to_Struct(RectangleInfo *);
extern VALUE   RectangleInfo_to_s(VALUE);
extern void    Struct_to_SegmentInfo(SegmentInfo *, VALUE);
extern VALUE   SegmentInfo_to_Struct(SegmentInfo *);
extern VALUE   SegmentInfo_to_s(VALUE);
extern VALUE   TypeInfo_to_Struct(TypeInfo *);
extern void    Struct_to_TypeInfo(TypeInfo *, VALUE);
extern VALUE   Font_to_s(VALUE);
extern void    Struct_to_TypeMetric(TypeMetric *, VALUE);
extern VALUE   TypeMetric_to_Struct(TypeMetric *);
extern VALUE   TypeMetric_to_s(VALUE);
extern VALUE   ImageMagickError_initialize(VALUE, VALUE, VALUE);

extern const char *    Str_to_CompositeOperator(VALUE);
extern AlignType       Num_to_AlignType(VALUE);
extern ChannelType     Num_to_ChannelType(VALUE);
extern ClassType       Num_to_ClassType(VALUE);
extern ComplianceType  Num_to_ComplianceType(VALUE);
extern CompressionType     Num_to_CompressionType(VALUE);
extern CompositeOperator   Num_to_CompositeOperator(VALUE);
extern DecorationType  Num_to_DecorationType(VALUE);
#if HAVE_DISPOSETYPE
extern DisposeType     Num_to_DisposeType(VALUE);
#endif
extern FilterTypes     Num_to_FilterType(VALUE);
extern GravityType     Num_to_GravityType(VALUE);
extern InterlaceType   Num_to_InterlaceType(VALUE);
extern ImageType       Num_to_ImageType(VALUE);
extern ColorspaceType  Num_to_ColorspaceType(VALUE);
extern NoiseType       Num_to_NoiseType(VALUE);
extern ResolutionType  Num_to_ResolutionType(VALUE);
extern RenderingIntent Num_to_RenderingIntent(VALUE);
extern PaintMethod     Num_to_PaintMethod(VALUE);
extern StretchType     Num_to_StretchType(VALUE);
extern StyleType       Num_to_StyleType(VALUE);
#ifndef StringValuePtr
extern char *rm_string_value_ptr(volatile VALUE *);
#endif
extern char *rm_string_value_ptr_len(volatile VALUE *, Strlen_t *);

extern void *magick_malloc(const size_t);
extern void magick_free(void *);
extern void *magick_realloc(void *, size_t);
extern void magick_clone_string(char **, const char *);
extern void write_temp_image(Image *, char *);
extern void delete_temp_image(char *);
extern void handle_error(ExceptionInfo *);
extern void attr_write(VALUE, VALUE);
extern Image *toseq(VALUE);
extern void unseq(Image *);

#endif

