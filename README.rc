h1. RMagick 0.0.0$ README

h2. YY/MM/DD

h3. Table of Contents

* "Introduction":#intro
* "Prerequisites":#prereq
* "Installing RMagick":#install
* "Configuration Options":#config
* "Things that can go wrong":#wrong
* "Upgrading":#upgrade
* "Uninstalling":#uninstall
* "More samples":#samples
* "Reporting Bugs":#bugs
* "Contact Information":#contact
* "Credits":#credits
* "License":#license

h2(#intro). Introduction

RMagick is an interface between the Ruby programming language and the
ImageMagick image processing library.

h2(#prereq). Prerequisites

*O/S* Linux, &#042;BSD, OS X, Windows 2000, XP, Vista, other &#042;nix-like systems.

*Ruby* Version 1.8.2 or later. You can get Ruby from "www.ruby-lang.org":http://www.ruby-lang.org.
The use of versions of Ruby older than 1.8.4 with RMagick is deprecated. Support will be
removed in a future release.

*ImageMagick* Version 6.3.0 or later. You can get ImageMagick from "www.imagemagick.org":http://www.imagemagick.org.

h2(#install). Installing RMagick

First install ImageMagick. Complete and up-to-date instructions for installing
ImageMagick on Linux, &#042;BSD, and other &#042;nix-type O/S's are available
"here":http://rmagick.rubyforge.org/install-linux.html. Use steps 0, 1, and 2.
Similarly, instructions for installing ImageMagick using MacPorts on OS X
are available "here":http://rmagick.rubyforge.org/install-osx.html. Use steps
1 and 2.

This release of RMagick uses Minero Aoki's setup.rb script for installation.
See the next section for configuration options. Usually you do not need to
specify any of these options. You can get more information about setup.rb from
his "web site":<http://i.loveruby.net.

I assume you've already decompressed the tarball, or you wouldn't be reading
this. If you have not decompressed the tarball, do so with one of these commands,
depending on which tarball you have:

<pre>
    tar xvzf RMagick-0.0.0$-tar.gz              (gzipped tarball)
    tar xvjf RMagick-0.0.0$-tar.bz2             (bzipped tarball)
    7z e RMagick-x.y.z.tar.lzma -so | tar xv    (7zipped tarball)
</pre>

Change to the RMagick-0.0.0 directory. If you are not using any
configuration options (usually you don't need to) enter the command

<pre>
   ruby setup.rb
</pre>

Note that setup.rb executes all the example programs, so this can take
some time.  This process both builds the example images used in the
documentation and validates your RMagick installation.

After this command completes, make sure you have root privileges (that
is, login as root or use su or sudo) and enter the command

<pre>
    ruby setup.rb install
</pre>

h2(#config). Configuration options

Type @ruby setup.rb --help@ to see a list of configuration options.  In
addition to the regular options, there are a few RMagick-specific options:

h4. --doc-dir=directory

Specify the directory to install the RMagick documentation.
By default this is $prefix/share/RMagick, where $prefix is the
prefix specified by --prefix. For example, to install the
documentation in /Users/me/RMagick, specify:

<pre>
    ruby setup.rb --doc-dir=/Users/me/RMagick
</pre>

h4. --allow-example-errors

Normally the documentation installation terminates if 5 examples fail.
If you use this option, the installation does not check for failing
examples and will always complete. This option is useful if you're
having trouble installing RMagick and you want to see all the failing examples.

h4. -- disable-htmldoc

By default the install process runs all the RMagick example programs and
generates HTML versions of all the examples.  This option causes the
install process to skip this step.  No install verification will take
place and no documentation will be installed.

h2(#wrong). Things that can go wrong

The "RMagick installation FAQ":http://rmagick.rubyforge.org/install-faq.html
has answers to the most commonly reported problems.

h4. Can't install RMagick. Can't find libMagick or one of the dependent libraries. Check the mkmf.log file for more detailed information.

Typically this message means that one or more of the libraries that ImageMagick
depends on hasn't been installed. Examine the mkmf.log file in the ext/RMagick
subdirectory of the installation directory for any error messages. These messages
typically contain enough additional information for you to be able to diagnose
the problem. Also see "this FAQ":http://rmagick.rubyforge.org/install-faq.html#libmagick.

h4. Cannot open shared object file

If you get a message like this:

<pre>
    $DIR/RMagick.rb:11:in `require': libMagick.so.0:
      cannot open shared object file: No such file or directory -
      $DIR/RMagick2.so (LoadError)
</pre>

you probably do not have the directory in which the ImageMagick library
is installed in your load path.  An easy way to fix this is to define
the directory in the LD_LIBRARY_PATH environment variable.  For
example, suppose you installed the ImageMagick library libMagick.so in
/usr/local/lib.  (By default this is where it is installed.) Create the
LD_LIBRARY_PATH variable like this:

<pre>
        export LD_LIBRARY_PATH=/usr/local/lib
</pre>

On Linux, see @ld(1)@ and @ld.so(8)@ for more information. On other operating
systems, see the documentation for the dynamic loading facility.

h4. No such file or directory - "/tmp/rmagick6872.6"

When setup.rb is running the examples, if you get a message like this:

<pre>
    hook /home/me/src/RMagick-0.0.0/./post-setup.rb failed:
    No such file or directory - "/tmp/rmagick6872.6"
</pre>

you probably do not have a temporary directory environment variable set. Set
the TMPDIR environment variable to your temporary directory. For example:

<pre>
    export TMPDIR=/home/me/tmp
</pre>


h2(#upgrade). Upgrading

If you upgrade to a newer release of ImageMagick, make sure you're using a
release of RMagick that supports that release. It's safe to install a new
release of RMagick over an earlier release.

h2(#uninstall). Uninstalling

The uninstall.rb script will uninstall RMagick completely. Make sure you
have administrator priviledges. Then run this command:

<pre>
    ruby uninstall.rb
</pre>

h2(#samples). More samples

You can find more sample RMagick programs in the /example directory.
These programs are not installed in the RMagick documentation tree.

h2(#bugs). Reporting bugs

Please report bugs in RMagick, its documentation, or its installation
programs to me via the bug tracker on the "RMagick project page":http://rubyforge.org/projects/rmagick.
However, I can't help with Ruby installation and configuration or ImageMagick
installation and configuration. Information about reporting problems and
getting help for ImageMagick is available at the "ImageMagick web site":http://www.imagemagick.org
or the "ImageMagick Forum":http://www.imagemagick.org/discourse-server.

h2(#contact). Contact Information

*Author:* Tim Hunter, Omer Bar-or, Benjamin Thomas

*Email:* "rmagick@rubyforge.org":mailto:rmagick@rubyforge.org

*Web site:* "http://rmagick.rubyforge.org":http://rmagick.rubyforge.org

h2(#credits). Credits

Thanks to "ImageMagick Studio LLC":http://www.imagemagick.org for ImageMagick
and for hosting the RMagick documentation.

h2(#license). License

<pre>
Copyright &copy; 2002-2009 by Timothy P. Hunter

Changes since Nov. 2009 copyright &copy; by Benjamin Thomas and Omer Bar-or

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
</pre>

<hr>
This file is marked up using "Textile":http://hobix.com/textile/ and converted
to HTML with "RedCloth":http://whytheluckystiff.net/ruby/redcloth/.
