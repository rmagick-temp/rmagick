
# RMagick configure.ac

AC_INIT(RMagick, x.y.z, rmagick@rubyforge.org, RMagick-x.y.z.tar.gz)
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

# See if user prefers GraphicsMagick over ImageMagick
AC_ARG_WITH(graphics-magick,
    AC_HELP_STRING([--with-graphics-magick], [configure RMagick with GraphicsMagick]),
    [with_graphics_magick=$withval],
    [with_graphics_magick='no'])

if test "$with_graphics_magick" != 'no' ; then
    AC_CHECK_PROGS(MAGICKCONFIG, GraphicsMagick-config)
    if test -z "$MAGICKCONFIG"; then
        AC_MSG_ERROR([Can't install RMagick. Can't find GraphicsMagick-config program.])
    fi
    MAGICKNAME=GraphicsMagick
    MAGICKLIB=GraphicsMagick
    AC_DEFINE(GRAPHICSMAGICK,1)

# Otherwise see which one is installed
else
    AC_CHECK_PROGS(MAGICKCONFIG, [Magick-config GraphicsMagick-config])
    if test -z "$MAGICKCONFIG"; then
        AC_MSG_ERROR([Can't install RMagick. Can't find Magick-config or GraphicsMagick-config program.])
    else
        case "$MAGICKCONFIG" in
            Magick-config )
                MAGICKNAME=ImageMagick
                MAGICKLIB=Magick
                ;;
            GraphicsMagick-config )
                MAGICKNAME=GraphicsMagick
                MAGICKLIB=GraphicsMagick
                AC_DEFINE(GRAPHICSMAGICK,1)
                ;;
        esac
    fi
fi

# Append ***Magick's lists of flags to the user's lists (if any)
LDFLAGS="${LDFLAGS} `${MAGICKCONFIG} --ldflags`"
CPPFLAGS="${CPPFLAGS} `${MAGICKCONFIG} --cppflags`"

AC_SUBST(LDFLAGS, ${LDFLAGS})
AC_SUBST(CFLAGS, ${rm_saved_CFLAGS})
AC_SUBST(CPPFLAGS, ${CPPFLAGS})
AC_SUBST(LOCAL_LIBS, `${MAGICKCONFIG} --libs`)

AC_CHECK_LIB($MAGICKLIB, InitializeMagick, [],
    [AC_MSG_ERROR([Can't install RMagick. Can't find lib${MAGICKLIB}])],
    [`${MAGICKCONFIG} --libs`])


AC_SUBST(MAGICKLIB)
AC_SUBST(MAGICKNAME)
AC_SUBST(MAGICKCONFIG)
AC_DEFINE_UNQUOTED(MAGICKNAME, $MAGICKNAME)

AC_CHECK_HEADER(magick/api.h, [],
    [AC_MSG_ERROR([Can't find magick/api.h])])

# What standard functions?
AC_CHECK_FUNCS([snprintf])

# Append ***Magick's lists of libs to the user's list (if any)
LIBS="${LIBS} `${MAGICKCONFIG} --libs`"

# What ImageMagick/GraphicsMagick functions?
AC_CHECK_FUNCS([AdaptiveThresholdImage AppendImageToList LevelImageChannel])
AC_CHECK_FUNCS([ThumbnailImage RemoveFirstImageFromList GetNextImageInList])
AC_CHECK_FUNCS([SetLogFormat XImportImage ExportImagePixels ImportImagePixels])
AC_CHECK_FUNCS([ThresholdImageChannel RandomChannelThresholdImage StripImage])
AC_CHECK_FUNCS([AcquireMagickMemory AcquireStringInfo BlackThresholdImage WhiteThresholdImage])
AC_CHECK_FUNCS([GetMagickInfoArray GetColorInfoArray GetLocaleExceptionMessage])
AC_CHECK_FUNCS([GetMagickGeometry ParseSizeGeometry TintImage GetColorHistogram])
AC_CHECK_FUNCS([GetImageHistogram FuzzyColorCompare GetImageProfile GetNextImageProfile])
AC_CHECK_FUNCS([GetImageChannelDepth GetImageChannelExtrema GetImageChannelMean])
AC_CHECK_FUNCS([CompareImageChannels PreviewImage SetImageColorspace])

# We need to be able to query this later in the configure script, so set a shell variable here
# as well as defining a symbol.
AC_CHECK_FUNC(GetImageQuantumDepth,
              rm_have_getimagequantumdepth=yes,
              rm_have_getimagequantumdepth=no)
if test "$rm_have_getimagequantumdepth" = yes; then
   AC_DEFINE(HAVE_GETIMAGEQUANTUMDEPTH)
fi

AC_CHECK_FUNCS([BilevelImageChannel GammaImageChannel NegateImageChannel])
AC_CHECK_FUNCS([SetImageOption AddDefinitions GrayscalePseudoClassImage])
AC_CHECK_FUNCS([RadialBlurImage RandomThresholdImageChannel SeparateImageChannel])
AC_CHECK_FUNCS([QuantumOperatorRegionImage GetImageStatistics])

# We need to be able to query these symbols later in the configure script, so set a shell
# variable here as well as defining symbols.
AC_CHECK_FUNC(GetColorInfoList,
              rm_have_getcolorinfolist=yes,
              rm_have_getcolorinfolist=no)
if test "$rm_have_getcolorinfolist" = yes; then
   AC_DEFINE(HAVE_GETCOLORINFOLIST)
fi

AC_CHECK_FUNC(GetTypeInfoList,
              rm_have_gettypeinfolist=yes,
              rm_have_gettypeinfolist=no)
if test "$rm_have_gettypeinfolist" = yes; then
   AC_DEFINE(HAVE_GETTYPEINFOLIST)
fi

AC_CHECK_FUNC(GetMagickInfoList,
              rm_have_getmagickinfolist=yes,
              rm_have_getmagickinfolist=no)
if test "$rm_have_getmagickinfolist" = yes; then
   AC_DEFINE(HAVE_GETMAGICKINFOLIST)
fi

AC_CHECK_FUNCS([EvaluateImageChannel PosterizeImage GaussianBlurImageChannel])
AC_CHECK_FUNCS([ConvolveImageChannel SpliceImage SharpenImageChannel])
AC_CHECK_FUNCS([SetImageChannelDepth BlurImageChannel GetMultilineTypeMetrics])
AC_CHECK_FUNCS([NormalizeImageChannel])

# What typedefs?
AC_CHECK_TYPES([DisposeType,
                magick_int64_t,
                magick_uint64_t,
                ExtendedSignedIntegralType,
                ExtendedUnsignedIntegralType,
                MagickOffsetType,
                MagickSizeType,
                MagickBooleanType],
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

AC_CACHE_CHECK(for ImageInfo.number_scenes, rm_cv_member_ImageInfo_number_scenes,
  AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdio.h>
#include "magick/api.h"],
    [ImageInfo i; i.number_scenes = 0;],
    rm_cv_member_ImageInfo_number_scenes=yes,
    rm_cv_member_ImageInfo_number_scenes=no))
if test "$rm_cv_member_ImageInfo_number_scenes" = yes; then
  AC_DEFINE(HAVE_IMAGEINFO_NUMBER_SCENES)
fi

AC_CACHE_CHECK(for NoCompliance enum value, rm_cv_enum_nocompliance,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ComplianceType x; x = NoCompliance],
     rm_cv_enum_nocompliance='yes',
     rm_cv_enum_nocompliance='no')])

if test "$rm_cv_enum_nocompliance" = yes; then
  AC_DEFINE(HAVE_NOCOMPLIANCE)
fi

AC_CACHE_CHECK(for HSLColorspace enum value, rm_cv_enum_hslcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = HSLColorspace],
     rm_cv_enum_hslcolorspace='yes',
     rm_cv_enum_hslcolorspace='no')])

if test "$rm_cv_enum_hslcolorspace" = yes; then
  AC_DEFINE(HAVE_HSLCOLORSPACE)
fi

AC_CACHE_CHECK(for HWBColorspace enum value, rm_cv_enum_hwbcolorspace,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [ColorspaceType x; x = HWBColorspace],
     rm_cv_enum_hwbcolorspace='yes',
     rm_cv_enum_hwbcolorspace='no')])

if test "$rm_cv_enum_hwbcolorspace" = yes; then
  AC_DEFINE(HAVE_HWBCOLORSPACE)
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

AC_CACHE_CHECK(for CopyCyanCompositeOp enum value, rm_cv_enum_copycyancompositeop,
  [AC_TRY_COMPILE(
[#include <stdio.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "magick/api.h"],
     [CompositeOperator x; x = CopyCyanCompositeOp],
     rm_cv_enum_copycyancompositeop='yes',
     rm_cv_enum_copycyancompositeop='no')])

if test "$rm_cv_enum_copycyancompositeop" = yes; then
  AC_DEFINE(HAVE_COPYCYANCOMPOSITEOP)
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
  AC_DEFINE(HAVE_ColorDodgeCompositeOp)
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

# Where is ****Magick installed?
MAGICKPREFIX=`${MAGICKCONFIG} --prefix`

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

AC_ARG_WITH(doc-dir,
    AC_HELP_STRING([--with-doc-dir=DIR], [RMagick documentation directory [[PREFIX/share/RMagick]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --doc-dir=$withval"], )

AC_ARG_WITH(std-ruby,
    AC_HELP_STRING([--with-std-ruby=DIR], [the directory for standard ruby libraries [[PREFIX/lib/ruby/1.x]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --std-ruby=$withval"], )

AC_ARG_WITH(site-ruby,
    AC_HELP_STRING([--with-site-ruby=DIR], [the directory for non-standard ruby libraries [[PREFIX/lib/ruby/site_ruby/1.x]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --site-ruby=$withval"], )

AC_ARG_WITH(site-ruby-common,
    AC_HELP_STRING([--with-site-ruby-common=DIR], [the directory for version-independent non-standard ruby libraries [[PREFIX/lib/ruby/site_ruby]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --site-ruby-common=$withval"], )

AC_ARG_WITH(rb-dir,
    AC_HELP_STRING([--with-rb-dir=DIR], [the directory for ruby scripts [[PREFIX/lib/ruby/site_ruby/1.x]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --rb-dir=$withval"], )

AC_ARG_WITH(so-dir,
    AC_HELP_STRING([--with-so-dir=DIR], [the directory for ruby extensions]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --so-dir=$withval"], )

AC_ARG_WITH(ruby-path,
    AC_HELP_STRING([--with-ruby-path=DIR], [path to set to #! line [PREFIX/bin/ruby]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --ruby-path=$withval"], )

AC_ARG_WITH(ruby-prog,
    AC_HELP_STRING([--with-ruby-prog=NAME], [the ruby program used for installation [[PREFIX/bin/ruby]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --ruby-prog=$withval"
     RUBY=$withval], )

AC_ARG_WITH(make-prog,
    AC_HELP_STRING([--with-make-prog=NAME], [the make program to compile ruby extensions [[make]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --make-prog=$withval"], )

AC_ARG_WITH(rbconfig,
    AC_HELP_STRING([--with-rbconfig=PATH], [your rbconfig to load [[ruby's]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --rbconfig=$withval"], )

AC_ARG_ENABLE(without-ext,
    AC_HELP_STRING([--enable-without-ext], [does not compile/install ruby extensions [[--disable-without-ext]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --without-ext"], )

AC_ARG_ENABLE(no-harm,
    AC_HELP_STRING([--enable-no-harm], [only display what to do if given [[--disable-no-harm]]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --no-harm"], )

AC_ARG_ENABLE(allow-example-errors,
    AC_HELP_STRING([--enable-allow-example-errors], [allow installation to proceed even if a lot of examples fail [--disable-allow-example-errors]]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --allow-example-errors"], )


RUBY_VERSION_STRING=`$RUBY --version`
AC_DEFINE_UNQUOTED(RUBY_VERSION_STRING, "$RUBY_VERSION_STRING")
AC_SUBST(RUBY)
AC_SUBST(RUBY_CONFIG_OPTS)

AC_CONFIG_FILES(Makefile ext/RMagick/extconf.rb metaconfig)
AC_OUTPUT

echo ""
echo ""
echo "This installation of $PACKAGE_STRING is configured for"
echo "$RUBY_VERSION_STRING and $MAGICKNAME `$MAGICKCONFIG --version`."


