#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(250, 250, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new
gc.stroke('red')
gc.stroke_width(2)

# Move the origin to the center.
gc.translate(125, 125)
max_x = imgl.columns/2
max_y = imgl.rows/2

# Draw down-pointing arrow
gc.line(0, -max_y,   0, max_y)
gc.line(0,  max_y,  10, max_y-10)
gc.line(0,  max_y, -10, max_y-10)

# Draw right-pointing arrow
gc.line(-max_x, 0, max_x,     0)
gc.line( max_x, 0, max_x-10, -10)
gc.line( max_x, 0, max_x-10,  10)

# Add labels
gc.fill('black')
gc.stroke('transparent')
gc.text(8, 15, "'0,0'")
gc.text(110, 16, "x")
gc.text(12, 115, "y")

gc.draw(imgl)

imgl.border!(1,1, "lightcyan2")

imgl.write("translate.gif")
