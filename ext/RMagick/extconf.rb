require "mkmf"
require "date"

RMAGICK_VERS = "0.0.0"
MIN_RUBY_VERS = "1.8.2"
MIN_RUBY_VERS_NO = MIN_RUBY_VERS.tr(".","").to_i
MIN_IM_VERS = "6.3.0"
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




def have_new_rb_cvar_set(headers=nil, &b)
  checking_for "rb_cvar_set with 3 arguments" do
    if try_compile(<<"SRC", &b)
#{COMMON_HEADERS}
#{cpp_include(headers)}
/*top*/
int main() { rb_cvar_set(rb_cArray, rb_intern("x"), INT2FIX(0)); return 0; }
SRC
      $defs.push("-DHAVE_NEW_RB_CVAR_SET")
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




unless checking_for("Ruby version >= #{MIN_RUBY_VERS}") do
  version = RUBY_VERSION.tr(".","").to_i
  version >= MIN_RUBY_VERS_NO
end
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Ruby #{MIN_RUBY_VERS} or later required.\n"
end



# Magick-config is not available on Windows
if RUBY_PLATFORM !~ /mswin/
  # Check for Magick-config
  unless find_executable("Magick-config")
    exit_failure "Can't install RMagick #{RMAGICK_VERS}. Can't find Magick-config in #{ENV['PATH']}\n"
  end

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
  $CFLAGS = ENV["CFLAGS"].to_s + " " + `Magick-config --cflags`.chomp
  $CPPFLAGS = `Magick-config --cppflags`.chomp
  $LDFLAGS = `Magick-config --ldflags`.chomp
  $LOCAL_LIBS = `Magick-config --libs`.chomp

else  # mswin

  `convert -version` =~ /Version: ImageMagick (\d\.\d\.\d+) /
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


unless have_header("magick/MagickCore.h")
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Can't find MagickCore.h.\n"
else
  headers << "magick/MagickCore.h"
end


if RUBY_PLATFORM !~ /mswin/

  unless have_library("Magick", "InitializeMagick", headers)
    exit_failure "Can't install RMagick #{RMAGICK_VERS}. " +
           "Can't find libMagick or one of the dependent libraries. " +
           "Check the mkmf.log file for more detailed information.\n"
  end
end


have_func("snprintf", headers)


  ["AcquireQuantumMemory",           # 6.3.5-9
   "ClutImageChannel",               # 6.3.5-8
   "CompositeLayers",                # 6.3.3-?
   "ConvertHSLToRGB",                # 6.3.5-9
   "ConvertRGBToHSL",                # 6.3.5-9
   "DistortImage",                   # 6.3.5
   "EqualizeImageChannel",           # 6.3.6-9
   "ExcerptImage",                   # 6.3.5-8
   "ExtentImage",                    # 6.3.1
   "GetImageProperty",               # 6.3.1
   "GetNextImageProperty",           # 6.3.1
   "IsHistogramImage",               # 6.3.5
   "LinearStretchImage",             # 6.3.1
   "LiquidRescaleImage",             # 6.3.8-2
   "MagickCoreGenesis",              # 6.3.4
   "OpaquePaintImageChannel",        # 6.3.7-10
   "PolaroidImage",                  # 6.3.1-6
   "RecolorImage",                   # 6.3.1-3
   "ResetImagePage",                 # 6.3.3
   "ResizeQuantumMemory",            # 6.3.5-9
   "SetImageAlphaChannel",           # 6.3.6-9
   "SetImageProperty",               # 6.3.1
   "SetImageRegistry",               # 6.3.4-?
   "SyncImageProfiles",              # 6.3.2
   "TransparentPaintImage",          # 6.3.7-10
   ].each do |func|
    have_func(func, headers)
  end


have_type("AlphaChannelType", headers)  # 6.3.5
have_type("ImageLayerMethod", headers)  # 6.3.6 replaces MagickLayerMethod
have_type("long double", headers)
have_type("unsigned long long", headers)
have_type("uint64_t", headers)
have_type("__int64", headers)
have_type("uintmax_t", headers)
check_sizeof("unsigned long", headers)
check_sizeof("Image *", headers)


have_enum_value("ColorspaceType", "CMYColorspace", headers)                  # 6.3.5
have_enum_values("CompositeOperator", ["ChangeMaskCompositeOp",              # 6.3.3
                                       "LinearLightCompositeOp",             # 6.3.5
                                       "DivideCompositeOp"], headers)        # 6.3.5
have_enum_values("DistortImageMethod", ["ArcDistortion",                     # 6.3.5-5
                               "PerspectiveProjectionDistortion"], headers)  # 6.3.5-9
have_enum_values("FilterTypes", ["KaiserFilter",                             # 6.3.6
                                 "WelshFilter",                              # 6.3.6-4
                                 "ParzenFilter",                             # 6.3.6-4
                                 "LagrangeFilter",                           # 6.3.7-2
                                 "BohmanFilter",                             # 6.3.7-2
                                 "BartlettFilter",                           # 6.3.7-2
                                 "SentinelFilter"], headers)                 # 6.3.7-2
have_enum_value("InterpolatePixelMethod", "SplineInterpolatePixel", headers) # 6.3.5
have_enum_values("InterlaceType", ["GIFInterlace",                           # 6.3.4
                                  "JPEGInterlace",                           # 6.3.4
                                  "PNGInterlace"], headers)                  # 6.3.4
have_enum_values("MagickLayerMethod", ["OptimizeTransLayer",                 # 6.3.3-4
                                       "RemoveDupsLayer",                    # 6.3.3-6
                                       "RemoveZeroLayer",                    # 6.3.3-6
                                       "CompositeLayer",                     # 6.3.3-6
                                       "FlattenLayer",                       # 6.3.6-2
                                       "MergeLayer",                         # 6.3.6
                                       "MosaicLayer",                        # 6.3.6-2
                                       "OptimizeImageLayer"], headers)       # 6.3.3-?
have_enum_value("MetricType", "MeanErrorPerPixelMetric", headers)            # 6.3.4-?
have_enum_value("NoiseType", "RandomNoise", headers)                         # 6.3.5-0
have_enum_values("VirtualPixelMethod", ["MaskVirtualPixelMethod",            # 6.3.3
                                        "BlackVirtualPixelMethod",           # 6.3.5
                                        "GrayVirtualPixelMethod",            # 6.3.5
                                        "WhiteVirtualPixelMethod"], headers) # 6.3.5


# Now test Ruby 1.9.0 features.
headers = ["ruby.h", "rubyio.h"]
have_func("rb_frame_this_func", headers)
have_new_rb_cvar_set(headers)



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
