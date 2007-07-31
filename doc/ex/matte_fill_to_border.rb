#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.new(200,200)
img.compression = Magick::LZWCompression

bg = Magick::Image.read('plasma:fractal') { self.size = '200x200' }
bg[0].matte = false

gc = Magick::Draw.new
gc.stroke_width(2)
gc.stroke('black')
gc.fill('white')
gc.roundrectangle(0, 0, 199, 199, 8, 8)

gc.fill('black')
gc.circle(100,  45, 100,  25)
gc.circle( 45, 100,  25, 100)
gc.circle(100, 155, 100, 175)
gc.circle(155, 100, 175, 100)

gc.draw(img)

img.write('matte_fill_to_border_before.gif')

# Set the border color. Set the fuzz attribute so that
# the matte fill will fill the aliased pixels around
# the edges of the black circles.
img.border_color = 'black'
img.fuzz = 10
img = img.matte_fill_to_border(100, 100)

# Composite the image over a nice bright background
# so that the transparent pixels will be obvious.
img = bg[0].composite(img, Magick::CenterGravity, Magick::OverCompositeOp)

img.write('matte_fill_to_border_after.gif')
exit

