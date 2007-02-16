#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first
result = img.posterize
result.write('posterize.jpg')
exit
