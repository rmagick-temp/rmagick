#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#adaptive_threshold method

img = Magick::Image.read("images/Flower_Hat.jpg").first

begin
    result = img.adaptive_threshold

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    result = Magick::Image.read("images/notimplemented.gif").first
    result.resize!(img.columns, img.rows)
end

result.write("adaptive_threshold.jpg")
exit

