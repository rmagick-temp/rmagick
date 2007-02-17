
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
    AC_MSG_ERROR([Can't install RMagick. Can't find Magick-config in PATH.])
fi

AC_MSG_CHECKING([ImageMagick version])
MAGICKVERS=`Magick-config --version`
AC_MSG_RESULT($MAGICKVERS)

MAGICKVERS=`echo ${MAGICKVERS} | tr -d .`
if test $MAGICKVERS -lt 626; then
    AC_MSG_ERROR([Can't install RMagick. You must have ImageMagick 6.2.6 or later.])
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
AC_CHECK_HEADERS([sys/types.h magick/MagickCore.h])

# Append ImageMagick's lists of libs to the user's list (if any)
LIBS="${LIBS} `Magick-config --libs`"


AC_CHECK_FUNCS([AdaptiveBlurImageChannel AdaptiveResizeImage AdaptiveSharpenImage])
AC_CHECK_FUNCS([InterpolatePixelColor IsImageSimilar LinearStretchImage])
AC_CHECK_FUNCS([OrderedPosterizeImageChannel])
AC_CHECK_FUNCS([PolaroidImage RecolorImage SketchImage])
AC_CHECK_FUNCS([TransposeImage TransverseImage UniqueImageColors])

# What typedefs?

AC_CHECK_MEMBERS([Image.transparent_color],,,
                [#include <stdio.h>
                 #if HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #include "magick/api.h"])


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


# The GetMagickPixelPacket
# function was introduced in 6.2.0. IM 6.3.0 changed the type of the 'color'
# member of the ColorInfo structure from PixelPacket to MagickPixelPacket.
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

# Where is ImageMagick installed?
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


