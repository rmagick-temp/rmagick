#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(540, 200, Magick::HatchFill.new('white','lightcyan2'))
gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(3)
gc.fill_opacity(0)
gc.bezier(100,150, 25,50, 475, 50, 400,150)

# Draw circles around endpoints
gc.fill_opacity(0)
gc.stroke('gray50').stroke_width(1)
gc.circle(100,150, 103, 153)
gc.circle(400,150, 403, 153)
gc.line(100,150, 25,50)
gc.line(400,150, 475,50)

# Draw filled circles around control points
gc.fill_opacity(1)
gc.fill('gray50')
gc.circle(25,50, 28,53)
gc.circle(475,50, 478,53)

# Annotate
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.fill('black')
gc.stroke('transparent')
gc.text(107,150, "'100,150'")
gc.text(30,50,  "'25,50'")
gc.text(409,150, "'400,150'")
gc.text(480,50, "'475,50'")

gc.draw(imgl)
imgl.border!(1,1, "lightcyan2")
imgl.write('cbezier3.gif')
exit(0)

