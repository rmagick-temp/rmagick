#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#flop method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj.flop!

#jj.display
jj.write('flop.jpg')
exit
