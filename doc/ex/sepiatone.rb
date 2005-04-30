#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first
sepiatone = img.sepiatone(MaxRGB * 0.8)
sepiatone.write('sepiatone.jpg')

