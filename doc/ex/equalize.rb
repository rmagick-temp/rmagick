#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#equalize method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.equalize

img.write('equalize.jpg')
exit
