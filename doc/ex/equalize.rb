#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#equalize method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj2 = jj.equalize

# Make a before-and-after composite
jj2.crop!(jj2.columns/2, 0, jj2.columns/2, jj2.rows)
jj = jj.composite(jj2, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a line between the before and after halves.
line = Magick::Draw.new
line.line(jj.columns/2, 0, jj.columns/2, jj.rows)
line.draw(jj)

#jj.display
jj.write('equalize.jpg')
exit
