#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#charcoal method

einstein = Magick::Image.read('images/Einstein.jpg').first
einstein = einstein.scale(250.0/einstein.rows)
einstein = einstein.charcoal
einstein.write('charcoal.jpg')
#einstein.display
exit
