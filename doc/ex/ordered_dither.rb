#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#negate method

blonde = Magick::Image.read('images/Blonde_with_dog.jpg').first
blonde.scale!(300.0/blonde.rows)

blonde = blonde.ordered_dither

#blonde.display
blonde.write('ordered_dither.jpg')
exit
