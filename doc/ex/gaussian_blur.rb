#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#gaussian_blur method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.gaussian_blur(0.0, 3.0)

img.write('gaussian_blur.jpg')
exit
