#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#chop method

img = Magick::Image.read('images/Flower_Hat.jpg')[0]

# Chop the specified rectangle out of the img.
chopped = img.chop(0, img.rows/2, img.columns/2, img.rows)

# Make a "before" image by highlighting the chopped area.
gc = Magick::Draw.new
gc.fill('white')
gc.stroke('transparent')
gc.fill_opacity(0.25)
gc.rectangle(0, img.rows/2, img.columns/2, img.rows)
gc.draw(img)

img.write('chop_before.jpg')

# Create a image to use as a background for
# the after image. Make the chopped image the
# same size as before the chop.
bg = Magick::Image.new(img.columns, img.rows)

chopped = bg.composite(chopped, Magick::NorthEastGravity, Magick::OverCompositeOp)

chopped.write('chop_after.jpg')
exit
