#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#adaptive_threshold method

img = Magick::Image.read("images/Grandma.jpg").first
img.resize!(200.0/img.rows)

begin
    adt = img.adaptive_threshold
    adt.crop!(adt.columns/2, 0, adt.columns/2, adt.rows)
    result = img.composite(adt, Magick::EastGravity, Magick::OverCompositeOp)

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    result = Magick::Image.read("images/notimplemented.gif").first
end

#result.display
result.write("adaptive_threshold.jpg")
exit

