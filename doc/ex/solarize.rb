#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#solarize method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj = jj.solarize

#jj.display
jj.write('solarize.jpg')
exit
