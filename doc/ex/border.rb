#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#border method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img.border!(10,10,'rgb(255,110,140)')
img.write('border.jpg')
exit
