#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Demonstrate the splice method.

img = Image.read('images/Flower_Hat.jpg').first

begin
    spliced_img = img.splice(img.columns/2, img.rows/2, 20, 20, 'gray80')
    spliced_img.write('splice.jpg')
rescue NotImplementedError
    not_implemented = Magick::Image.read("images/notimplemented.gif").first
    not_implemented.write('splice.jpg')
end

