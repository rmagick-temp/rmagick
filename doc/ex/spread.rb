#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#spread method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.spread

img.write('spread.jpg')
exit
