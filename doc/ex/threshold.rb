#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#threshold method

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

# Use a threshold of 40% of MaxRGB.
jj = jj.threshold(Magick::MaxRGB*0.40)

#jj.display
jj.write('threshold.jpg')
exit
