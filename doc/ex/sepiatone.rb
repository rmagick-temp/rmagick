#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first
sepiatone = img.sepiatone(Magick::QuantumRange * 0.8)
sepiatone.write('sepiatone.jpg')

