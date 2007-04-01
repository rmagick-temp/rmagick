require 'mkmf'
require 'date'

RMAGICK_VERS = '0.0.0'
MIN_IM_VERS = '6.2.6'
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
int main(){ #{enum} x = #{value}; x = x; return 0; }
SRC
      $defs.push(format("-DHAVE_%s", value.upcase))
      true
    else
      false
    end
  end
end




# Create a config.h file from the $defs array.
def create_config_file(file)
  message "creating #{file}\n"
  File.open(file, "w") do |f|
    while (sym = $defs.shift)
      define = sym.sub(/\A-D/, '#define ')
      if define['=']
        define['='] = ' '   # symbols with values
      else
        define << ' 1'      # symbols without values
      end
      f.puts define
    end
  end
end




def exit_failure(msg)
  Logging::message msg
  message msg+"\n"
  exit(1)
end




unless checking_for("Ruby version >= 1.8.2") do
  version = RUBY_VERSION.tr('.','').to_i
  version >= 182
end
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Ruby 1.8.2 or later required."
end


# Check for Magick-config
unless find_executable('Magick-config')
  exit_failure "Can't install RMagick #{RMAGICK_VERS}. Can't find Magick-config in your PATH"
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

headers = ['stdio.h', 'stdint.h']
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

have_func('snprintf', ['stdio.h'])


  ['AdaptiveBlurImageChannel',       # 6.2.8-5
   'AdaptiveResizeImage',            # 6.2.9
   'AdaptiveSharpenImage',           # 6.2.7
   'InterpolatePixelColor',          # 6.2.9
   'IsImageSimilar',                 # 6.2.8
   'LinearStretchImage',             # 6.3.1
   'OrderedPosterizeImageChannel',   # 6.3.0
   'PolaroidImage',                  # 6.3.1-6
   'RecolorImage',                   # 6.3.1-3
   'SketchImage',                    # 6.2.8
   'TransposeImage',                 # 6.2.8
   'TransverseImage',                # 6.2.8
   'UniqueImageColors'               # 6.2.8
   ].each do |func|
    have_func(func, headers)
  end


have_type('long double')
have_type('unsigned long long')
have_type('uint64_t', headers)
have_type('__int64')
check_sizeof('Image *', headers)


have_struct_member('Image', 'transparent_color', headers)         # 6.2.9
have_enum_value('MagickLayerMethod', 'CoalesceLayer', headers)    # 6.2.7
have_enum_value('ImageType', 'PaletteBilevelMatteType', headers)  # 6.2.9


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
$defs.push("-DPACKAGE_STRING=\"RMagick #{RMAGICK_VERS}\"")

create_config_file('rmagick_config.h')

# Force re-compilation if the generated Makefile or
# rmagick_config.h changed.
$config_h = 'Makefile rmagick_config.h'

create_makefile("RMagick")

Logging::message SUMMARY
message SUMMARY
