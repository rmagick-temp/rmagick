#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#texture_fill_to_border method
# This example is nearly identical to the color_fill_to_border example.

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

before.compression = Magick::LZWCompression
before.write('texture_fill_to_border_before.gif')

hat = Magick::Image.read('images/Flower_Hat.jpg').first
hat.resize!(0.3)
after = before.texture_fill_to_border(100,100, hat)

after.write('texture_fill_to_border_after.gif')
exit
