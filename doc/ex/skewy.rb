#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(250, 250, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new

# Move the origin to the center.
gc.translate(125, 125)
max_x = imgl.columns/2
max_y = imgl.rows/2

# Skew y 30 degrees
gc.skewy(30)

# Draw down-pointing arrow
gc.fill('gray50')
gc.line(0, -max_y,   0, max_y)
gc.line(0,  max_y,  10, max_y-10)
gc.line(0,  max_y, -10, max_y-10)

# Draw right-pointing arrow
gc.stroke('red')
gc.stroke_width(3)
gc.line(-max_x+10, 0, max_x-10,   0)
gc.line( max_x-10, 0, max_x-20, -10)
gc.line( max_x-10, 0, max_x-20,  10)

gc.draw(imgl)

# Add labels
gc = Magick::Draw.new
gc.pointsize(14)
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.stroke('transparent')
gc.gravity(Magick::CenterGravity)
gc.text(15, 0, "'0,0'")
gc.gravity(Magick::EastGravity)
gc.text(10, 0, "'+x'")
gc.gravity(Magick::SouthGravity)
gc.text(10, 20, "'+y'")
gc.draw(imgl)

imgl.border!(1,1, 'lightcyan2')
imgl.write("skewy.gif")
