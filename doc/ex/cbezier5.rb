#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(390, 150, Magick::HatchFill.new('white','lightcyan2'))
gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(3)
gc.fill_opacity(0)
gc.bezier(20,120, 95,20, 245,20, 320,120)

# Draw circles around endpoints
gc.fill_opacity(0)
gc.stroke('gray50').stroke_width(1)
gc.circle(20,120, 23, 123)
gc.circle(320,120, 323, 123)

# Draw filled circles around control points
gc.line(20,120, 95,20)
gc.line(320,120, 245,20)
gc.fill_opacity(1)
gc.fill('gray50')
gc.circle(95,20, 98,23)
gc.circle(245,20, 248,23)

# Annotate
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.fill('black')
gc.stroke('transparent')
gc.text(29,120, "'20,120'")
gc.text(104,20,  "'95,20'")
gc.text(254,20,  "'245,20'")
gc.text(329,120, "'320,120'")

gc.draw(imgl)
imgl.border!(1,1, 'lightcyan2')

imgl.write('cbezier5.gif')
exit(0)

