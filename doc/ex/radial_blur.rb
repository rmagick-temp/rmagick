#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#radial_blur method

img = Magick::Image.read('images/Flower_Hat.jpg').first
result = img.radial_blur(10.0)
result.write('radial_blur.jpg')
exit
