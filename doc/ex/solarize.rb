#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#solarize method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.solarize(127.5)

img.write('solarize.jpg')
exit
