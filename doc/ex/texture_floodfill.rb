#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#texture_floodfill method
# This example is nearly identical to the color_floodfill example.

before = Magick::Image.new(200,200) { self.background_color = 'white' }
before.border!(1,1,'black')

circle = Magick::Draw.new
circle.fill('transparent')
circle.stroke_width(2)
circle.stroke('black')
circle.circle(100,100,180,100)
circle.fill('plum1')
circle.stroke('transparent')
circle.circle( 60,100, 40,100)
circle.circle(140,100,120,100)
circle.circle(100, 60,100, 40)
circle.circle(100,140,100,120)
circle.draw(before)

before.compression = Magick::LZWCompression
before.write('texture_floodfill_before.gif')

hat = Magick::Image.read('images/Flower_Hat.jpg').first
hat.resize!(0.3)
before.fuzz = 25
after = before.texture_floodfill(100,100, hat)

after.write('texture_floodfill_after.gif')
exit
