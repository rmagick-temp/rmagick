#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#negate_channel method

img = Magick::Image.read('images/Flower_Hat.jpg').first

begin

result = img.negate_channel(false, Magick::GreenChannel)

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    result = Magick::Image.read("images/notimplemented.gif").first
    result.resize!(img.columns, img.rows)
end

result.write('negate_channel.jpg')
exit
