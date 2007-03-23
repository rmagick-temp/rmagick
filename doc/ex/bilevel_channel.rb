#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first
result = img.bilevel_channel(2*Magick::QuantumRange/3, Magick::RedChannel)
result.write('bilevel_channel.jpg')
exit

