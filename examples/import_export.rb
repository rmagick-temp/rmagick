#
# Demonstrate the export_pixels and import_pixels methods.
#

require 'RMagick'
include Magick

puts <<END_INFO

This example demonstrates the export_pixels and import_pixels methods
by copying an image one row at a time. The result is an copy that
is identical to the original.

END_INFO

img = Image.read('../doc/ex/images/Gold_Statue.jpg').first
copy = Image.new(img.columns, img.rows);

begin
    img.rows.times { |r|
        scanline = img.export_pixels(0, r, img.columns, 1, "RGB");
        copy.import_pixels(0, r, img.columns, 1, "RGB", scanline);
        }
rescue NotImplementedError
    $stderr.puts "The export_pixels and import_pixels methods are not supported" +
                 " by this version of ImageMagick/GraphicsMagick"
    exit
end

copy.write("copy.gif")
exit
