#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#swirl method

dog = Magick::Image.read('images/Dog2.jpg').first
dog.scale!(250.0/dog.rows)

# Make an animated image.
animation = Magick::ImageList.new
animation << dog.copy
30.step(360,45) { |degrees| animation << dog.swirl(degrees) }

animation.delay = 20
animation.iterations = 10000
#animation.animate
animation.write('swirl.gif')
exit
