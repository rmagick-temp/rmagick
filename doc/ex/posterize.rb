#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first

begin

result = img.posterize

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    result = Magick::Image.read("images/notimplemented.gif").first
    result.resize!(img.columns, img.rows)
end

result.write('posterize.jpg')
exit
