RMagick 0.0.0$ README
================================
YY/MM/DD
--------

Table Of Contents
-----------------

* [Introduction] [intro]
* [Contact Information] [contact]
* [Prerequisites] [prereq]
* [Installing RMagick] [install]
  + [Configuration options] [options]
* [Things that can go wrong] [uhoh]
* [Upgrading] [upgrade]
* [Uninstalling] [uninstall]
* [More samples] [samples]
* [Known issues] [issues]
* [Reporting bugs] [bugs]
* [Credits] [credits]
* [License] [license]

<h2 id="intro">Introduction</h2>

RMagick is an interface between the Ruby programming language and the
ImageMagick image processing library.

<h2 id="contact">Contact Information</h2>

__Author:__ Tim Hunter

__Email:__ <rmagick@rubyforge.org>

__RubyForge:__ <http://rubyforge.org/projects/rmagick/>

<h2 id="prereq">Prerequisites</h2>

__O/S:__ Linux, Sun Solaris, Cygwin, FreeBSD, OS X.

__Ruby__ 1.8.2 or later. You can get Ruby from <http://www.ruby-lang.org>.

__ImageMagick__ 6.2.8 or later.  You can get ImageMagick from <http://www.imagemagick.org>.

<h2 id="install">Installation</h2>

The installation procedure for RMagick 0.0.0 is different from that used
in earlier releases. Before installing RMagick, you must install ImageMagick.
Complete and up-to-date instructions for installing ImageMagick are available at <http://rmagick.rubyforge.org/install-faq.html>. After installing 
ImageMagick, use the instructions in the next section to install RMagick.

<h2 id="install">Installing RMagick 0.0.0</h2>

This release of RMagick uses Minero Aoki's setup.rb script for installation.
You can get more information about setup.rb from his web site
<http://i.loveruby.net>

Download the latest version of the RMagick tarball from RubyForge. 
Decompress the tarball into a temporary directory using the command

	tar xvzf RMagick-0.0.0-tar.gz 

Change to the RMagick-0.0.0 directory. Run the command

	ruby setup.rb
	
See the next section for configuration options. Usually you do not need to
specify any of these options. Note that setup.rb executes all the example
programs, so this can take some time. This process both builds the example
images used in the documentation and validates your RMagick installation. 

After this command completes, make sure you have administrator priviledges,
then enter the command

	ruby setup.rb install

<h4 id="options">Configuration Options</h4>

Type `ruby setup.rb --help` to see a list of configuration options.  In
addition to the regular options, there are a few RMagick-specific options:

* --doc-dir=_directory_
    >  Specify the directory to install the RMagick documentation.
    >  By default this is $prefix/share/RMagick, where $prefix is the
    >  prefix specified by --prefix. For example, to install the
    >  documentation in /home/user/RMagick, specify:

    >  `./configure --with-doc-dir=/home/user/RMagick`

* --allow-example-errors
    >  Normally the documentation installation terminates if 5 examples fail.
    >  If you use this option, the installation does not check for failing
    >  examples and will always complete. This option is useful if you're 
    >  having trouble installing RMagick and you want to see all the failing examples.

* --disable-htmldoc
    >  By default the install process runs all the RMagick example programs
    >  and generates HTML versions of all the examples. This option causes
    >  the install process to skip this step. No documentation is installed.

<<<<<<< README.txt
=======
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

De-compress the RMagick-0.0.0$.tar.gz archive and enter the top
directory.  Then type:

    $ ./configure <configuration options>
    $ make
    ($ su)
    $ make install
    (optionally)
    $ make clean

>>>>>>> 1.30
<h2 id="uhoh">Things that can go wrong</h2>

#### Can't install RMagick. Can't find libMagick or one of the dependent libraries. Check the config.log file for more detailed information.
Typically this message means that one or more of the libraries that ImageMagick depends on hasn't been installed. Examine the config.log file in the installation directory for any error messages. These messages typically contain enough additional information for you to be able to diagnose the problem.

#### Cannot open shared object file
When make is running the examples, if you get a message like this:

    /home/you/RMagick-0.0.0/lib/RMagick.rb:11:in `require': libMagick.so.0:
      cannot open shared object file: No such file or directory -
      /home/you/RMagick-0.0.0/ext/RMagick/RMagick.so (LoadError)

you probably do not have the directory in which the ImageMagick library is
installed in your load path. An easy way to fix this is to define the
directory in the LD\_LIBRARY\_PATH environment variable. For example, suppose
you installed the ImageMagick library libMagick.so in /usr/local/lib. (By
default this is where it is installed.) Create the LD\_LIBRARY\_PATH variable
like this:

    export LD_LIBRARY_PATH=/usr/local/lib

On Linux, see `ld(1)` and `ld.so(8)` for more information. On other operating
systems, see the documentation for the dynamic loading facility.

#### No such file or directory - "/tmp/rmagick6872.6"
When make is running the examples, if you get a message like this:

    hook /home/me/src/RMagick-0.0.0/./post-setup.rb failed:
    No such file or directory - "/tmp/rmagick6872.6"

you probably do not have a temporary directory environment variable set. Set
the TMPDIR environment variable to your temporary directory. For example:

    export TMPDIR=/home/me/tmp

<h2 id="upgrade">Upgrading</h2>

If you upgrade to a newer release of ImageMagick, make sure you're using a
release of RMagick that supports that release. It's safe to install a newer
release of RMagick over an earlier release.

<h2 id="uninstall">Uninstalling</h2>

The `uninstall.rb` script will uninstall RMagick completely. Make sure you
have administrator priviledges. Then run this command:

    ruby uninstall.rb

<h2 id="samples">More samples</h2>

You can find more sample RMagick programs in the /example directory.
These programs are not installed in the RMagick documentation tree.

<h2 id="bugs">Reporting bugs</h2>

Please report bugs in RMagick, its documentation, or its installation
programs to me via the bug tracker on the [RMagick project page at
RubyForge](http://rubyforge.org/projects/rmagick).  However, I can't help
with Ruby installation and configuration or ImageMagick
installation and configuration. Information about reporting problems and
getting help for ImageMagick is available at the ImageMagick web site
(http://www.imagemagick.org).

<h2 id="credits">Credits</h2>

Thanks to

   * ImageMagick Studio LLC, for ImageMagick and for hosting the RMagick documentation.
   * Kaspar Schiess and Brett DiFrischia, for the MS Windows gems.

<h2 id="license">License</h2>

   >  Copyright © 2002-2007 by Timothy P. Hunter
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
[install]: #install
[options]: #options
[uhoh]: #uhoh
[upgrade]: #upgrade
[uninstall]: #uninstall
[samples]: #samples
[undoc]: #undoc
[issues]: #issues
[bugs]: #bugs
[credits]: #credits
[license]: #license

