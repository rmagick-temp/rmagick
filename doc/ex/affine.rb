#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the affine primitive. Transform the
# coordinate space to put the origin in the lower
# left corner.

imgl = Magick::ImageList.new
imgl.new_image 200, 200, Magick::HatchFill.new('white','lightcyan2')

gc = Magick::Draw.new

max_x = imgl.columns
max_y = imgl.rows

# Translate the y origin to the bottom of the window.
# Invert the y points by scaling by -1. Combine the
# two operations using the affine method. That is, the
# affine method is equivalent to:
#       gc.translate 0, max_y
#       gc.scale 1, -1
gc.affine(1, 0, 0, -1, 0, max_y)
gc.stroke('gray50')
gc.fill('gray50')
gc.stroke_width(1)

# Draw up-pointing arrow.
gc.polyline(10, 10, 10, max_y-10, 5, max_y-15, 15, max_y-15, 10, max_y-10)

# Draw right-pointing arrow
gc.polyline(10, 10, max_x-10, 10, max_x-15, 5, max_x-15, 15, max_x-10, 10)

gc.draw(imgl)

# Add labels. Use a different graphics context with a "normal"
# coordinate system so the text isn't inverted.
text_gc = Magick::Draw.new
text_gc.pointsize(14)
text_gc.font_weight(Magick::NormalWeight)
text_gc.stroke('transparent')
text_gc.text(15, max_y-15, "'0,0'")
text_gc.text(max_x-20, max_y-16, "'+x'")
text_gc.text(12, 15, "'+y'")
text_gc.draw(imgl)

imgl.border!(1, 1, "lightcyan2")

imgl.write("affine.gif")
