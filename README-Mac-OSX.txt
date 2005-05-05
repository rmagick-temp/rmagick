HOWTO Install RMagick on Mac OS X

This HOWTO describes a method for installing RMagick, ImageMagick or
GraphicsMagick, and the delegate libraries used by ImageMagick and
GraphicsMagick.  You only need to install one of ImageMagick or GraphicsMagick.
The procedure is the same for either library.  Throughout the remainder of this
document I will use the word "xMagick" to refer to either of these two
libraries.  I developed this procedure using a Powerbook G4 and Mac OS X 10.3.8.
If you are using a different version of Mac OS X some of the details may be
different.  In particular these instructions assume you are using bash as your
shell.

You will need to have your Mac OS X installation disks, a connection to
the Internet, and at least an hour of free time (assuming you have a
broadband connection).

Step 1. Install X11, the Xcode Tools, and the X11SDK.

xMagick needs a X server to display images, so you'll need to install X11.  Some
versions of Mac OS X come with X11 on the installation disk.  Mine did not.  If
your install disks do not include this package, you can download it from
http://www.apple.com/macosx/features/x11/download/.

After installing X11, install the Xcode Tools and the X11SDK.  These packages
can be found on your OS X installation disk.  The X11SDK is not installed by
default.  When you get to the Installer step labeled "Installation Type" click
the "Custom" button.  You will be presented with a list of packages to install.
Check "X11SDK".

Step 2. Install DarwinPorts

Go to http://darwinports.opendarwin.org/getdp/ and follow the instructions to
download and install DarwinPorts.  (The remainder of this document assumes that
you take all the defaults during the installation.)

Step 3: Install the delegate libraries

xMagick uses a large number of delegate libraries.  (See the README.txt file in
the xMagick distribution for a complete list.) Here we'll use DarwinPorts to
install delegates for popular image formats and that are needed to run the
RMagick example programs.  Enter the following port commands:

sudo port install jpeg
sudo port install libpng
sudo port install libwmf
sudo port install tiff
sudo port install lcms
sudo port install freetype
sudo port install ghostscript

Note that some of these libraries have prerequisites which will be automatically
installed.

Step 4: Install ImageMagick or GraphicsMagick

Go to http://www.imagemagick.org or http://www.graphicsmagick.org and download
the latest version of the software to a temporary directory.  Unroll the tarball
and make the new directory current.

Before proceeding, you need to make sure you're using the correct version of the
freetype library.  The X11 files you installed in Step 1 include a version of
the freetype library, and of course you just installed another version using
DarwinPorts.  You need to use the DarwinPorts version when you're building
xMagick.  To make sure you have the right version, enter the command:

freetype-config --cflags

You should see this output:

-I/opt/local/include/freetype2 -I/opt/local/include

If you see the following output instead,

-I/usr/X11R6/include -I/usr/X11R6/include/freetype2

you should edit your $PATH to make sure that /opt/local/bin preceeds
/usr/X11R6/bin.  Do not try to install xMagick until you get the correct output
from the freetype-config program.

To configure xMagick, enter these commands:

export CPPFLAGS=-I/opt/local/include
export LDFLAGS=-L/opt/local/lib
./configure --prefix=/opt/local --disable-static --with-modules --without-perl --without-magick-plus-plus --with-quantum-depth=8 --with-gs-font-dir=/opt/local/share/ghostscript/fonts

The ./configure command should be entered on a single line.  The
--prefix=/opt/local option will cause xMagick to be installed in the same
directory as the libraries we installed with DarwinPorts.  If you want to
install xMagick somewhere else, specify a different directory.  If you do not
specify the --prefix option xMagick will be installed in /usr/local.  The
--disable-static and --with-modules options cause xMagick to be built with
dynamically loaded modules.  Since you're installing xMagick for use with Ruby,
I've included the --without-perl and --without-magick-plus-plus options to
suppress the Perl and C++ support.  The --with-quantum-depth=8 option configures
xMagick to use a bit depth of 8.  If you need to build with a different bit
depth (and if you need to you'll already know it) you can specify 16 or 32.
Finally, the --with-gs-font-dir option tells xMagick where the Ghostscript fonts
are installed.

For more information about all these options see xMagick's README.txt file.

./configure will produce quite a bit of output.  The last page is the most
interesting.  If you've successfully performed all the steps so far and used all
the deafults, the output from configure should end with a page like this:

-------------------------------------------------------------------------------

ImageMagick is configured as follows. Please verify that this configuration matches your expectations.

Host system type : powerpc-apple-darwin7.8.0

Option Value
-------------------------------------------------------------------------
Shared libraries --enable-shared=yes yes
Static libraries --enable-static=no no
Module support --with-modules=yes yes
GNU ld --with-gnu-ld=no no
Quantum depth --with-quantum-depth=8 8

Delegate Configuration:
BZLIB       --with-bzlib=yes           yes
DPS         --with-dps=yes             yes
FlashPIX    --with-fpx=no              no
FreeType 2.0 --with-ttf=yes            yes
Ghostscript None                       gs (8.14)
Ghostscript fonts --with-gs-font-dir=/opt/local/share/ghostscript/fonts /opt/local/share/ghostscript/fonts/
Ghostscript lib   --with-gslib=no      no
Graphviz    --with-dot=yes             no
JBIG        --with-jbig=yes            no
JPEG v1     --with-jpeg=yes            yes
JPEG-2000   --with-jp2=yes             no
LCMS        --with-lcms=yes            yes
Magick++    --with-magick-plus-plus=no no
PERL        --with-perl=no             no
PNG         --with-png=yes             yes
TIFF        --with-tiff=yes            yes
Windows fonts --with-windows-font-dir=none
WMF         --with-wmf=yes             yes
X11         --with-x=                  yes
XML         --with-xml=yes             yes
ZLIB        --with-zlib=yes            yes

X11 Configuration:
X_CFLAGS = -I/usr/X11R6/include

X_PRE_LIBS = -lSM -lICE
X_LIBS = -L/usr/X11R6/lib
X_EXTRA_LIBS =

Options used to compile and link:
PREFIX = /opt/local
EXEC-PREFIX = /opt/local
VERSION = X.Y.Z
CC = gcc
CFLAGS = -g -O2 -Wall
CPPFLAGS = -I/opt/local/include
PCFLAGS =
DEFS = -DHAVE_CONFIG_H
LDFLAGS = -L/opt/local/lib -L/opt/local/lib -L/usr/X11R6/lib -L/opt/local/lib -lfreetype -lz -L/usr/lib
LIBS = -lMagick -llcms -ltiff -lfreetype -ljpeg -lXext -lSM -lICE -lX11 -lXt -lbz2 -lz -lpthread -lm -lpthread
CXX = g++
CXXFLAGS =

-------------------------------------------------------------------------------

Of course, instead of VERSION X.Y.Z you will see the version number of the
version of xMagick that you downloaded.  Check your output to make sure that
xMagick located all the delegate libraries.  You should see "yes" in the Value
column for bzlib, FreeType 2.0, JPEG v1, LCMS, PNG, TIFF, WMF, X11, XML, and
ZLIB.

If you get this output from ./configure you're ready to proceed.  If you are
missing some delegates you should resolve those issues before continuing.
Re-run ./configure, being very careful to enter the commands correctly.

Once you're satisfied that you've configured xMagick the way you want it, enter
these two commands:

make
sudo make install

Step 5: Installing RMagick

The hard part is done.  All we have to do now is install RMagick.  If you
haven't already done so, download the RMagick tarball from Rubyforge and unroll
it into a temporary directory.  Make that directory current.  Enter these
commands

./configure
make
sudo make install

The make step will take a few minutes to run since it builds all of the RMagick
examples. That's it. You should have a complete install of xMagick and RMagick.
