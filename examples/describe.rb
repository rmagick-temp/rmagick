
# Purpose: Demonstrate getting information from the image attributes.
# Usage: describe.rb filename1 [filename2...]
# Notes: The output is similar to ImageMagick's identify command.

require 'RMagick'

puts <<END_INFO

This example shows how to extract attributes from an image.

END_INFO

if ARGV.length == 0
    puts "Specify one or more image filenames as arguments."
    exit
end


ARGV.each { |file|
    puts file
    img = Magick::Image::read(file).first
    puts "   Format: #{img.format}"
    puts "   Geometry: #{img.columns}x#{img.rows}"
    puts "   Class: " + case img.class_type
                            when Magick::DirectClass
                                "DirectClass"
                            when Magick::PseudoClass
                                "PseudoClass"
                        end
    puts "   Depth: #{img.depth} bits-per-pixel"
    puts "   Colors: #{img.number_colors}"
    puts "   Filesize: #{img.filesize}"
    puts "   Resolution: #{img.x_resolution.to_i}x#{img.y_resolution.to_i} "+
         "pixels/#{img.units == Magick::PixelsPerInchResolution ?
         "inch" : "centimeter"}"

    if img.properties.length > 0
        puts "   Properties:"
        img.properties { |name,value|
            puts %Q|      #{name} = "#{value}"|
        }
    end
}