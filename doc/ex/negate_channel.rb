#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#negate_channel method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.negate_channel(false, Magick::GreenChannel)

img.write('negate_channel.jpg')
exit
