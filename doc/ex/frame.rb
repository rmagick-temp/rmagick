#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#frame method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

jj.matte_color="VioletRed4"
jj = jj.frame


#jj.display
jj.write('frame.jpg')
exit
