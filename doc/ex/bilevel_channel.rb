#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first

img = img.bilevel_channel(2*MaxRGB/3, RedChannel)
img.write('bilevel_channel.jpg')
exit

