
  RMagick MAJOR.MINOR.TEENY


    README


        YY/MM/DD


    Table of Contents

    * Introduction <#introduction>
    * Contact information <#contact_information>
    * Prerequisites <#prerequisites>
    * Tips for installing and configuring ImageMagick and GraphicsMagick
      <#tips>
    * Installing RMagick <#installing_rmagick>
    *
          o Configuration options <#configuration_options>
          o Running the configure and make scripts <#running>
    * Things that can go wrong <#things_that_can_go_wrong>
    * Upgrading <#upgrading>
    * More samples <#more_samples>
    * Undocumented features <#undocumented_features>
    * Known issues <#known_issues>
    * Reporting bugs <#reporting_bugs>
    * Credits <#credits>
    * License <#license>


    Introduction

RMagick is an interface between the Ruby programming language and the
ImageMagick and GraphicsMagick image processing libraries.


    Contact Information

*Author:* Tim Hunter
*Email:* cyclists@nc.rr.com
<mailto:cyclists@nc.rr.com?subject=RMagick%20Feedback>


    Prerequisites

You must be running either Linux, Solaris, Cygwin, or FreeBSD. This
version of RMagick does not run on MS Windows except with Cygwin.

You must have Version 1.6.7 or later of Ruby. If you do not have Ruby,
you can get it from www.ruby-lang.org <http://www.ruby-lang.org>. You
must have installed ImageMagick 5.5.1 or later, or GraphicsMagick 1.0 or
later. If you have not already installed ImageMagick or GraphicsMagick,
you can get ImageMagick from www.imagemagick.org
<http://www.imagemagick.org>, and you can get GraphicsMagick from
www.graphicsmagick.org <http://www.graphicsmagick.org>. For more
information about installing ImageMagick and GraphicsMagick, read the
tips below.

You only need one of ImageMagick or GraphicsMagick. RMagick works
equally well with both.


    Tips for installing and configuring ImageMagick and GraphicsMagick

If you have never installed ImageMagick or GraphicsMagick before, I
strongly encourage you to install the newest version of your choice from
source. I also strongly encourage you to read the README.txt file as
many times as is necessary for you to understand how to configure it.
ImageMagick and GraphicsMagick are large and complicated programs and
can be difficult to configure. Follow these tips to minimize the amount
of time you'll spend and your frustration level.

Do /not/ simply type ./configure and expect the defaults to be correct
for you. Through version 5.5.7, by default, ImageMagick is configured to
use a color depth ("quantum-depth") of 16. Unless you have very special
image processing needs you should use the --with-quantum-depth=8 option.
This will cause ImageMagick run faster and use half as much memory. If
you need to use 16 as the color depth, you'll know it.

GraphicsMagick defaults to a color depth of 8, so you need do nothing.

Since you are installing ImageMagick/GraphicsMagick to use with Ruby,
consider whether you want to skip ImageMagick's/GraphicMagicks' support
for Perl and C++ by using the --without-perl and
--without-magick-plus-plus options. Doing so will speed up the
installation process and save some disk space.

Determine which image formats you are interested in using and make sure
that you have installed the libraries that ImageMagick/GraphicsMagick
uses to process these formats. ImageMagick and GraphicsMagick use
additional libraries to support some image formats. If you do not
install those libraries you cannot read and write those image formats.
You will need to configure ImageMagick/GraphicsMagick to support the
JPG, PNG, TIFF, and WMF formats in order to execute all the RMagick
sample programs. See ImageMagick's or GraphicMagick's README.txt file
for more information.

You will almost certainly want to specify the --enable-shared and
--disable-static options.

Once you have determined the configuration options you need, run the
configure script. When it completes, read the summary output to see if
configuration worked the way you expected. Here's an example of the
summary output from ImageMagick's configure script. Notice that the
result of each option is listed in the "Configured value" column.

Option            Configure option              Configured value
-----------------------------------------------------------------
Shared libraries  --enable-shared=yes           yes
Static libraries  --enable-static=no            no
GNU ld            --with-gnu-ld=yes             yes
LZW support       --enable-lzw=no               no
Quantum depth     --with-quantum-depth=8        8
Delegate Configuration:
BZLIB             --with-bzlib=yes              yes
DPS               --with-dps=yes                no (failed tests)
FlashPIX          --with-fpx=yes                no
FreeType 2.0      --with-ttf=yes                yes
Ghostscript       None                          /usr/bin/gs
Ghostscript fonts --with-gs-font-dir=default    /usr/share/fonts/default/Type1/
Ghostscript lib   --with-gslib=no               no
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
config.cache file, and re-run configure. Repeat as often as necessary
before moving to the `make' and `make install' steps.

Detailed information about all of ImageMagick's and GraphicsMagick's
configuration options may be found in their README.txt and INSTALL.txt
files.


        Windows Metafile Format

As noted in the ImageMagick and GraphicsMagick README.txt files, to
support images in the Windows Metafile format,
ImageMagick/GraphicsMagick requires an additional library. Without this
library some of the RMagick sample programs will not work. ImageMagick
and GraphicsMagick require libwmf 0.2.5, 0.2.7, or 0.2.2 to support the
WMF format. (Avoid libwmf 0.2.6!)


    Installing RMagick

Installing RMagick is much simpler than installing ImageMagick or
GraphicsMagick. Note that the /make/ step runs all the example programs.
This process both builds the example images used in the documentation
and validates your RMagick installation. This step can take 5-15 minutes
depending on the speed of your computer.


        Configuration options

Type |./configure --help| to see a list of configuration options. In
addition to the regular options, there are a few RMagick-specific options:

--with-imdoc-base-uri=uri

    During installation, all links in the RMagick documentation that
    link to the ImageMagick or GraphicsMagick web site are replaced with
    links to your local copies of the documentation. If you want to
    specify a different location, use the --with-imdoc-base-uri option
    during the config step to specify a different base URI for the
    links. By default, the base URI is
    "file:///$prefix/share/****Magick" (where $prefix is the prefix you
    used when you installed ImageMagick or GraphicsMagick). If the
    configure script doesn't find the documentation in the default
    directory it will use "http://www.imagemagick.org" or
    "http://www.graphicsmagick.org". For example,

./configure --with-imdoc-base-uri=file:///home/user/ImageMagick

--with-doc-dir=directory
    Specify the directory to install the RMagick documentation. By
    default this is $prefix/share/RMagick, where $prefix is the prefix
    specified by --prefix. For example, to install the documentation in
    /home/user/RMagick, specify:

./configure --with-doc-dir=/home/user/RMagick

--enable-allow-example-errors
    Normally the documentation installation terminates if 5 examples
    fail. If you use this option, the installation does not check for
    failing examples and will always complete. This option is useful if
    you're having trouble installing RMagick and you want to see all the
    failing examples.


        Running the |configure| and |make| scripts

De-compress the RMagick-MAJOR.MINOR.TEENY.tar.gz archive and enter the
top directory. Then type:

    $ ./configure <configuration options>
    $ make
   ($ su)
    $ make install
   (optionally)
    $ make clean


    Things that can go wrong

*Cannot open shared object file*

    When |make| is running the examples, if you get a message like this:

/home/you/RMagick-MAJOR.MINOR.TEENY/lib/RMagick.rb:11:in `require': libMagick.so.0:
  cannot open shared object file: No such file or directory -
  /home/you/RMagick-MAJOR.MINOR.TEENY/ext/RMagick/RMagick.so (LoadError)

    you probably do not have the directory in which the ImageMagick or
    GraphicsMagick library is installed in your load path. An easy way
    to fix this is to define the directory in the LD_LIBRARY_PATH
    environment variable. For example, suppose you installed the
    GraphicsMagick library libGraphicsMagick.so in /usr/local/lib. (By
    default this is where it is installed.) Create the LD_LIBRARY_PATH
    variable like this:

export LD_LIBRARY_PATH=/usr/local/lib

    On Linux, see |ld(1)| and |ld.so(8)| for more information. On other
    operating systems, see the documentation for the dynamic loading
    facility.

*No such file or directory - "/tmp/rmagick6872.6"*

    When |make| is running the examples, if you get a message like this:

hook /home/me/src/RMagick-MAJOR.MINOR.TEENY/./post-setup.rb failed:
No such file or directory - "/tmp/rmagick6872.6"

    you probably do not have a temporary directory environment variable
    set. Set the TMPDIR environment variable to your temporary
    directory. For example:

export TMPDIR=/home/me/tmp


    Upgrading

If you upgrade to a newer release of ImageMagick or GraphicsMagick, make
sure you're using a release of RMagick that supports that release.
Usually I put out a new release of RMagick with every new release of
ImageMagick. It's safe to install a newer release of RMagick over an
earlier release.


    More samples

You can find more sample RMagick programs in the /example directory.
These programs are not installed in the RMagick documentation tree.


    Undocumented features

This release includes an extra feature that isn't in the documentation.
The Magick module defines two methods that control ImageMagick's logging
function. This function is not officially documented by ImageMagick, so
I have decided not to add them to RMagick's documentation. However, you
may find them helpful for debugging your application. (Caveat: these two
methods may change behavior or be removed without advance notice! You
are on your own!)

/Magick::set_log_event_mask(event [,...]) -> Magick/

The arguments are one or more "event domains". The set_log_event_mask
method recognizes these event domains: "all", "annotate", "blob",
"cache", "coder", "configure", "deprecate", "draw", "locale", "none",
"resource", "transform", "user", and "x11". ImageMagick events that
match the mask are logged. The default domain is "none". For example,

          Magick::set_log_event_mask("render")
       

Logging is controlled by the log.mgk file, which may be found in the
same directory as the delegates.mgk files. (See ImageMagick's README.txt
file.) The purpose of the entries in this file is undocumented, so your
guess is as good as mine. Also, the meaning of the event domains and
exactly what events are logged are undocumented.

/Magick::set_log_format(format) -> Magick/

The default log format is described in the log.mgk file. This method
allows you to redefine the format at run-time. The format argument is a
string similar to an fprintf format string. Each line in the log has the
format described by the format string. Characters that are not control
characters are printed as-is. The control characters are:

         %t - the current time
         %r - the elapsed time
         %u - the user time
         %p - the pid (process id)
         %m - the name of the ImageMagick source file that contains the
              function that generated the event
         %f - the name of the ImageMagick function that generated the
              event
         %l - the line number in the source file
         %d - the event domain (one of the event mask strings listed above)
         %e - the event name

For example, the default log format is:

         Magick::set_log_format("%t %r %u %p %m/%f/%l/%d:\n  %e")


    Known issues

   1. gcc 3.2, during the "install.rb setup" step, issues the warning:

cc1: warning: changing search order for system directory "/usr/local/include"
cc1: warning:   as it has already been specified as a non-system directory

      This does not indicate a problem.


    Reporting bugs

Please report bugs in RMagick, its documentation, or its installation
programs to me at cyclists@nc.rr.com
<mailto:cyclists@nc.rr.com?subject=RMagick%20bug%20report>. However, I
cannot help with Ruby installation and configuration or ImageMagick or
GraphicsMagick installation and configuration. Please report problems
with that software to their respective authors or distributors.


    Credits

Thanks to

    * Akinori MUSHA, for his work making RMagick available on FreeBSD
    * Tom Payne, for reporting bugs, sending me the Ruby 1.8.0 patches,
      and for the Gentoo ebuild
    * Bob Friesenhahn, for helping with the GraphicsMagick support and
      in many other ways
    * Simple Systems, for hosting the RMagick documentation online.
    * Mike Williams, for the RMagick Debian package


    License

RMagick is distributed under the "Artistic License" included below.
Please note that this license is not a license for the Ruby software,
the GraphicsMagick software, or the ImageMagick software, which are
distributed under separate licenses. If you have questions about the
RMagick license please contact the author at cyclists@nc.rr.com
<mailto:cyclists@nc.rr.com?subject=RMagick%20license%20question>. If you
have questions about the Ruby license, the ImageMagick license, or the
GraphicsMagick license, please refer to the legal information included
with that software.


        Preamble

    The intent of this document is to state the conditions under which a
    Package may be copied, such that the Copyright Holder maintains some
    semblance of artistic control over the development of the package,
    while giving the users of the package the right to use and
    distribute the Package in a more-or-less customary fashion, plus the
    right to make reasonable modifications.


        Definitions

        * "Package" refers to the collection of files distributed by the
          Copyright Holder, and derivatives of that collection of files
          created through textual modification.
        * "Standard Version" refers to such a Package if it has not been
          modified, or has been modified in accordance with the wishes
          of the Copyright Holder as specified below.
        * "Copyright Holder" is whoever is named in the copyright or
          copyrights for the package.
        * "You" is you, if you're thinking about copying or distributing
          this Package.
        * "Reasonable copying fee" is whatever you can justify on the
          basis of media cost, duplication charges, time of people
          involved, and so on. (You will not be required to justify it
          to the Copyright Holder, but only to the computing community
          at large as a market that must bear the fee.)
        * "Freely Available" means that no fee is charged for the item
          itself, though there may be fees involved in handling the
          item. It also means that recipients of the item may
          redistribute it under the same conditions they received it.

    You may make and give away verbatim copies of the source form of the
    Standard Version of this Package without restriction, provided that
    you duplicate all of the original copyright notices and associated
    disclaimers.

    You may apply bug fixes, portability fixes and other modifications
    derived from the Public Domain or from the Copyright Holder. A
    Package modified in such a way shall still be considered the
    Standard Version.

    You may otherwise modify your copy of this Package in any way,
    provided that you insert a prominent notice in each changed file
    stating how and when you changed that file, and provided that you do
    at least ONE of the following:

       1. place your modifications in the Public Domain or otherwise
          make them Freely Available, such as by posting said
          modifications to Usenet or an equivalent medium, or placing
          the modifications on a major archive site such as
          uunet.uu.net, or by allowing the Copyright Holder to include
          your modifications in the Standard Version of the Package.
       2. use the modified Package only within your corporation or
          organization.
       3. rename any non-standard executables so the names do not
          conflict with standard executables, which must also be
          provided, and provide a separate manual page for each
          non-standard executable that clearly documents how it differs
          from the Standard Version.
       4. make other distribution arrangements with the Copyright Holder.

    You may distribute the programs of this Package in object code or
    executable form, provided that you do at least ONE of the following:

       1. distribute a Standard Version of the executables and library
          files, together with instructions (in the manual page or
          equivalent) on where to get the Standard Version.
       2. accompany the distribution with the machine-readable source of
          the Package with your modifications.
       3. give non-standard executables non-standard names, and clearly
          document the differences in manual pages (or equivalent),
          together with instructions on where to get the Standard Version.
       4. make other distribution arrangements with the Copyright Holder.

    You may charge a reasonable copying fee for any distribution of this
    Package. You may charge any fee you choose for support of this
    Package. You may not charge a fee for this Package itself. However,
    you may distribute this Package in aggregate with other (possibly
    commercial) programs as part of a larger (possibly commercial)
    software distribution provided that you do not advertise this
    Package as a product of your own. You may embed this Package's
    interpreter within an executable of yours (by linking); this shall
    be construed as a mere form of aggregation, provided that the
    complete Standard Version of the interpreter is so embedded.

    The scripts and library files supplied as input to or produced as
    output from the programs of this Package do not automatically fall
    under the copyright of this Package, but belong to whomever
    generated them, and may be sold commercially, and may be aggregated
    with this Package. If such scripts or library files are aggregated
    with this Package via the so-called "undump" or "unexec" methods of
    producing a binary executable image, then distribution of such an
    image shall neither be construed as a distribution of this Package
    nor shall it fall under the restrictions of Paragraphs 3 and 4,
    provided that you do not represent such an executable image as a
    Standard Version of this Package.

    C subroutines (or comparably compiled subroutines in other
    languages) supplied by you and linked into this Package in order to
    emulate subroutines and variables of the language defined by this
    Package shall not be considered part of this Package, but are the
    equivalent of input as in Paragraph 6, provided these subroutines do
    not change the language in any way that would cause it to fail the
    regression tests for the language.

    Aggregation of this Package with a commercial distribution is always
    permitted provided that the use of this Package is embedded; that
    is, when no overt attempt is made to make this Package's interfaces
    visible to the end user of the commercial distribution. Such use
    shall not be construed as a distribution of this Package.

    The End

    The name of the Copyright Holder may not be used to endorse or
    promote products derived from this software without specific prior
    written permission.

    THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
    MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

RMagick is Copyright © 2003 by Timothy P. Hunter
