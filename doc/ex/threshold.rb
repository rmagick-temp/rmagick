#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#threshold method

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Use a threshold of 55% of QuantumRange.
img = img.threshold(Magick::QuantumRange*0.55)

#img.display
img.write('threshold.jpg')
exit
