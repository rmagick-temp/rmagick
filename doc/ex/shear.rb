#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#shear method.

img = Magick::Image.read("images/Flower_Hat.jpg").first
img = img.shear(-30,-30)
#img.display
img.write('shear.jpg')
exit
