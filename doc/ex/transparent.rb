#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#transparent method.
# Change all black pixels in the image to transparent.

before = Magick::Image.new(200,200) {
    self.background_color = 'black'
    }

circle = Magick::Draw.new
circle.fill('transparent')
circle.stroke('white')
circle.stroke_width(8)
circle.circle(100,100,180,100)
circle.fill('transparent')
circle.stroke('white')
circle.circle( 60,100, 40,100)
circle.circle(140,100,120,100)
circle.circle(100, 60,100, 40)
circle.circle(100,140,100,120)
circle.draw(before)

before.compression = Magick::LZWCompression
before.write('transparent_before.gif')

before.fuzz = 100
after = before.transparent('black', Magick::TransparentOpacity)

# Different way of reading an image - start with an imagelist.
# Use the plasma image as a background so we can see that
# the black pixels have been made transparent.
bg = Magick::ImageList.new
bg.read('plasma:purple-gold') { self.size = '200x200' }

after = bg.composite(after, Magick::CenterGravity, Magick::OverCompositeOp)
after.write('transparent_after.gif')
exit
