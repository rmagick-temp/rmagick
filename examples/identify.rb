require 'RMagick'

module Magick
    class Image

        # Print information similar to the identify -verbose command
        def identify
            printf "Image: "
            puts "#{base_filename}=>" if base_filename != filename
            puts filename + "\n"
            puts "\tFormat: #{format}\n"
            puts "\tGeometry: #{columns}x#{rows}\n"
            puts "\tClass: #{class_type.to_s}\n"
            puts "\tType: #{image_type.to_s}\n"
            puts "\tEndianess: #{endian}\n"
            puts "\tColorspace: #{colorspace}\n"
            puts "\tChannelDepth:\n"
            color_space = gray? ? Magick::GrayColorspace : colorspace
            case color_space
            when Magick::RGBColorspace
                puts "\t\tRed: #{channel_depth(Magick::RedChannel)}-bits\n"
                puts "\t\tGreen: #{channel_depth(Magick::GreenChannel)}-bits\n"
                puts "\t\tBlue: #{channel_depth(Magick::BlueChannel)}-bits\n"
                puts "\t\tOpacity: #{channel_depth(Magick::OpacityChannel)}-bits\n" if matte
            when Magick::CMYKColorspace
                puts "\t\tCyan : #{channel_depth(Magick::CyanChannel)}-bits\n"
                puts "\t\tMagenta: #{channel_depth(Magick::MagentaChannel)}-bits\n"
                puts "\t\tYellow: #{channel_depth(Magick::YellowChannel)}-bits\n"
                puts "\t\tBlack: #{channel_depth(Magick::BlackChannel)}-bits\n"
                puts "\t\tOpacity: #{channel_depth(Magick::OpacityChannel)}-bits\n" if matte
            when Magick::GrayColorspace
                puts "\t\tGray: #{channel_depth(Magick::GrayChannel)}-bits\n"
                puts "\t\tOpacity: #{channel_depth(Magick::OpacityChannel)}-bits\n" if matte
            end
            scale = Magick::QuantumRange / (Magick::QuantumRange >> (Magick::QuantumDepth-channel_depth))
            puts "\tChannel statistics:\n"
            case color_space
            when Magick::RGBColorspace
                puts "\t\tRed:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::RedChannel)[0]/scale, channel_extrema(Magick::RedChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::RedChannel)[1]/scale, channel_extrema(Magick::RedChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::RedChannel)[0]/scale, channel_mean(Magick::RedChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::RedChannel)[1]/scale, channel_mean(Magick::RedChannel)[1]/Magick::QuantumRange)
                puts "\t\tGreen:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::GreenChannel)[0]/scale, channel_extrema(Magick::GreenChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::GreenChannel)[1]/scale, channel_extrema(Magick::GreenChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::GreenChannel)[0]/scale, channel_mean(Magick::GreenChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::GreenChannel)[1]/scale, channel_mean(Magick::GreenChannel)[1]/Magick::QuantumRange)
                puts "\t\tBlue:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::BlueChannel)[0]/scale, channel_extrema(Magick::BlueChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::BlueChannel)[1]/scale, channel_extrema(Magick::BlueChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::BlueChannel)[0]/scale, channel_mean(Magick::BlueChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::BlueChannel)[1]/scale, channel_mean(Magick::BlueChannel)[1]/Magick::QuantumRange)
            when Magick::CMYKColorspace
                puts "\t\tCyan:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::CyanChannel)[0]/scale, channel_extrema(Magick::CyanChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::CyanChannel)[1]/scale, channel_extrema(Magick::CyanChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::CyanChannel)[0]/scale, channel_mean(Magick::CyanChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::CyanChannel)[1]/scale, channel_mean(Magick::CyanChannel)[1]/Magick::QuantumRange)
                puts "\t\tMagenta:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::MagentaChannel)[0]/scale, channel_extrema(Magick::MagentaChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::MagentaChannel)[1]/scale, channel_extrema(Magick::MagentaChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::MagentaChannel)[0]/scale, channel_mean(Magick::MagentaChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::MagentaChannel)[1]/scale, channel_mean(Magick::MagentaChannel)[1]/Magick::QuantumRange)
                puts "\t\tYellow:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::YellowChannel)[0]/scale, channel_extrema(Magick::YellowChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::YellowChannel)[1]/scale, channel_extrema(Magick::YellowChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::YellowChannel)[0]/scale, channel_mean(Magick::YellowChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::YellowChannel)[1]/scale, channel_mean(Magick::YellowChannel)[1]/Magick::QuantumRange)
                puts "\t\tBlack:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::BlackChannel)[0]/scale, channel_extrema(Magick::BlackChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::BlackChannel)[1]/scale, channel_extrema(Magick::BlackChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::BlackChannel)[0]/scale, channel_mean(Magick::BlackChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::BlackChannel)[1]/scale, channel_mean(Magick::BlackChannel)[1]/Magick::QuantumRange)
            when Magick::GrayColorspace
                puts "\t\tGray:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::GrayChannel)[0]/scale, channel_extrema(Magick::GrayChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::GrayChannel)[1]/scale, channel_extrema(Magick::GrayChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean: " + sprintf("%g (%g)\n", channel_mean(Magick::GrayChannel)[0]/scale, channel_mean(Magick::GrayChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation: " + sprintf("%g (%g)\n", channel_mean(Magick::GrayChannel)[1]/scale, channel_mean(Magick::GrayChannel)[1]/Magick::QuantumRange)
            end
            if matte
                puts "\t\tOpacity:\n"
                puts "\t\t\tMin: " + sprintf("%u (%g)\n", channel_extrema(Magick::OpacityChannel)[0]/scale, channel_extrema(Magick::OpacityChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tMax: " + sprintf("%u (%g)\n", channel_extrema(Magick::OpacityChannel)[1]/scale, channel_extrema(Magick::OpacityChannel)[1]/Magick::QuantumRange)
                puts "\t\t\tMean:" + sprintf("%u (%g)\n", channel_mean(Magick::OpacityChannel)[0]/scale, channel_mean(Magick::OpacityChannel)[0]/Magick::QuantumRange)
                puts "\t\t\tStandard deviation:" + sprintf("%u (%g)\n", channel_mean(Magick::OpacityChannel)[1]/scale, channel_mean(Magick::OpacityChannel)[1]/Magick::QuantumRange)
            end
            if class_type == Magick::DirectClass
                puts "\tColors: #{total_colors}\n"
            else
                if total_colors <= colors
                    puts "\tColors: #{colors}\n"
                else
                    puts "\tColors: #{total_colors}=>#{colors}\n"
                end
            end
            # Histogram goes here
            puts "\tMean error per pixel: #{mean_error_per_pixel}\n" if mean_error_per_pixel != 0.0
            puts "\tNormalized mean error: #{normalized_mean_error}\n" if normalized_mean_error != 0.0
            puts "\tNormalized maximum error: #{normalized_maximum_error}\n" if normalized_maximum_error != 0.0
            puts "\tRendering-intent: #{rendering_intent.to_s}\n"
            puts "\tGamma: #{gamma}\n" if gamma != 0.0
            chrom = chromaticity
            if chrom.red_primary.x != 0.0 || chrom.green_primary.x != 0.0 || chrom.blue_primary.x != 0.0 || chrom.white_point.x != 0.0
                puts "\tChromaticity:\n"
                puts "\t\tred primary: (#{sprintf("%g,%g", chrom.red_primary.x, chrom.red_primary.y)})\n"
                puts "\t\tgreen primary: (#{sprintf("%g,%g", chrom.green_primary.x, chrom.green_primary.y)})\n"
                puts "\t\tblue primary: (#{sprintf("%g,%g", chrom.blue_primary.x, chrom.blue_primary.y)})\n"
                puts "\t\twhite point: (#{sprintf("%g,%g", chrom.white_point.x, chrom.white_point.y)})\n"
            end
            ex_info = extract_info
            if ex_info.width * ex_info.height != 0.0
                puts "\tTile geometry: #{ex_info.width}x#{ex_info.height}+#{ex_info.x}+#{ex_info.y}\n"
            end
            if x_resolution != 0.0 && y_resolution != 0.0
                puts "\tResolution: #{sprintf("%gx%g", x_resolution, y_resolution)}\n"
            end
            puts "\tUnits: #{units.to_s}\n"
            size = filesize
            if size >= 1048576
                puts "\tFilesize: #{"%.1f" % (size/1048576.0)}mb\n"
            elsif size >= 1024
                puts "\tFilesize: #{"%.0f" % (size/1024.0)}kb\n"
            else
                puts "\tFilesize: #{size}b\n"
            end
            puts "\tInterlace: #{interlace.to_s}\n"
            puts "\tBackground Color: #{background_color}\n"
            puts "\tBorder Color: #{border_color}\n"
            puts "\tMatte Color: #{matte_color}\n"
            pg = page
            if pg.width != 0 || pg.height != 0 || pg.x != 0 || pg.y != 0
                puts "\tPage geometry: #{pg.width}x#{pg.height}+#{pg.x}+#{pg.y}\n"
            end
            puts "\tDispose: #{dispose.to_s}\n"
            puts "\tDelay: #{delay}\n" if delay != 0
            puts "\tIterations: #{iterations}\n" unless iterations == 1
            puts "\tScene: #{scene}\n" if scene != 0
            puts "\tCompression: #{compression.to_s}\n"
            puts "\tQuality: #{quality}\n" unless quality == 0
            puts "\tOrientation: #{orientation.to_s}\n"
            puts "\tMontage: #{montage}\n" if montage
            signature # compute but ignore - will be displayed along with the other properties
            properties.each do |prop, value|
                next if prop[0,1] == '['
                puts "\t#{prop}: #{value}\n"
            end
            clip_path = self["8BIM:1999,2998:#1"]
            if clip_path
                puts "\tClipping path: #{clip_path}\n"
            end
            each_profile do |name, value|
                puts "\tProfile-#{name}: #{value.length}\n"
                if name == 'exif'
                    exif_attrs = get_exif_by_entry
                    exif_attrs.each do |attr|
                        puts "\t\t#{attr[0]}: #{attr[1]}\n"
                    end
                end
            end
            puts "\tTainted: True\n" if changed?
            puts "\tTainted: False\n" unless changed?
            puts "\tVersion: #{Magick::Version}\n"
            puts "\t         #{Magick::Magick_version}\n"
        end
    end
end

if ARGV.length == 0
    puts <<-'END_USAGE'
    This example displays information about the specified image file(s)
    that is similar to ImageMagick/GraphicsMagick's identify command.

    Usage:
    ruby identify.rb filename [filename...]
    END_USAGE
    exit
end

ilist = Magick::ImageList.new(*ARGV)
ilist.each do |img|
    img.identify
end

exit

