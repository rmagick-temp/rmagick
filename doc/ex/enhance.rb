#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#enhance method

img = Magick::Image.read('images/Flower_Hat.jpg').first
img = img.add_noise(Magick::UniformNoise)

eimg = img.enhance

# Zoom in so we can see the change
img.resize!(3)
eimg.resize!(3)

# Make a before-and-after composite
eimg.crop!(Magick::EastGravity, eimg.columns/2, eimg.rows)
img = img.composite(eimg, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a black line between the before and after parts.
line = Magick::Draw.new
line.line(img.columns/2, 0, img.columns/2, img.rows)
line.draw(img)

# Crop everything but the face.
img.crop!(Magick::CenterGravity, 250, 200)

img.write('enhance.jpg')
exit
