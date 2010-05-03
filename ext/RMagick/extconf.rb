require "mkmf"
require "date"

RMAGICK_VERS = "0.0.0"
MIN_RUBY_VERS = "1.8.5"
MIN_RUBY_VERS_NO = MIN_RUBY_VERS.tr(".","").to_i
MIN_IM_VERS = "6.4.9"
MIN_IM_VERS_NO = MIN_IM_VERS.tr(".","").to_i



# Test for a specific value in an enum type
def have_enum_value(enum, value, headers=nil, &b)
  checking_for "#{enum}.#{value}" do
    if try_compile(<<"SRC", &b)
#{COMMON_HEADERS}
#{cpp_include(headers)}
/*top*/
int main() { #{enum} t = #{value}; t = t; return 0; }
SRC
      $defs.push(format("-DHAVE_ENUM_%s", value.upcase))
      true
    else
      false
    end
  end
end




# Test for multiple values of the same enum type
def have_enum_values(enum, values, headers=nil, &b)
  values.each do |value|
    have_enum_value(enum, value, headers, &b)
  end
end




def exit_failure(msg)
  Logging::message msg
  message msg+"\n"
  exit(1)
end




# Seems like lots of people have multiple versions of ImageMagick installed.
def check_multiple_imagemagick_versions()
   versions = []
   path = ENV['PATH'].split(File::PATH_SEPARATOR)
   path.each do |dir|
      file = File.join(dir, "Magick-config")
      if File.executable? file
         vers = `#{file} --version`.chomp.strip
         prefix = `#{file} --prefix`.chomp.strip
         versions << [vers, prefix, dir]
      end
   end
   versions.uniq!
   if versions.size > 1
      msg = "\nWarning: Found more than one ImageMagick installation. This could cause problems at runtime.\n"
      versions.each do |vers, prefix, dir|
         msg << "         #{dir}/Magick-config reports version #{vers} is installed in #{prefix}\n"
      end
      msg << "Using #{versions[0][0]} from #{versions[0][1]}.\n\n"
      Logging::message msg
      message msg
   end
end


# Ubuntu (maybe other systems) comes with a partial installation of
# ImageMagick in the prefix /usr (some libraries, no includes, and no
# binaries). This causes problems when /usr/lib is in the path (e.g., using
# the default Ruby installation).
def check_partial_imagemagick_versions()
   prefix = config_string("prefix")
   matches = [
     prefix+"/lib/lib?agick*",
     prefix+"/include/ImageMagick",
     prefix+"/bin/Magick-config",
   ].map do |file_glob|
     Dir.glob(file_glob)
   end
   matches.delete_if { |arr| arr.empty? }
   if 0 < matches.length and matches.length < 3
      msg = "\nWarning: Found a partial ImageMagick installation. Your operating system likely has some built-in ImageMagick libraries but not all of ImageMagick. This will most likely cause problems at both compile and runtime.\nFound partial installation at: "+prefix+"\n"
      Logging::message msg
      message msg
   end
end



if RUBY_PLATFORM =~ /mswin/
  abort <<END_MSWIN
+----------------------------------------------------------------------------+
| This rmagick gem is for use only on Linux, BSD, OS X, and similar systems  |
| that have a gnu or similar toolchain installed. The rmagick-win32 gem is a |
| pre-compiled version of RMagick bundled with ImageMagick for use on        |
| Microsoft Windows systems. The rmagick-win32 gem is available on RubyForge.|
| See http://rmagick.rubyforge.org/install-faq.html for more information.    |
+----------------------------------------------------------------------------+
END_MSWIN
end




unless checking_for("Ruby version >= #{MIN_RUBY_VERS}") do
  version = RUBY_VERSION.tr(".","").to_i
  version >= MIN_RUBY_VERS_NO
end
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Ruby #{MIN_RUBY_VERS} or later required.\n"
end




# Magick-config is not available on Windows
if RUBY_PLATFORM !~ /mswin|mingw/

  # Check for compiler. Extract first word so ENV['CC'] can be a program name with arguments.
  cc = (ENV["CC"] or Config::CONFIG["CC"] or "gcc").split(' ').first
  unless find_executable(cc)
    exit_failure "No C compiler found in ${ENV['PATH']}. See mkmf.log for details."
  end

  # Check for Magick-config
  unless find_executable("Magick-config")
    exit_failure "Can't install RMagick #{RMAGICK_VERS}. Can't find Magick-config in #{ENV['PATH']}\n"
  end

  check_multiple_imagemagick_versions()
  check_partial_imagemagick_versions()

  # Ensure minimum ImageMagick version
  unless checking_for("ImageMagick version >= #{MIN_IM_VERS}")  do
    version = `Magick-config --version`.chomp.tr(".","").to_i
    version >= MIN_IM_VERS_NO
  end
    exit_failure "Can't install RMagick #{RMAGICK_VERS}. You must have ImageMagick #{MIN_IM_VERS} or later.\n"
  end




  $magick_version = `Magick-config --version`.chomp

  # Ensure ImageMagick is not configured for HDRI
  unless checking_for("HDRI disabled version of ImageMagick") do
    not (`Magick-config --version`["HDRI"])
  end
    exit_failure "\nCan't install RMagick #{RMAGICK_VERS}."+
           "\nRMagick does not work when ImageMagick is configured for High Dynamic Range Images."+
           "\nDon't use the --enable-hdri option when configuring ImageMagick.\n"
  end

  # Save flags
  $CFLAGS     = ENV["CFLAGS"].to_s   + " " + `Magick-config --cflags`.chomp
  $CPPFLAGS   = ENV["CPPFLAGS"].to_s + " " + `Magick-config --cppflags`.chomp
  $LDFLAGS    = ENV["LDFLAGS"].to_s  + " " + `Magick-config --ldflags`.chomp
  $LOCAL_LIBS = ENV["LIBS"].to_s     + " " + `Magick-config --libs`.chomp

elsif RUBY_PLATFORM =~ /mingw/  # mingw

  `convert -version` =~ /Version: ImageMagick (\d+\.\d+\.\d+)-\d+ /
  abort "Unable to get ImageMagick version" unless $1
  $magick_version = $1
  $LOCAL_LIBS = '-lCORE_RL_magick_ -lX11'

else  # mswin

  `convert -version` =~ /Version: ImageMagick (\d+\.\d+\.\d+)-\d+ /
  abort "Unable to get ImageMagick version" unless $1
  $magick_version = $1
  $CFLAGS = "-W3"
  $CPPFLAGS = %Q{-I"C:\\Program Files\\Microsoft Platform SDK for Windows Server 2003 R2\\Include" -I"C:\\Program Files\\ImageMagick-#{$magick_version}-Q8\\include"}
  # The /link option is required by the Makefile but causes warnings in the mkmf.log file.
  $LDFLAGS = %Q{/link /LIBPATH:"C:\\Program Files\\Microsoft Platform SDK for Windows Server 2003 R2\\Lib" /LIBPATH:"C:\\Program Files\\ImageMagick-#{$magick_version}-Q8\\lib" /LIBPATH:"C:\\ruby\\lib"}
  $LOCAL_LIBS = 'CORE_RL_magick_.lib X11.lib'

end



#headers = %w{assert.h ctype.h errno.h float.h limits.h math.h stdarg.h stddef.h stdint.h stdio.h stdlib.h string.h time.h}
headers = %w{assert.h ctype.h stdio.h stdlib.h math.h time.h}
headers << "stdint.h" if have_header("stdint.h")  # defines uint64_t
headers << "sys/types.h" if have_header("sys/types.h")


if have_header("wand/MagickWand.h")
   headers << "wand/MagickWand.h"
else
   exit_failure "\nCan't install RMagick #{RMAGICK_VERS}. Can't find MagickWand.h."
end



if RUBY_PLATFORM !~ /mswin|mingw/

  unless have_library("MagickCore", "InitializeMagick", headers) || have_library("Magick", "InitializeMagick", headers) || have_library("Magick++","InitializeMagick",headers)
    exit_failure "Can't install RMagick #{RMAGICK_VERS}. " +
           "Can't find the ImageMagick library or one of the dependent libraries. " +
           "Check the mkmf.log file for more detailed information.\n"
  end
end


have_func("snprintf", headers)
  ["AcquireImage",                   # 6.4.1
   "AffinityImage",                  # 6.4.3-6
   "AffinityImages",                 # 6.4.3-6
   "AutoGammaImageChannel",          # 6.5.5-1
   "AutoLevelImageChannel",          # 6.5.5-1
   "BlueShiftImage",                 # 6.5.4-3
   "ConstituteComponentTerminus",    # 6.5.7-9
   "DeskewImage",                    # 6.4.2-5
   "EncipherImage",                  # 6.3.8-6
   "EqualizeImageChannel",           # 6.3.6-9
   "FloodfillPaintImage",            # 6.3.7
   "FunctionImageChannel",           # 6.4.8-8
   "GetAuthenticIndexQueue",         # 6.4.5-6
   "GetAuthenticPixels",             # 6.4.5-6
   "GetImageAlphaChannel",           # 6.3.9-2
   "GetVirtualPixels",               # 6.4.5-6
   "LevelImageColors",               # 6.4.2
   "LevelColorsImageChannel",        # 6.5.6-4
   "LevelizeImageChannel",           # 6.4.2
   "LiquidRescaleImage",             # 6.3.8-2
   "MagickLibAddendum",              # 6.5.9-1
   "OpaquePaintImageChannel",        # 6.3.7-10
   "QueueAuthenticPixels",           # 6.4.5-6
   "RemapImage",                     # 6.4.4-0
   "RemoveImageArtifact",            # 6.3.6
   "SelectiveBlurImageChannel",      # 6.5.0-3
   "SetImageAlphaChannel",           # 6.3.6-9
   "SetImageArtifact",               # 6.3.6
   "SetMagickMemoryMethods",         # 6.4.1
   "SparseColorImage",               # 6.3.6-?
   "SyncAuthenticPixels",            # 6.4.5-6
   "TransformImageColorspace",       # 6.5.1
   "TransparentPaintImage",          # 6.3.7-10
   "TransparentPaintImageChroma"     # 6.4.5-6
   ].each do |func|
    have_func(func, headers)
  end




checking_for("QueryMagickColorname() new signature")  do
  if try_compile(<<"SRC")
#{COMMON_HEADERS}
#{cpp_include(headers)}
/*top*/
int main() {
  MagickBooleanType okay;
  Image *image;
  MagickPixelPacket *color;
  char *name;
  ExceptionInfo *exception;
  okay = QueryMagickColorname(image, color, SVGCompliance, name, exception);
  return 0;
  }
SRC
    $defs.push("-DHAVE_NEW_QUERYMAGICKCOLORNAME")
    true
  else
    false
  end
end




have_struct_member("Image", "type", headers)          # ???
have_struct_member("DrawInfo", "kerning", headers)    # 6.4.7-8
have_struct_member("DrawInfo", "interline_spacing", headers)   # 6.5.5-8
have_struct_member("DrawInfo", "interword_spacing", headers)   # 6.4.8-0
have_type("DitherMethod", headers)                    # 6.4.2
have_type("MagickFunction", headers)                  # 6.4.8-8
have_type("ImageLayerMethod", headers)                # 6.3.6 replaces MagickLayerMethod
have_type("long double", headers)
#have_type("unsigned long long", headers)
#have_type("uint64_t", headers)
#have_type("__int64", headers)
#have_type("uintmax_t", headers)
#check_sizeof("unsigned long", headers)
#check_sizeof("Image *", headers)


have_enum_values("AlphaChannelType", ["CopyAlphaChannel",                    # 6.4.3-7
                                      "BackgroundAlphaChannel"], headers)    # 6.5.2-5
have_enum_values("CompositeOperator", ["BlurCompositeOp",                    # 6.5.3-7
                                       "DistortCompositeOp",                 # 6.5.3-10
                                       "LinearBurnCompositeOp",              # 6.5.4-3
                                       "LinearDodgeCompositeOp",             # 6.5.4-3
                                       "MathematicsCompositeOp",             # 6.5.4-3
                                       "PegtopLightCompositeOp",             # 6.5.4-3
                                       "PinLightCompositeOp",                # 6.5.4-3
                                       "VividLightCompositeOp"], headers)    # 6.5.4-3
have_enum_values("CompressionType", ["DXT1Compression",                      # 6.3.9-3
                                     "DXT3Compression",                      # 6.3.9-3
                                     "DXT5Compression",                      # 6.3.9-3
                                     "ZipSCompression",                      # 6.5.5-4
                                     "PizCompression",                       # 6.5.5-4
                                     "Pxr24Compression",                     # 6.5.5-4
                                     "B44Compression",                       # 6.5.5-4
                                     "B44ACompression"], headers)            # 6.5.5-4

have_enum_values("DistortImageMethod", ["BarrelDistortion",                  # 6.4.2-5
                                        "BarrelInverseDistortion",           # 6.4.3-8
                                        "BilinearForwardDistortion",         # 6.5.1-2
                                        "BilinearReverseDistortion",         # 6.5.1-2
                                        "DePolarDistortion",                 # 6.4.2-6
                                        "PolarDistortion",                   # 6.4.2-6
                                        "PolynomialDistortion",              # 6.4.2-4
                                        "ShepardsDistortion"], headers)      # 6.4.2-4
have_enum_value("DitherMethod", "NoDitherMethod", headers)                   # 6.4.3
have_enum_values("FilterTypes", ["KaiserFilter",                             # 6.3.6
                                 "WelshFilter",                              # 6.3.6-4
                                 "ParzenFilter",                             # 6.3.6-4
                                 "LagrangeFilter",                           # 6.3.7-2
                                 "BohmanFilter",                             # 6.3.7-2
                                 "BartlettFilter",                           # 6.3.7-2
                                 "SentinelFilter"], headers)                 # 6.3.7-2
have_enum_values("MagickEvaluateOperator", ["PowEvaluateOperator",           # 6.4.1-9
                                           "LogEvaluateOperator",            # 6.4.2
                                           "ThresholdEvaluateOperator",      # 6.4.3
                                           "ThresholdBlackEvaluateOperator", # 6.4.3
                                           "ThresholdWhiteEvaluateOperator", # 6.4.3
                                           "GaussianNoiseEvaluateOperator",  # 6.4.3
                                           "ImpulseNoiseEvaluateOperator",   # 6.4.3
                                           "LaplacianNoiseEvaluateOperator", # 6.4.3
                                           "MultiplicativeNoiseEvaluateOperator", # 6.4.3
                                           "PoissonNoiseEvaluateOperator",   # 6.4.3
                                           "UniformNoiseEvaluateOperator",   # 6.4.3
                                           "CosineEvaluateOperator",         # 6.4.8-5
                                           "SineEvaluateOperator",           # 6.4.8-5
                                           "AddModulusEvaluateOperator"],    # 6.4.8-5
                                                                 headers)
have_enum_values("MagickFunction", ["ArcsinFunction",                        # 6.5.2-8
                                    "ArctanFunction",                        # 6.5.2-8
                                    "PolynomialFunction",                    # 6.4.8-8
                                    "SinusoidFunction"], headers)            # 6.4.8-8
have_enum_values("ImageLayerMethod", ["FlattenLayer",                           # 6.3.6-2
                                      "MergeLayer",                             # 6.3.6
                                      "MosaicLayer",                            # 6.3.6-2
                                      "TrimBoundsLayer" ], headers)             # 6.4.3-8
have_enum_values("VirtualPixelMethod", ["HorizontalTileVirtualPixelMethod",     # 6.4.2-6
                                        "VerticalTileVirtualPixelMethod",       # 6.4.2-6
                                        "HorizontalTileEdgeVirtualPixelMethod", # 6.5.0-1
                                        "VerticalTileEdgeVirtualPixelMethod",   # 6.5.0-1
                                        "CheckerTileVirtualPixelMethod"],       # 6.5.0-1
                                                                 headers)


# Now test Ruby 1.9.0 features.
headers = ["ruby.h"]
if have_header("ruby/io.h")
   headers << "ruby/io.h"
else
   headers << "rubyio.h"
end

have_func("rb_frame_this_func", headers)

# Miscellaneous constants
$defs.push("-DRUBY_VERSION_STRING=\"ruby #{RUBY_VERSION}\"")
$defs.push("-DRMAGICK_VERSION_STRING=\"RMagick #{RMAGICK_VERS}\"")

create_header()
# Prior to 1.8.5 mkmf duplicated the symbols on the command line and in the
# extconf.h header. Suppress that behavior by removing the symbol array.
$defs = []

# Force re-compilation if the generated Makefile changed.
$config_h = "Makefile rmagick.h"

create_makefile("RMagick2")


SUMMARY = <<"END_SUMMARY"


#{"=" * 70}
#{DateTime.now.strftime("%a %d%b%y %T")}
This installation of RMagick #{RMAGICK_VERS} is configured for
Ruby #{RUBY_VERSION} (#{RUBY_PLATFORM}) and ImageMagick #{$magick_version}
#{"=" * 70}


END_SUMMARY

Logging::message SUMMARY
message SUMMARY
