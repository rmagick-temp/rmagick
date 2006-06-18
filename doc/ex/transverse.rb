#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#transverse method

img = Magick::Image.read('images/Flower_Hat.jpg').first

begin
    img = img.transverse
rescue NotImplementedError
    not_implemented = Magick::Image.read("images/notimplemented.gif").first
    not_implemented.resize!(img.columns, img.rows)
    img = not_implemented
end
img.write('transverse.jpg')
exit
