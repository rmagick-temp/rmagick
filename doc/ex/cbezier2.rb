#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(470, 150, Magick::HatchFill.new('white','lightcyan2'))
gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(3)
gc.fill_opacity(0)
gc.bezier(25,125, 100,25, 400,25, 325,125)

# Draw circles around end points
gc.fill_opacity(0)
gc.stroke('gray50').stroke_width(1)
gc.circle(25,125, 28, 128)
gc.circle(325,125, 328, 123)

# Draw filled circles around control points
gc.line(25,125, 100,25)
gc.line(325,125, 400,25)
gc.fill_opacity(1)
gc.fill('gray50')
gc.circle(100,25, 103,28)
gc.circle(400,25, 403,28)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(34,125, "'25,125'")
gc.text(107,25, "'100,25'")
gc.text(335,125, "'325,125'")
gc.text(405,25, "'400,25'")

gc.draw(imgl)
imgl.border!(1,1, "lightcyan2")
imgl.write('cbezier2.gif')
exit(0)

