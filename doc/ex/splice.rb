#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the splice method.

img = Magick::Image.read('images/Flower_Hat.jpg').first
spliced_img = img.splice(img.columns/2, img.rows/2, 20, 20, 'gray80')
spliced_img.write('splice.jpg')

