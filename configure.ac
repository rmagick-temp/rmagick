
# RMagick configure.ac

AC_INIT(RMagick, 0.0.0, rmagick@rubyforge.org, RMagick-0.0.0.tar.gz)
echo "Configuring ${PACKAGE_STRING}"

AC_CONFIG_HEADER([ext/RMagick/rmagick_config.h])

# Save caller's CFLAGS before configure changes it.
rm_saved_CFLAGS=${CFLAGS}

# Check for required programs
AC_PROG_CC
AC_PROG_CC_STDC

AC_PATH_PROG(RUBY, ruby,)
if test -z "$RUBY"; then
    AC_MSG_ERROR([Can't install RMagick. Ruby must be installed first.])
fi

AC_PATH_PROG(MAGICKCONFIG, Magick-config)
if test -z "$MAGICKCONFIG"; then
    AC_MSG_ERROR([Can't install RMagick. Can't find Magick-config program.])
fi

MAGICKVERS=`Magick-config --version | tr -d .`
if test $MAGICKVERS -lt 625; then
    MAGICKVERS=`${MAGICKCONFIG} --version`
    AC_MSG_ERROR([Can't install RMagick. You must have ImageMagick 6.2.5 or later installed. Your version of ImageMagick is ${MAGICKVERS}])
fi


# Append ImageMagick's lists of flags to the user's lists (if any)
LDFLAGS="${LDFLAGS} `Magick-config --ldflags`"
CPPFLAGS="${CPPFLAGS} `Magick-config --cppflags`"

AC_SUBST(LDFLAGS, ${LDFLAGS})
AC_SUBST(CFLAGS, ${rm_saved_CFLAGS})
AC_SUBST(CPPFLAGS, ${CPPFLAGS})
AC_SUBST(LOCAL_LIBS, `Magick-config --libs`)

AC_CHECK_LIB(Magick, InitializeMagick, [],
    [AC_MSG_ERROR([Can't install RMagick. Can't find libMagick or one of the dependent libraries.
                  Check the config.log file for more detailed information.])],
    [`Magick-config --libs`])


# What standard functions?
AC_CHECK_FUNCS([snprintf])

# Append ImageMagick's lists of libs to the user's list (if any)
LIBS="${LIBS} `Magick-config --libs`"

# What ImageMagick/GraphicsMagick functions?
AC_CHECK_FUNCS([AcquireMagickMemory])

# We need to be able to query this later in the configure script, so set a shell variable here
# as well as defining a symbol.
AC_CHECK_FUNC(AcquireStringInfo,
              rm_have_acquirestringinfo=yes,
              rm_have_acquirestringinfo=no)
if test "$rm_have_acquirestringinfo" = yes; then
   AC_DEFINE(HAVE_ACQUIRESTRINGINFO)
fi

AC_CHECK_FUNCS([AdaptiveBlurImageChannel AdaptiveResizeImage AdaptiveSharpenImage])
AC_CHECK_FUNCS([AdaptiveThresholdImage])
AC_CHECK_FUNCS([AddNoiseImageChannel BilevelImageChannel BlackThresholdImage])
AC_CHECK_FUNCS([BlurImageChannel ClearMagickException CompareImageChannels])
AC_CHECK_FUNCS([CompareImageLayers CompositeImageChannel ContrastStretchImageChannel])
AC_CHECK_FUNCS([ConvolveImageChannel EvaluateImageChannel ExportImagePixels])
AC_CHECK_FUNCS([FxImageChannel GammaImageChannel])
AC_CHECK_FUNCS([GaussianBlurImageChannel])

# We need to be able to query these symbols later in the configure script, so set a shell
# variable here as well as defining symbols.
AC_CHECK_FUNC(GetColorInfoList,
              rm_have_getcolorinfolist=yes,
              rm_have_getcolorinfolist=no)
if test "$rm_have_getcolorinfolist" = yes; then
   AC_DEFINE(HAVE_GETCOLORINFOLIST)
fi

AC_CHECK_FUNCS([GetImageChannelDepth GetImageChannelDistortion])
AC_CHECK_FUNCS([GetImageChannelExtrema GetImageChannelMean GetImageClipMask])
AC_CHECK_FUNCS([GetImageHistogram GetImageProfile])

# We need to be able to query this later in the configure script, so set a shell variable here
# as well as defining a symbol.
AC_CHECK_FUNC(GetImageQuantumDepth,
              rm_have_getimagequantumdepth=yes,
              rm_have_getimagequantumdepth=no)
if test "$rm_have_getimagequantumdepth" = yes; then
   AC_DEFINE(HAVE_GETIMAGEQUANTUMDEPTH)
fi

AC_CHECK_FUNC(GetMagickPixelPacket,
              rm_have_getmagickpixelpacket=yes,
              rm_have_getmagickpixelpacket=no)
if test "$rm_have_getmagickpixelpacket" = yes; then
   AC_DEFINE(HAVE_GETMAGICKPIXELPACKET)
fi

AC_CHECK_FUNC(GetMagickInfoList,
              rm_have_getmagickinfolist=yes,
              rm_have_getmagickinfolist=no)
if test "$rm_have_getmagickinfolist" = yes; then
   AC_DEFINE(HAVE_GETMAGICKINFOLIST)
fi

AC_CHECK_FUNCS([GetMagickResourceLimit GetMultilineTypeMetrics GetNextImageAttribute GetNextImageProfile])

AC_CHECK_FUNC(GetTypeInfoList,
              rm_have_gettypeinfolist=yes,
              rm_have_gettypeinfolist=no)
if test "$rm_have_gettypeinfolist" = yes; then
   AC_DEFINE(HAVE_GETTYPEINFOLIST)
fi

AC_CHECK_FUNCS([ImagesToBlob ImportImagePixels])
AC_CHECK_FUNCS([InheritException InterpolatePixelColor InterpretImageAttributes])
AC_CHECK_FUNCS([IsImageSimilar LinearStretchImage NegateImageChannel])
AC_CHECK_FUNCS([NormalizeImageChannel OrderedPosterizeImageChannel])
AC_CHECK_FUNCS([ParseSizeGeometry PolaroidImage PosterizeImage PreviewImage])
AC_CHECK_FUNCS([RadialBlurImage RadialBlurImageChannel])
AC_CHECK_FUNCS([RandomChannelThresholdImage RandomThresholdImageChannel])
AC_CHECK_FUNCS([RecolorImage SeparateImageChannel SepiaToneImage])
AC_CHECK_FUNCS([SetImageBackgroundColor SetImageChannelDepth SetImageColorspace SetImageExtent])
AC_CHECK_FUNCS([SetImageInfoFile SetImageOption SetImageStorageClass])
AC_CHECK_FUNCS([SetImageProgressMonitor ShadowImage SharpenImageChannel])
AC_CHECK_FUNCS([SigmoidalContrastImageChannel SketchImage SpliceImage StripImage])
AC_CHECK_FUNCS([ThresholdImageChannel TintImage TransposeImage TransverseImage UniqueImageColors])
AC_CHECK_FUNCS([UnsharpMaskImageChannel VignetteImage WhiteThresholdImage XImportImage])

# What typedefs?

# Need a separate test for this so we can set an env. variable to test below.
AC_CHECK_TYPE(MagickBooleanType,
              rm_have_magickbooleantype=yes,
              rm_have_magickbooleantype=no,
              [#include <stdio.h>
               #if HAVE_SYS_TYPES_H
               #include <sys/types.h>
               #endif
               #include "magick/api.h"])
if test "$rm_have_magickbooleantype" = yes; then
   AC_DEFINE(HAVE_MAGICKBOOLEANTYPE)
fi

AC_CHECK_TYPES([MagickPixelPacket],
                [], [],
                [#include <stdio.h>
                 #if HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #include "magick/api.h"])


AC_CHECK_MEMBERS([ExceptionInfo.module],,,
                [#include <stdio.h>
                 #if HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #include "magick/api.h"])

AC_CHECK_MEMBERS([Image.bias, Image.orientation, Image.quality, Image.ticks_per_second, Image.transparent_color],,,
                [#include <stdio.h>
                 #if HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #include "magick/api.h"])

AC_CHECK_MEMBERS([ImageInfo.orientation, ImageInfo.channel],,,
                [#include <stdio.h>
                 #if HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #include "magick/api.h"])


# Don't use AC_CHECK_MEMBERS!
AC_CACHE_CHECK(for Image.extract_info, rm_cv_member_Image_extract_info,
  AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdio.h>
#include "magick/api.h"],
    [Image i; i.extract_info.x = 0;],
    rm_cv_member_Image_extract_info=yes,
    rm_cv_member_Image_extract_info=no))
if test "$rm_cv_member_Image_extract_info" = yes; then
  AC_DEFINE(HAVE_IMAGE_EXTRACT_INFO)
fi

AC_CACHE_CHECK(for CineonLogRGBColorspace enum value, rm_cv_enum_cineonlogrgbcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = CineonLogRGBColorspace],
     rm_cv_enum_cineonlogrgbcolorspace='yes',
     rm_cv_enum_cineonlogrgbcolorspace='no')])

if test "$rm_cv_enum_cineonlogrgbcolorspace" = yes; then
  AC_DEFINE(HAVE_CINEONLOGRGBCOLORSPACE)
fi

AC_CACHE_CHECK(for HSBColorspace enum value, rm_cv_enum_hsbcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = HSBColorspace],
     rm_cv_enum_hsbcolorspace='yes',
     rm_cv_enum_hsbcolorspace='no')])

if test "$rm_cv_enum_hsbcolorspace" = yes; then
  AC_DEFINE(HAVE_HSBCOLORSPACE)
fi

AC_CACHE_CHECK(for LABColorspace enum value, rm_cv_enum_labcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = LABColorspace],
     rm_cv_enum_labcolorspace='yes',
     rm_cv_enum_labcolorspace='no')])

if test "$rm_cv_enum_labcolorspace" = yes; then
  AC_DEFINE(HAVE_LABCOLORSPACE)
fi

AC_CACHE_CHECK(for Rec601LumaColorspace enum value, rm_cv_enum_rec601lumacolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = Rec601LumaColorspace],
     rm_cv_enum_rec601lumacolorspace='yes',
     rm_cv_enum_rec601lumacolorspace='no')])

if test "$rm_cv_enum_rec601lumacolorspace" = yes; then
  AC_DEFINE(HAVE_REC601LUMACOLORSPACE)
fi

AC_CACHE_CHECK(for Rec601YCbCrColorspace enum value, rm_cv_enum_rec601ycbcrcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = Rec601YCbCrColorspace],
     rm_cv_enum_rec601ycbcrcolorspace='yes',
     rm_cv_enum_rec601ycbcrcolorspace='no')])

if test "$rm_cv_enum_rec601ycbcrcolorspace" = yes; then
  AC_DEFINE(HAVE_REC601YCBCRCOLORSPACE)
fi

AC_CACHE_CHECK(for Rec709LumaColorspace enum value, rm_cv_enum_rec709lumacolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = Rec709LumaColorspace],
     rm_cv_enum_rec709lumacolorspace='yes',
     rm_cv_enum_rec709lumacolorspace='no')])

if test "$rm_cv_enum_rec709lumacolorspace" = yes; then
  AC_DEFINE(HAVE_REC709LUMACOLORSPACE)
fi

AC_CACHE_CHECK(for Rec709YCbCrColorspace enum value, rm_cv_enum_rec709ycbcrcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = Rec709YCbCrColorspace],
     rm_cv_enum_rec709ycbcrcolorspace='yes',
     rm_cv_enum_rec709ycbcrcolorspace='no')])

if test "$rm_cv_enum_rec709ycbcrcolorspace" = yes; then
  AC_DEFINE(HAVE_REC709YCBCRCOLORSPACE)
fi

AC_CACHE_CHECK(for LogColorspace enum value, rm_cv_enum_logcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = LogColorspace],
     rm_cv_enum_logcolorspace='yes',
     rm_cv_enum_logcolorspace='no')])

if test "$rm_cv_enum_logcolorspace" = yes; then
  AC_DEFINE(HAVE_LOGCOLORSPACE)
fi

AC_CACHE_CHECK(for ReplaceCompositeOp enum value, rm_cv_enum_replacecompositeop,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [CompositeOperator x; x = ReplaceCompositeOp],
     rm_cv_enum_replacecompositeop='yes',
     rm_cv_enum_replacecompositeop='no')])

if test "$rm_cv_enum_replacecompositeop" = yes; then
  AC_DEFINE(HAVE_REPLACECOMPOSITEOP)
fi

AC_CACHE_CHECK(for DstCompositeOp enum value, rm_cv_enum_dstcompositeop,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [CompositeOperator x; x = DstCompositeOp],
     rm_cv_enum_dstcompositeop='yes',
     rm_cv_enum_dstcompositeop='no')])

if test "$rm_cv_enum_dstcompositeop" = yes; then
  AC_DEFINE(HAVE_DSTCOMPOSITEOP)
fi

AC_CACHE_CHECK(for ColorDodgeCompositeOp enum value, rm_cv_enum_ColorDodgeCompositeOp,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [CompositeOperator x; x = ColorDodgeCompositeOp],
     rm_cv_enum_ColorDodgeCompositeOp='yes',
     rm_cv_enum_ColorDodgeCompositeOp='no')])

if test "$rm_cv_enum_ColorDodgeCompositeOp" = yes; then
  AC_DEFINE(HAVE_COLORDODGECOMPOSITEOP)
fi

AC_CACHE_CHECK(for IndexChannel enum value, rm_cv_enum_indexchannel,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ChannelType x; x = IndexChannel],
     rm_cv_enum_indexchannel='yes',
     rm_cv_enum_indexchannel='no')])

if test "$rm_cv_enum_indexchannel" = yes; then
  AC_DEFINE(HAVE_INDEXCHANNEL)
fi

AC_CACHE_CHECK(for GrayChannel enum value, rm_cv_enum_graychannel,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ChannelType x; x = GrayChannel],
     rm_cv_enum_graychannel='yes',
     rm_cv_enum_graychannel='no')])

if test "$rm_cv_enum_graychannel" = yes; then
  AC_DEFINE(HAVE_GRAYCHANNEL)
fi

AC_CACHE_CHECK(for AllChannels enum value, rm_cv_enum_allchannels,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ChannelType x; x = AllChannels],
     rm_cv_enum_allchannels='yes',
     rm_cv_enum_allchannels='no')])

if test "$rm_cv_enum_allchannels" = yes; then
  AC_DEFINE(HAVE_ALLCHANNELS)
fi

AC_CACHE_CHECK(for JPEG2000Compression enum value, rm_cv_enum_jpeg2000compression,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [CompressionType x; x = JPEG2000Compression],
     rm_cv_enum_jpeg2000compression='yes',
     rm_cv_enum_jpeg2000compression='no')])

if test "$rm_cv_enum_jpeg2000compression" = yes; then
  AC_DEFINE(HAVE_JPEG2000COMPRESSION)
fi

AC_CACHE_CHECK(for UndefinedGravity enum value, rm_cv_enum_undefinedgravity,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [GravityType x; x = UndefinedGravity],
     rm_cv_enum_undefinedgravity='yes',
     rm_cv_enum_undefinedgravity='no')])

if test "$rm_cv_enum_undefinedgravity" = yes; then
  AC_DEFINE(HAVE_UNDEFINEDGRAVITY)
fi

AC_CACHE_CHECK(for QuantumPixel enum value, rm_cv_enum_quantumpixel,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [StorageType x; x = QuantumPixel],
     rm_cv_enum_quantumpixel='yes',
     rm_cv_enum_quantumpixel='no')])
if test "$rm_cv_enum_quantumpixel" = yes; then
  AC_DEFINE(HAVE_QUANTUMPIXEL)
fi

AC_CACHE_CHECK(for TransparentVirtualPixelMethod enum value, rm_cv_enum_transparentvirtualpixelmethod,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [VirtualPixelMethod x; x = TransparentVirtualPixelMethod],
     rm_cv_enum_transparentvirtualpixelmethod='yes',
     rm_cv_enum_transparentvirtualpixelmethod='no')])

if test "$rm_cv_enum_transparentvirtualpixelmethod" = yes; then
  AC_DEFINE(HAVE_TRANSPARENTVIRTUALPIXELMETHOD)
fi

AC_CACHE_CHECK(for CoalesceLayer enum value, rm_cv_enum_coalescelayer,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [MagickLayerMethod x; x = CoalesceLayer],
     rm_cv_enum_coalescelayer='yes',
     rm_cv_enum_coalescelayer='no')])

if test "$rm_cv_enum_coalescelayer" = yes; then
  AC_DEFINE(HAVE_COALESCELAYER)
fi

AC_CACHE_CHECK(for MaxEvaluateOperator enum value, rm_cv_enum_maxevaluateoperator,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [MagickEvaluateOperator x; x = MaxEvaluateOperator],
     rm_cv_enum_maxevaluateoperator='yes',
     rm_cv_enum_maxevaluateoperator='no')])

if test "$rm_cv_enum_maxevaluateoperator" = yes; then
  AC_DEFINE(HAVE_MAXEVALUATEOPERATOR)
fi

AC_CACHE_CHECK(for PaletteBilevelMatteType enum value, rm_cv_enum_palettebilevelmattetype,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ImageType x; x = PaletteBilevelMatteType],
     rm_cv_enum_palettebilevelmattetype='yes',
     rm_cv_enum_palettebilevelmattetype='no')])

if test "$rm_cv_enum_palettebilevelmattetype" = yes; then
  AC_DEFINE(HAVE_PALETTEBILEVELMATTETYPE)
fi

if test "$rm_have_getimagequantumdepth" = yes; then
   # IM 6.0.5 changed the number of arguments to GetImageQuantumDepth.
   AC_CACHE_CHECK(if GetImageQuantumDepth has only 1 argument, rm_cv_args_get_image_quantum_depth,
     [AC_TRY_COMPILE(
   [#include <stdio.h>
   #if HAVE_SYS_TYPES_H
   #include <sys/types.h>
   #endif
   #include "magick/api.h"],
        [Image *image; (void)GetImageQuantumDepth(image)],
        rm_cv_args_get_image_quantum_depth='yes',
        rm_cv_args_get_image_quantum_depth='no')])

   if test "$rm_cv_args_get_image_quantum_depth" = yes; then
      AC_DEFINE(HAVE_OLD_GETIMAGEQUANTUMDEPTH)
   fi
fi

if test "$rm_have_getcolorinfolist" = yes; then
   # IM 6.1.3 changed the number of arguments to GetColorInfoList
   AC_CACHE_CHECK(if GetColorInfoList has only 2 arguments, rm_cv_args_get_color_info_list,
     [AC_TRY_COMPILE(
   [#include <stdio.h>
   #if HAVE_SYS_TYPES_H
   #include <sys/types.h>
   #endif
   #include "magick/api.h"],
        [const char *pattern; unsigned long ncolors; (void)GetColorInfoList(pattern, &ncolors)],
        rm_cv_args_get_color_info_list='yes',
        rm_cv_args_get_color_info_list='no')])

   if test "$rm_cv_args_get_color_info_list" = yes; then
      AC_DEFINE(HAVE_OLD_GETCOLORINFOLIST)
   fi
fi

if test "$rm_have_gettypeinfolist" = yes; then
   # IM 6.1.3 changed the number of arguments to GetTypeInfoList
   AC_CACHE_CHECK(if GetTypeInfoList has only 2 arguments, rm_cv_args_get_type_info_list,
     [AC_TRY_COMPILE(
   [#include <stdio.h>
   #if HAVE_SYS_TYPES_H
   #include <sys/types.h>
   #endif
   #include "magick/api.h"],
        [const char *pattern; unsigned long ncolors; (void)GetTypeInfoList(pattern, &ncolors)],
        rm_cv_args_get_type_info_list='yes',
        rm_cv_args_get_type_info_list='no')])

   if test "$rm_cv_args_get_type_info_list" = yes; then
      AC_DEFINE(HAVE_OLD_GETTYPEINFOLIST)
   fi
fi

if test "$rm_have_getmagickinfolist" = yes; then
   # IM 6.1.3 changed the number of arguments to GetMagickInfoList
   AC_CACHE_CHECK(if GetMagickInfoList has only 2 arguments, rm_cv_args_get_magick_info_list,
     [AC_TRY_COMPILE(
   [#include <stdio.h>
   #if HAVE_SYS_TYPES_H
   #include <sys/types.h>
   #endif
   #include "magick/api.h"],
        [const char *pattern; unsigned long ncolors; (void)GetMagickInfoList(pattern, &ncolors)],
        rm_cv_args_get_magick_info_list='yes',
        rm_cv_args_get_magick_info_list='no')])

   if test "$rm_cv_args_get_magick_info_list" = yes; then
      AC_DEFINE(HAVE_OLD_GETMAGICKINFOLIST)
   fi
fi

# IM 6.2.1 changed the return value from RemoveImageProfile from StringInfo * to
# MagickBooleanType. The presence of StringInfo implies RemoveImageProfile.
if test "$rm_have_acquirestringinfo" = yes -a "$rm_have_magickbooleantype" = yes; then
   AC_CACHE_CHECK(if RemoveImageProfile returns MagickBooleanType, rv_cv_args_remove_image_profile,
     [AC_TRY_COMPILE(
   [#include <stdio.h>
   #if HAVE_SYS_TYPES_H
   #include <sys/types.h>
   #endif
   #include "magick/api.h"],
        [MagickBooleanType bool; Image *image; bool=RemoveImageProfile(image, "icc")],
        rv_cv_args_remove_image_profile='yes',
        rv_cv_args_remove_image_profile='no')])

   if test "$rv_cv_args_remove_image_profile" = yes; then
      AC_DEFINE(HAVE_NEW_REMOVEIMAGEPROFILE)
   fi
fi


# The MagickPixelPacket type was introduced in IM 6.0.0. The GetMagickPixelPacket
# function was introduced in 6.2.0. IM 6.3.0 changed the type of the 'color'
# member of the ColorInfo structure from PixelPacket to MagickPixelPacket.
if test "$rm_have_getmagickpixelpacket" = yes; then
   AC_CACHE_CHECK(if ColorInfo.color is a MagickPixelPacket, rv_cv_struct_color_info_color,
     [AC_TRY_COMPILE(
   [#include <stdio.h>
   #if HAVE_SYS_TYPES_H
   #include <sys/types.h>
   #endif
   #include "magick/api.h"],
        [ColorInfo color_info; MagickPixelPacket p; color_info.color = p],
        rv_cv_struct_color_info_color='yes',
        rv_cv_struct_color_info_color='no')])

   if test "$rv_cv_struct_color_info_color" = yes; then
      AC_DEFINE(HAVE_NEW_COLORINFO)
   fi
fi

# Where is ****Magick installed?
MAGICKPREFIX=`Magick-config --prefix`

# Options to pass to ruby config step

RUBY_CONFIG_OPTS=''

if test "x$prefix" != xNONE; then
  RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --prefix=$prefix"
fi
if test `eval echo $bindir` != NONE/bin; then
  RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --bin-dir=`eval echo $bindir`"
fi
if test `eval echo $datadir` != ${prefix}/share; then
  RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --data-dir=`eval echo $datadir`"
fi

AC_ARG_WITH(ruby-prog,
    AS_HELP_STRING([--with-ruby-prog=NAME], [the ruby program used for installation [[PREFIX/bin/ruby]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --ruby-prog=$withval"
     RUBY=$withval], )

AC_ARG_WITH(rbconfig,
    AS_HELP_STRING([--with-rbconfig=PATH], [your rbconfig to load [[ruby's]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --rbconfig=$withval"], )

AC_ARG_WITH(doc-dir,
    AS_HELP_STRING([--with-doc-dir=DIR], [RMagick documentation directory [[PREFIX/share/RMagick]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --doc-dir=$withval"], )

AC_ARG_WITH(std-ruby,
    AS_HELP_STRING([--with-std-ruby=DIR], [the directory for standard ruby libraries [[PREFIX/lib/ruby/1.x]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --std-ruby=$withval"], )

AC_ARG_WITH(site-ruby,
    AS_HELP_STRING([--with-site-ruby=DIR], [the directory for non-standard ruby libraries [[PREFIX/lib/ruby/site_ruby/1.x]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --site-ruby=$withval"], )

AC_ARG_WITH(site-ruby-common,
    AS_HELP_STRING([--with-site-ruby-common=DIR], [the directory for version-independent non-standard ruby libraries [[PREFIX/lib/ruby/site_ruby]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --site-ruby-common=$withval"], )

AC_ARG_WITH(rb-dir,
    AS_HELP_STRING([--with-rb-dir=DIR], [the directory for ruby scripts [[PREFIX/lib/ruby/site_ruby/1.x]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --rb-dir=$withval"], )

AC_ARG_WITH(so-dir,
    AS_HELP_STRING([--with-so-dir=DIR], [the directory for ruby extensions]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --so-dir=$withval"], )

AC_ARG_WITH(ruby-path,
    AS_HELP_STRING([--with-ruby-path=DIR], [path to set to #! line [PREFIX/bin/ruby]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --ruby-path=$withval"], )

AC_ARG_WITH(make-prog,
    AS_HELP_STRING([--with-make-prog=NAME], [the make program to compile ruby extensions [[make]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --make-prog=$withval"], )

AC_ARG_ENABLE(without-ext,
    AS_HELP_STRING([--enable-without-ext], [does not compile/install ruby extensions [[--disable-without-ext]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --without-ext"], )

AC_ARG_ENABLE(no-harm,
    AS_HELP_STRING([--enable-no-harm], [only display what to do if given [[--disable-no-harm]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --no-harm"], )

AC_ARG_ENABLE(allow-example-errors,
    AS_HELP_STRING([--enable-allow-example-errors], [allow installation to proceed even if a lot of examples fail [[--disable-allow-example-errors]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --allow-example-errors"], )

AC_ARG_ENABLE(htmldoc,
    AS_HELP_STRING([--disable-htmldoc], [skip creating the HTML documentation and examples [[--enable-htmldoc]]]),
    [if test "$enable_htmldoc" = no; then
        RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --disable-htmldoc"
     fi], )


RUBY_VERSION_STRING=`$RUBY --version`
AC_DEFINE_UNQUOTED(RUBY_VERSION_STRING, "$RUBY_VERSION_STRING")
AC_SUBST(RUBY)
AC_SUBST(RUBY_CONFIG_OPTS)

AC_CONFIG_FILES(Makefile ext/RMagick/extconf.rb metaconfig)
AC_OUTPUT

echo ""
echo ""
echo "This installation of $PACKAGE_STRING is configured for"
echo "$RUBY_VERSION_STRING and ImageMagick `Magick-config --version`."


