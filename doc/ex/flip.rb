#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#flip method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj.flip!

#jj.display
jj.write('flip.jpg')
exit
