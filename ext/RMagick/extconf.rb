require 'mkmf'
require 'date'

RMAGICK_VERS = '0.0.0$'
MIN_RUBY_VERS = "1.8.2"
MIN_RUBY_VERS_NO = MIN_RUBY_VERS.tr(',','').to_i
MIN_IM_VERS = '6.3.0'
MIN_IM_VERS_NO = MIN_IM_VERS.tr('.','').to_i

SUMMARY = <<"END_SUMMARY"


#{'=' * 70}
#{DateTime.now.strftime("%a %d%b%y %T")}
This installation of RMagick #{RMAGICK_VERS} is configured
for Ruby #{RUBY_VERSION} (#{RUBY_PLATFORM}) and ImageMagick #{`Magick-config --version`.chomp}.
#{'=' * 70}


END_SUMMARY




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




unless checking_for("Ruby version >= #{MIN_RUBY_VERS}") do
  version = RUBY_VERSION.tr('.','').to_i
  version >= MIN_RUBY_VERS_NO
end
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Ruby #{MIN_RUBY_VERS} or later required."
end


# Check for Magick-config
unless find_executable('Magick-config')
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Can't find Magick-config in #{ENV['PATH']}"
end


# Ensure minimum ImageMagick version
unless checking_for("ImageMagick version >= #{MIN_IM_VERS}")  do
  version = `Magick-config --version`.chomp.tr('.','').to_i
  version >= MIN_IM_VERS_NO
end
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. You must have ImageMagick #{MIN_IM_VERS} or later."
end

# Save flags
$CFLAGS = ENV['CFLAGS'] + ' ' + `Magick-config --cflags`.chomp
$CPPFLAGS = `Magick-config --cppflags`.chomp
$LDFLAGS = `Magick-config --ldflags`.chomp
$LOCAL_LIBS = `Magick-config --libs`.chomp

#headers = %w{assert.h ctype.h errno.h float.h limits.h math.h stdarg.h stddef.h stdint.h stdio.h stdlib.h string.h time.h}
headers = %w{assert.h ctype.h stdint.h stdio.h stdlib.h math.h time.h}
headers << 'sys/types.h' if have_header('sys/types.h')


unless have_header('magick/MagickCore.h')
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Can't find MagickCore.h."
else
  headers << 'magick/MagickCore.h'
end

unless have_library('Magick', 'InitializeMagick', headers)
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. " +
               "Can't find libMagick or one of the dependent libraries. " +
               "Check the mkmf.log file for more detailed information."
end

have_func('snprintf', headers)


  ['DistortImage',                   # 6.3.5
   'InterpolatePixelColor',          # 6.2.9
   'IsImageSimilar',                 # 6.2.8
   'LinearStretchImage',             # 6.3.1
   'OrderedPosterizeImageChannel',   # 6.3.0
   'PolaroidImage',                  # 6.3.1-6
   'RecolorImage',                   # 6.3.1-3
   'SeparateImages',                 # 6.2.9
   'SetImageRegistry',               # 6.3.4-?
   'SketchImage',                    # 6.2.8
   'TransposeImage',                 # 6.2.8
   'TransverseImage',                # 6.2.8
   'UniqueImageColors'               # 6.2.8
   ].each do |func|
    have_func(func, headers)
  end


have_type('long double', headers)
have_type('unsigned long long', headers)
have_type('uint64_t', headers)
have_type('__int64', headers)
check_sizeof('Image *', headers)


have_struct_member('Image', 'transparent_color', headers)               # 6.2.9
have_enum_value('ImageType', 'PaletteBilevelMatteType', headers)        # 6.2.9
have_enum_value('CompositeOperator', 'ChangeMaskCompositeOp', headers)  # 6.3.3
have_enum_values('MagickLayerMethod', ['CoalesceLayer',                 # 6.2.7
                                       'OptimizeTransLayer',            # 6.3.?
                                       'RemoveDupsLayer',               # 6.3.3-6
                                       'RemoveZeroLayer',               # 6.3.3-6
                                       'CompositeLayer',                # 6.3.3-6
                                       'OptimizeImageLayer'], headers)  # 6.3.3-?
have_enum_value('NoiseType', 'RandomNoise', headers)                    # 6.3.5-0



# ColorInfo.color changed in 6.3.0
checking_for "ColorInfo.color is a MagickPixelPacket" do
  if try_compile(<<"SRC")
#{COMMON_HEADERS}
#{cpp_include(headers)}
/*top*/
int main()
{
  ColorInfo color_info;
  MagickPixelPacket p;
  color_info.color = p;
  return 0;
}
SRC
    $defs.push("-DHAVE_NEW_COLORINFO")
    true
  else
    false
  end
end

# Miscellaneous constants
$defs.push("-DRUBY_VERSION_STRING=\"ruby #{RUBY_VERSION}\"")
$defs.push("-DRMAGICK_VERSION_STRING=\"RMagick #{RMAGICK_VERS}\"")

create_header()

# Force re-compilation if the generated Makefile changed.
$config_h = 'Makefile rmagick.h'

create_makefile("RMagick")

Logging::message SUMMARY
message SUMMARY
