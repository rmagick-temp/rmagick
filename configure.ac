
# RMagick configure.ac

AC_INIT(RMagick, x.y.z, cyclists@nc.rr.com, RMagick-x.y.z.tar.gz)
echo "Configuring ${PACKAGE_STRING}"

AC_CONFIG_HEADER([ext/RMagick/rmagick_config.h])

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

AC_CHECK_LIB($MAGICKLIB, InitializeMagick, [],
    [AC_MSG_ERROR([Can't install RMagick. Can't find lib${MAGICKLIB}])],
    [`${MAGICKCONFIG} --libs`])

AC_SUBST(MAGICKLIB)
AC_SUBST(MAGICKNAME)
AC_SUBST(MAGICKCONFIG)
AC_DEFINE_UNQUOTED(MAGICKNAME, $MAGICKNAME)

AC_CHECK_HEADER(magick/api.h, [],
    [AC_MSG_ERROR([Can't find magick/api.h])])

# What functions?
AC_CHECK_FUNCS([AdaptiveThresholdImage AppendImageToList LevelImageChannel ThumbnailImage RemoveFirstImageFromList GetNextImageInList SetLogFormat XImportImage ExportImagePixels ImportImagePixels ThresholdImageChannel RandomChannelThresholdImage StripImage AcquireMagickMemory])

# What typedefs?
AC_CHECK_TYPES([DisposeType,
                ExtendedSignedIntegralType,
                ExtendedUnsignedIntegralType],
                [], [],
                [#include <stdio.h>
                 #if HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #include "magick/api.h"])

# Don't use AC_CHECK_MEMBERS!
AC_CACHE_CHECK(if Image has extract_info, rm_cv_member_Image_extract_info,
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

AC_CACHE_CHECK(if ImageInfo has number_scenes, rm_cv_member_ImageInfo_number_scenes,
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

# Determine documentation base URI

# Where is ****Magick installed?
MAGICKPREFIX=`${MAGICKCONFIG} --prefix`

AC_CACHE_CHECK(for documentation base URI, rm_cv_path_doc_base_uri,
  [rm_cv_path_doc_base_uri=''
  # Look for ImageMagick.html.
  for docroot in $MAGICKPREFIX/share/$MAGICKNAME*
    do
      if test -f "$docroot/$MAGICKNAME.html"; then
        rm_cv_path_doc_base_uri=file://${docroot}
        break 1
      fi
    done
    # In IM 5.5.7 and later, and GM 1.0, there is no ImageMagick.html file.
    # Look for www directory.
    if test "${rm_cv_path_doc_base_uri}x" = 'x'; then
      for docroot in $MAGICKPREFIX/share/$MAGICKNAME*
        do
        if test -d "$docroot/www"; then
          rm_cv_path_doc_base_uri="file://${docroot}"
          break 1
        fi
        done
    fi
    # Give up and use the default.
    if test "${rm_cv_path_doc_base_uri}x" = 'x'; then
      case "$MAGICKNAME" in
        ImageMagick )
          rm_cv_path_doc_base_uri=http://www.imagemagick.org
          ;;
        GraphicsMagick )
          rm_cv_path_doc_base_uri=http://www.graphicsmagick.org
          ;;
      esac
    fi
  ])

MAGICKBASEURI="$rm_cv_path_doc_base_uri"
AC_SUBST(MAGICKBASEURI)

# Options to pass to ruby config step

RUBY_CONFIG_OPTS=''

if test "x$prefix" != xNONE; then
  RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --prefix=$prefix"
fi
if test `eval echo $bindir` != NONE/bin; then
  RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --bindir=`eval echo $bindir`"
fi
if test `eval echo $datadir` != ${prefix}/share; then
  RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --datadir=`eval echo $datadir`"
fi

AC_ARG_WITH(imdoc-base-uri,
    AC_HELP_STRING([--with-imdoc-base-uri=URI], [ImageMagick/GraphicsMagick documentation URI]),
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --imdoc-base-uri=$withval"],
    [RUBY_CONFIG_OPTS="$RUBY_CONFIG_OPTS --imdoc-base-uri=$rm_cv_path_doc_base_uri"])

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

AC_SUBST(RUBY)
AC_SUBST(RUBY_CONFIG_OPTS)

AC_OUTPUT(Makefile ext/RMagick/extconf.rb metaconfig)

