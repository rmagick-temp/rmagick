#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#cycle_colormap method

balloons = Magick::Image.read('images/Hot_Air_Balloons.jpg').first
balloons = balloons.quantize(256, Magick::RGBColorspace)

jump = balloons.colors / 10

animation = Magick::ImageList.new
animation[0] = balloons

5.times { animation << animation.cycle_colormap(jump) }
4.times { animation << animation.cycle_colormap(-jump) }

animation.delay=20
animation.iterations = 10000
#animation.animate
animation.write('cycle_colormap.gif')
exit
