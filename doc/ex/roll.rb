#! /usr/local/bin/ruby -w
require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.roll(img.columns/4, img.rows/4)

img.write('roll.jpg')
exit
