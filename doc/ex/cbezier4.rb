#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(390, 360, Magick::HatchFill.new('white','lightcyan2'))
gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(3)
gc.fill_opacity(0)
gc.bezier(20,180, 20,30, 320, 330, 320,180)

# Draw circles around endpoints
gc.fill_opacity(0)
gc.stroke('gray50').stroke_width(1)
gc.circle(20,180, 23, 183)
gc.circle(320,180, 323, 183)

# Draw filled circles around control points
gc.line(20,180, 20,30)
gc.line(320,180, 320,330)
gc.fill_opacity(1)
gc.fill('gray50')
gc.circle(20,30, 23,33)
gc.circle(320,330, 323,333)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(29,180, "'20,180'")
gc.text(29,33,  "'20,30'")
gc.text(329,330, "'320,330'")
gc.text(329,180, "'320,180'")

gc.draw(imgl)
imgl.border!(1,1, "lightcyan2")

imgl.write('cbezier4.gif')
exit(0)

