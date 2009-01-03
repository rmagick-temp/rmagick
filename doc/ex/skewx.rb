#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(250, 250, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new

# Move the origin to the center.
gc.translate(125, 125)
max_x = imgl.columns/2
max_y = imgl.rows/2 - 5

# Skew x 30 degrees
gc.skewx(30)

# Draw down-pointing arrow
gc.fill('red')
gc.stroke('red')
gc.stroke_width(3)
gc.line(0, -max_y,  0, max_y)
gc.line(0,  max_y,  7, max_y-7)
gc.line(0,  max_y, -7, max_y-7)

# Draw right-pointing arrow
gc.stroke('gray50')
gc.stroke_width(1)
gc.line(-max_x, 0, max_x,    0)
gc.line( max_x, 0, max_x-5, -5)
gc.line( max_x, 0, max_x-5,  5)

gc.draw(imgl)

# Add labels using "normal" skew
gc = Magick::Draw.new
gc.pointsize(14)
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.stroke('transparent')
gc.gravity(Magick::CenterGravity)
gc.text(10, -10, "'0,0'")
gc.gravity(Magick::EastGravity)
gc.text(10, 10, "'+x'")
gc.gravity(Magick::SouthGravity)
gc.text(0, 20, "'+y'")

gc.draw(imgl)

imgl.border!(1,1, "lightcyan2")

imgl.write("skewx.gif")

