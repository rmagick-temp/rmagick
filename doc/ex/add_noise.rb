#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#add_noise method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.add_noise(Magick::MultiplicativeGaussianNoise)

img.write("add_noise.jpg")
exit
