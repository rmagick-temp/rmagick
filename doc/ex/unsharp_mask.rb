#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#enhance method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

ejj = jj.unsharp_mask(0.0, 1.0, 1.0, 0.05)

# Make a before-and-after composite
ejj.crop!(ejj.columns/2, 0, ejj.columns/2, ejj.rows)
jj = jj.composite(ejj, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a black line between the before and after parts.
line = Magick::Draw.new
line.line(jj.columns/2, 0, jj.columns/2, jj.rows)
line.draw(jj)

# Zoom in so we can see the change,
# then crop everything but the face.
jj.resize!(3)
jj.crop!(169,106,222,325)

#jj.display
jj.write('unsharp_mask.jpg')
exit
