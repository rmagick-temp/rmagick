#! /usr/local/bin/ruby -w
require 'RMagick'

blonde = Magick::Image.read('images/Blonde_with_dog.jpg').first
blonde.resize!(250.0/blonde.rows)

blonde = blonde.wave
#blonde.display
blonde.write('wave.jpg')
exit
