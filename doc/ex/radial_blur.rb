#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#radial_blur method

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Make a blurry copy.
img = img.radial_blur(10.0)

img.write('radial_blur.jpg')
exit
