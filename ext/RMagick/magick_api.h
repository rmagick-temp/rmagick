/* Called both during configuration and from rmagick.h to     */
/* include the appropriate ImageMagick API header file.       */
/* It is highly unlikely that neither symbol will be defined. */
#if HAVE_MAGICK_CORE_H
#include "magick/MagickCore.h"
#elif HAVE_MAGICK_API_H
#include "magick/api.h"
#else
#error Magick API header not found!
#endif
