#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#threshold method

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Use a threshold of 55% of MaxRGB.
img = img.threshold(Magick::MaxRGB*0.55)

#img.display
img.write('threshold.jpg')
exit
