#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#blur_image method

lighthouse = Magick::Image.read('images/Lighthouse.jpg').first

# Crop to the interesting part of the picture.
lighthouse.crop!(202, 184, 206, 230)

# Make a blurry copy.
blurry = lighthouse.motion_blur(0,7,90)

# Crop the copy, retaining only the right half.
blurry.crop!(blurry.columns/2, 0, blurry.columns/2, blurry.rows)

# Composite the copy over the right half of the original.
result = lighthouse.composite(blurry, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a white line down the middle to emphasize the
# border between the original image and the blurry image.
line = Magick::Draw.new
line.stroke('white')
line.stroke_width(1)
line.line(lighthouse.columns/2, 0, lighthouse.columns/2, lighthouse.rows)
line.draw(result)

#result.display
result.write('motion_blur.jpg')
exit
