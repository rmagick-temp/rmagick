RMagick MAJOR.MINOR.TEENY README
================================
YY/MM/DD
--------

Table Of Contents
-----------------

* [Introduction] [intro]
* [Contact Information] [contact]
* [Prerequisites] [prereq]
* [Tips for installing and configuring ImageMagick and GraphicsMagick] [tips]
* [Installing RMagick] [install]
  + [Configuration options] [options]
  + [Running the configure and make scripts] [scripts]
* [Things that can go wrong] [uhoh]
* [Upgrading] [upgrade]
* [Uninstalling] [uninstall]
* [More samples] [samples]
* [Undocumented features] [undoc]
* [Known issues] [issues]
* [Reporting bugs] [bugs]
* [Credits] [credits]
* [License] [license]

<h2 id="intro">Introduction</h2>

RMagick is an interface between the Ruby programming language and the
ImageMagick and GraphicsMagick image processing libraries.

<h2 id="contact">Contact Information</h2>

__Author:__ Tim Hunter

__Email:__ <rmagick@rubyforge.org>

__RubyForge:__ <http://rubyforge.org/projects/rmagick/>

<h2 id="prereq">Prerequisites</h2>

__O/S:__ Linux, Sun Solaris, Cygwin, FreeBSD, OS X.

__Ruby__ 1.6.7 or later. You can get Ruby from <http://www.ruby-lang.org>.

Either __ImageMagick__ 6.0.0 or later, or any release of
__GraphicsMagick__.  GraphicsMagick is a friendly fork of ImageMagick
5.5.1.  You can get ImageMagick from <http://www.imagemagick.org>.  You
can get GraphicsMagick from <http://www.graphicsmagick.org>.  ImageMagick
and GraphicsMagick have slightly different capabilities.  Please consult
their web sites if you have questions.

<h2 id="tips">Tips for installing and configuring ImageMagick and GraphicsMagick</h2>

If you are installing RMagick by compiling the source code, I strongly
encourage you to install the latest version of ImageMagick or
GraphicsMagick _from source_.  If you have never installed ImageMagick or
GraphicsMagick before, I also strongly encourage you to read the
README.txt file as many times as is necessary for you to understand how to
configure it.  ImageMagick and GraphicsMagick are large and complicated
programs and can be difficult to configure.  Follow these tips to minimize
the amount of time you'll spend and your frustration level.

_Do not_ simply type `./configure` and expect the defaults to be correct
for you.  Since you are installing ImageMagick/GraphicsMagick to use with
Ruby, consider whether you want to skip ImageMagick's/GraphicMagick's
support for Perl and C++ by using the `--without-perl` and
`--without-magick-plus-plus` options.  Doing so will speed up the
installation process and save some disk space.  You will almost certainly
want to specify the `--enable-shared` and `--disable-static` options.

Determine which image formats you are interested in using and make sure
that you have installed the libraries that ImageMagick/GraphicsMagick uses
to process these formats.  ImageMagick and GraphicsMagick use additional
libraries to support some image formats.  If you do not install those
libraries you cannot read and write those image formats.  You will need to
configure ImageMagick/GraphicsMagick to support the JPG, PNG, TIFF, and
WMF formats in order to execute all the RMagick sample programs.  See
ImageMagick's or GraphicMagick's README.txt file for more information.

Once you have determined the configuration options you need, run the
configure script.  When it completes, read the summary output to see if
configuration worked the way you expected.  Here's an example of the
summary output from ImageMagick's configure script.  Notice that the
result of each option is listed in the "Configured value" column.


                      Option                        Value
    -------------------------------------------------------------------------
    Shared libraries  --enable-shared=yes           yes
    Static libraries  --enable-static=no            no
    Module support    --with-modules=yes            yes
    GNU ld            --with-gnu-ld=yes             yes
    LZW support       --enable-lzw=yes              yes
    Quantum depth     --with-quantum-depth=8        8

    Delegate Configuration:
    BZLIB             --with-bzlib=yes              yes
    DPS               --with-dps=yes                yes
    FlashPIX          --with-fpx=yes                no
    FreeType 2.0      --with-ttf=yes                yes
    Ghostscript       None                          gs (7.07.2)
    Ghostscript fonts --with-gs-font-dir=default    /usr/share/fonts/default/Type1/
    Ghostscript lib   --with-gslib=no               no
    Graphviz          --with-dot=yes                no
    JBIG              --with-jbig=yes               no
    JPEG v1           --with-jpeg=yes               yes
    JPEG-2000         --with-jp2=yes                no
    LCMS              --with-lcms=yes               yes
    Magick++          --with-magick-plus-plus=no    no
    PERL              --with-perl=no                no
    PNG               --with-png=yes                yes
    TIFF              --with-tiff=yes               yes
    Windows fonts     --with-windows-font-dir=/mnt/windows/windows/fonts    /mnt/windows/windows/fonts/
    WMF               --with-wmf=yes                yes
    X11               --with-x=                     yes
    XML               --with-xml=yes                yes
    ZLIB              --with-zlib=yes               yes

    X11 Configuration:
      X_CFLAGS     = -I/usr/X11R6/include
      X_PRE_LIBS   = -lSM -lICE
      X_LIBS       = -L/usr/X11R6/lib
      X_EXTRA_LIBS =

If the results are not what you wanted, install any missing libraries,
choose new or different options, or whatever it takes, erase the
config.cache file, and re-run `configure`.  Repeat as often as necessary
before moving to the `make` and `make install` steps.

Detailed information about all of ImageMagick's and GraphicsMagick's
configuration options may be found in their README.txt and INSTALL.txt
files.

#### Windows Metafile Format
As noted in the ImageMagick and GraphicsMagick README.txt files, to
support images in the Windows Metafile format, ImageMagick/GraphicsMagick
requires an additional library.  Without this library some of the RMagick
sample programs will not work.  ImageMagick and GraphicsMagick require
libwmf 0.2.5, 0.2.7, or 0.2.2 to support the WMF format.  (Avoid libwmf
0.2.6!)

<h2 id="install">Installing RMagick</h2>

Installing RMagick is much simpler than installing ImageMagick or
GraphicsMagick.  Note that the make step runs all the example programs.
This process both builds the example images used in the documentation and
validates your RMagick installation.  This step can take 5-15 minutes
depending on the speed of your computer.

<h4 id="options">Configuration Options</h4>

Type `./configure --help` to see a list of configuration options.  In
addition to the regular options, there are a few RMagick-specific options:

* --with-doc-dir=_directory_
    >  Specify the directory to install the RMagick documentation.
    >  By default this is $prefix/share/RMagick, where $prefix is the
    >  prefix specified by --prefix. For example, to install the
    >  documentation in /home/user/RMagick, specify:

    >  `./configure --with-doc-dir=/home/user/RMagick`

* --enable-allow-example-errors
    >  Normally the documentation installation terminates if 5 examples fail.
    >  If you use this option, the installation does not check for failing
    >  examples and will always complete. This option is useful if you're having
    >  trouble installing RMagick and you want to see all the failing examples.

* --disable-htmldoc (or --enable-htmldoc=no)
    >  By default the install process runs all the RMagick example programs
    >  and generates HTML versions of all the examples. This options causes
    >  the install process to skip this step. No documentation is installed.

* --with-graphics-magick
    >  If you have both ImageMagick and GraphicsMagick installed, this option will
    >  force RMagick to be configured with GraphicsMagick.

* --with-so-dir=_directory_
    >  The directory for ruby extensions.

* --with-ruby-path=_directory_
    >  The path to set the !# line in Ruby scripts. The default is $prefix/bin/ruby.

* --with-ruby-prog=_name_
    >  The name of the Ruby executable. The default is `ruby`.

* --with-make-prog=_name_
    >  The name of the `make` program.

* --with-rbconfig=_directory_
    > The directory of the rbconfig.rb file to use.  The default is Ruby's
    > rbconfig.

<h4 id="scripts">Running the <code>configure</code> and <code>make</code> scripts</h4>

De-compress the RMagick-MAJOR.MINOR.TEENY.tar.gz archive and enter the top
directory.  Then type:

    $ ./configure <configuration options>
    $ make
    ($ su)
    $ make install
    (optionally)
    $ make clean

<h2 id="uhoh">Things that can go wrong</h2>

#### Can't install RMagick. Can't find libMagick or one of the dependent libraries. Check the config.log file for more detailed information.
The message can also refer to "libGraphicsMagick". Typically this message means that one or more of the libraries that Imagemagick/GraphicsMagick depends on hasn't been installed. Examine the config.log file in the installation directory for any error messages. These messages typically contain enough additional information for you to be able to diagnose the problem.

#### Cannot open shared object file
When make is running the examples, if you get a message like this:

    /home/you/RMagick-MAJOR.MINOR.TEENY/lib/RMagick.rb:11:in `require': libMagick.so.0:
      cannot open shared object file: No such file or directory -
      /home/you/RMagick-MAJOR.MINOR.TEENY/ext/RMagick/RMagick.so (LoadError)

you probably do not have the directory in which the ImageMagick or GraphicsMagick
library is installed in your load path. An easy way to fix this is to define the
directory in the LD\_LIBRARY\_PATH environment variable. For example, suppose you
installed the GraphicsMagick library libGraphicsMagick.so in /usr/local/lib.
(By default this is where it is installed.) Create the LD\_LIBRARY\_PATH variable
like this:

    export LD_LIBRARY_PATH=/usr/local/lib

On Linux, see `ld(1)` and `ld.so(8)` for more information. On other operating
systems, see the documentation for the dynamic loading facility.

#### No such file or directory - "/tmp/rmagick6872.6"
When make is running the examples, if you get a message like this:

    hook /home/me/src/RMagick-MAJOR.MINOR.TEENY/./post-setup.rb failed:
    No such file or directory - "/tmp/rmagick6872.6"

you probably do not have a temporary directory environment variable set. Set
the TMPDIR environment variable to your temporary directory. For example:

    export TMPDIR=/home/me/tmp

<h2 id="upgrade">Upgrading</h2>

If you upgrade to a newer release of ImageMagick or GraphicsMagick, make sure
you're using a release of RMagick that supports that release. Usually I put out
a new release of RMagick with every new release of ImageMagick. It's safe to
install a newer release of RMagick over an earlier release.

<h2 id="uninstall">Uninstalling</h2>

The `uninstall` target will uninstall RMagick completely:

    make uninstall

<h2 id="samples">More samples</h2>

You can find more sample RMagick programs in the /example directory.
These programs are not installed in the RMagick documentation tree.

<h2 id="undoc">Undocumented features</h2>
This release includes an extra feature that isn't in the documentation.
The Magick module defines two methods that control ImageMagick's logging
function.  This function is not officially documented by ImageMagick, so I
have decided not to add them to RMagick's documentation.  However, you may
find them helpful for debugging your application.  (Caveat: these two
methods may change behavior or be removed without advance notice!  You are
on your own!)

##### Magick::set\_log\_event_mask(event [,...])

The arguments are one or more "event domains".  The set_log_event_mask
method recognizes these event domains: "all", "annotate", "blob", "cache",
"coder", "configure", "deprecate", "draw", "locale", "none", "resource",
"transform", "user", and "x11".  ImageMagick events that match the mask
are logged.  The default domain is "none".  For example,

        Magick::set\_log\_event_mask("render")

Logging is controlled by the log.mgk file, which may be found in the same
directory as the delegates.mgk files.  (See ImageMagick's README.txt
file.) The purpose of the entries in this file is undocumented, so your
guess is as good as mine.  Also, the meaning of the event domains and
exactly what events are logged are undocumented.

##### Magick::set\_log\_format(format)

The default log format is described in the log.mgk file.  This method
allows you to redefine the format at run-time.  The format argument is a
string similar to an fprintf format string.  Each line in the log has the
format described by the format string.  Characters that are not control
characters are printed as-is.  The control characters are:

        %t - the current time
        %r - the elapsed time
        %u - the user time
        %p - the pid (process id)
        %m - the name of the ImageMagick source file that contains the
             function that generated the event
        %f - the name of the ImageMagick function that generated the event
        %l - the line number in the source file
        %d - the event domain (one of the event mask strings listed above)
        %e - the event name

For example, the default log format is:

        Magick::set_log_format("%t %r %u %p %m/%f/%l/%d:\n  %e")

<h2 id="issues">Known issues</h2>

1. gcc 3.2, during the "install.rb setup" step, issues the following warning.
   This does not indicate a problem.

    `cc1: warning: changing search order for system directory "/usr/local/include"`   <br>
    `cc1: warning:   as it has already been specified as a non-system directory`

2. With older releases of ImageMagick you may see this warning. It does not
   indicate a problem.

    `/usr/include/sys/ipc.h:25:3: warning: #warning "Files using this header must
    be compiled with _SVID_SOURCE or _XOPEN_SOURCE"`

<h2 id="bugs">Reporting bugs</h2>

Please report bugs in RMagick, its documentation, or its installation
programs to me via the bug tracker on the [RMagick project page at
RubyForge](http://rubyforge.org/projects/rmagick).  However, I can't help
with Ruby installation and configuration or ImageMagick or GraphicsMagick
installation and configuration.  Please report problems with that software
to their respective authors or distributors.

<h2 id="credits">Credits</h2>

Thanks to

   * Akinori MUSHA, for his work making RMagick available on FreeBSD.
   * Tom Payne, for reporting bugs, sending me the Ruby 1.8.0 patches, and for the Gentoo ebuild.
   * Bob Friesenhahn, for GraphicsMagick. Also for his help with the RMagick installation and many other things.
   * Simple Systems, for hosting the RMagick documentation online.
   * Mike Williams, for the RMagick Debian package
   * ImageMagick Studio LLC, for ImageMagick and for hosting the RMagick documentation.
   * Kaspar Schiess, for the MS Windows gem.
   * Jeremy Hinegardner, for the amd64 patch.

<h2 id="license">License</h2>

   >  Copyright © 2006 by Timothy P. Hunter
   >
   >  Permission is hereby granted, free of charge, to any person obtaining a
   >  copy of this software and associated documentation files (the "Software"),
   >  to deal in the Software without restriction, including without limitation
   >  the rights to use, copy, modify, merge, publish, distribute, sublicense,
   >  and/or sell copies of the Software, and to permit persons to whom the
   >  Software is furnished to do so, subject to the following conditions:
   >
   >  The above copyright notice and this permission notice shall be included in
   >  all copies or substantial portions of the Software.

   >  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   >  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   >  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   >  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   >  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   >  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   >  DEALINGS IN THE SOFTWARE.


<div align="center">
<a href="http://www.opensource.org/docs/definition.php">
<img src="http://opensource.org/trademarks/osi-certified/web/osi-certified-90x75.gif"
border="0" width="90" height="75">
</a>
</div>

-------------------------------------------------------------------------------

<em>
This file is marked up using [Markdown](http://daringfireball.net/projects/markdown).
The HTML version was produced with [BlueCloth](http://bluecloth.rubyforge.org).
</em>

[intro]: #intro
[contact]: #contact
[prereq]: #prereq
[tips]: #tips
[install]: #install
[options]: #options
[scripts]: #scripts
[uhoh]: #uhoh
[upgrade]: #upgrade
[uninstall]: #uninstall
[samples]: #samples
[undoc]: #undoc
[issues]: #issues
[bugs]: #bugs
[credits]: #credits
[license]: #license

