#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#texture_fill_to_border method
# This example is nearly identical to the color_fill_to_border example.

before = Magick::Image.new(200,200) {
    self.background_color = 'white'
    self.border_color = 'black'
    }
before = before.border(2,2,'black')

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

rose = Magick::Image.read('rose:').first
after = before.texture_fill_to_border(100,100, rose)

after.crop!(after.columns/2, 0, after.columns/2, after.rows)
result = before.composite(after, Magick::EastGravity, Magick::OverCompositeOp)

line = Magick::Draw.new
line.line(result.columns/2, 0, result.columns/2, result.rows)
line.draw(result)

#result.display
result.write('texture_fill_to_border.gif')
exit
