#! /usr/local/bin/ruby -w
require 'RMagick'

blonde = Magick::Image.read('images/Blonde_with_dog.jpg').first
blonde.resize!(250.0/blonde.rows)

blonde = blonde.roll(blonde.columns/4, blonde.rows/4)
#blonde.display
blonde.write('roll.jpg')
exit
