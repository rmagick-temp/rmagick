#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#edge method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj2 = jj.emboss

# Make a before-and-after composite
jj2.crop!(jj2.columns/2, 0, jj2.columns/2, jj2.rows)
jj = jj.composite(jj2, Magick::EastGravity, Magick::OverCompositeOp)
jj = jj.border(1,1,'black')

#jj.display
jj.write('emboss.jpg')
exit
