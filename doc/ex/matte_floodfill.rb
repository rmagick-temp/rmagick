#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.new(200,200)
img.compression = LZWCompression

bg = Image.read('plasma:fractal') { self.size = '200x200' }
bg[0].matte = false

gc = Draw.new
gc.stroke_width(2)
gc.stroke('black')
gc.fill('white')
gc.roundrectangle(0, 0, 199, 199, 8, 8)

gc.fill('yellow')
gc.stroke('red')
gc.circle(100, 100, 100,  25)
gc.draw(img)

img.write('matte_floodfill_before.gif')

img.fuzz = 100
img = img.matte_floodfill(100, 100)

# Composite the image over a nice bright background
# so that the transparent pixels will be obvious.
img = bg[0].composite(img, CenterGravity, OverCompositeOp)

img.write('matte_floodfill_after.gif')
exit

