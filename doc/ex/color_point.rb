#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#color_point method

f = Magick::Image.new(100,100) { self.background_color = 'white' }
red = Magick::Pixel.from_color('red')
f = f.color_point(50,50, red)

circle = Magick::Draw.new
circle.stroke('gray50')
circle.fill_opacity(0)
circle.circle(50,50,55,55)
circle.draw(f)

f = f.border(1,1,'black')
#f.display
f.write('color_point.gif')
exit
