#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#gaussian_blur method

lighthouse = Magick::Image.read('images/Lighthouse.jpg').first

# Crop to the interesting part of the picture.
lighthouse.crop!(202, 184, 206, 230)

# Make a blurry copy
blurry = lighthouse.gaussian_blur(0.0, 3.0)

# Copy the copy, retaining only the right half.
blurry.crop!(blurry.columns/2, 0, blurry.columns/2, blurry.rows)

# Composite the copy over the right half of the original.
result = lighthouse.composite(blurry, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a black line between the before and after parts.
line = Magick::Draw.new
line.stroke('white')
line.line(lighthouse.columns/2, 0, lighthouse.columns/2, lighthouse.rows)
line.draw(result)

#result.display
result.write('gaussian_blur.jpg')
exit
