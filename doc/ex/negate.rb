#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#negate method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj = jj.negate

#jj.display
jj.write('negate.jpg')
exit
