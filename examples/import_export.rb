#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('../doc/ex/images/Gold_Statue.jpg').first
copy = Image.new(img.columns, img.rows);

img.rows.times { |r|
    scanline = img.export_pixels(0, r, img.columns, 1, "RGB");
    copy.import_pixels(0, r, img.columns, 1, "RGB", scanline);
    }

copy.display
exit
