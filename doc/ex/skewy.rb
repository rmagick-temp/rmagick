#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(250, 250, Magick::HatchFill.new('light cyan'))

gc = Magick::Draw.new

# Move the origin to the center.
gc.translate(124.5, 124.5)
max_x = (i.columns-1)/2
max_y = (i.rows-1)/2

# Skew y 30 degrees
gc.skewy(30)

# Draw down-pointing arrow
gc.fill('black')
gc.line(0, -max_y,   0, max_y)
gc.line(0,  max_y,  10, max_y-10)
gc.line(0,  max_y, -10, max_y-10)

# Draw right-pointing arrow
gc.fill('red')
gc.line(-max_x, 0, max_x,     0)
gc.line( max_x, 0, max_x-10, -10)
gc.line( max_x, 0, max_x-10,  10)

# Add labels
gc.fill('black')
gc.text(8, 15, "'0,0'")
gc.text(110, 16, "x")
gc.text(12, 115, "y")

gc.draw(i)

#i.display
i.write("skewy.gif")
