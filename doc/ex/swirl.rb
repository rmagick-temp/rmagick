#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#swirl method

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Make an animated image.
animation = Magick::ImageList.new
animation << img.copy
30.step(360,45) { |degrees| animation << img.swirl(degrees) }

animation.delay = 20
animation.iterations = 10000

animation.write('swirl.gif')
exit
