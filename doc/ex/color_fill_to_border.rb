#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#color_fill_to_border method

before = Magick::Image.new(200,200) {
    self.background_color = 'white'
    self.border_color = 'black'
    }
before.border!(1,1,'black')

circle = Magick::Draw.new
circle.fill('transparent')
circle.stroke('black')
circle.stroke_width(2)
circle.circle(100,100,180,100)
circle.fill('plum1')
circle.stroke('transparent')
circle.circle( 60,100, 40,100)
circle.circle(140,100,120,100)
circle.circle(100, 60,100, 40)
circle.circle(100,140,100,120)
circle.draw(before)

before.write('color_fill_to_border_before.gif')

after = before.color_fill_to_border(100,100, 'aquamarine')
after.write('color_fill_to_border_after.gif')
exit
