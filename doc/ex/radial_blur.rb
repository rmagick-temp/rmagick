#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#radial_blur method

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Make a blurry copy.
begin
    result = img.radial_blur(10.0)

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    result = Magick::Image.read("images/notimplemented.gif").first
    result.resize!(img.columns, img.rows)
end

result.write('radial_blur.jpg')
exit
