#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#enhance method

img = Magick::Image.read('images/Flower_Hat.jpg').first

eimg = img.unsharp_mask(3.0, 1.0, 1.0, 0.05)

img.resize!(3)
eimg.resize!(3)

# Make a before-and-after composite
eimg.crop!(eimg.columns/2, 0, eimg.columns/2, eimg.rows)
img = img.composite(eimg, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a black line between the before and after parts.
line = Magick::Draw.new
line.line(img.columns/2, 0, img.columns/2, img.rows)
line.draw(img)

# Zoom in so we can see the change,
# then crop everything but the face.
img.crop!(Magick::CenterGravity, 250, 200)

#img.display
img.write('unsharp_mask.jpg')
exit
