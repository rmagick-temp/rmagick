#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#shear method.

einstein = Magick::Image.read("images/Einstein.jpg").first
einstein.scale!(250.0/einstein.rows)
einstein = einstein.shear(-30,-30)
#einstein.display
einstein.write('shear.jpg')
exit
