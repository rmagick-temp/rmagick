#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the affine primitive. Transform the
# coordinate space to put the origin in the lower
# left corner.

i = Magick::ImageList.new
i.new_image 250, 250, Magick::HatchFill.new('LightCyan')

primitive = Magick::Draw.new

max_x = i.columns-1
max_y = i.rows-1

# Translate the y origin to the bottom of the window.
# Invert the y points by scaling by -1. Combine the
# two operations using the affine method. That is, the
# affine method is equivalent to:
#       primitive.translate 0, max_y
#       primitive.scale 1, -1
primitive.affine 1, 0, 0, -1, 0, max_y
# Draw up-pointing arrow.
primitive.stroke 'red'
primitive.stroke_width 3
primitive.line 1, 0, 1, max_y
primitive.line 1, max_y, 10, max_y-10

# Draw right-pointing arrow
primitive.line 0, 1, max_x, 1
primitive.line max_x, 1, max_x-10, 10
primitive.draw i

# Add labels. Use a different graphics context with a "normal"
# coordinate system so the text isn't inverted.
text_primitive = Magick::Draw.new
text_primitive.pointsize 14
text_primitive.font_weight Magick::BoldWeight
text_primitive.stroke 'transparent'

text_primitive.text 12, max_y-12, "'0,0'"
text_primitive.text max_x-20, max_y-16, "'+x'"
text_primitive.text 12, 15, "'+y'"
text_primitive.draw i

#i.display
i.write "affine.gif"
