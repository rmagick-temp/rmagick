#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(500, 350, Magick::HatchFill.new('seashell'))
gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(2)
gc.fill_opacity(0)
gc.bezier(100,200, 100,100, 400, 100, 400,200)

# Draw circles around endpoints
gc.fill_opacity(0)
gc.stroke('gray50').stroke_width(1)
gc.circle(100,200, 103, 203)
gc.circle(400,200, 403, 203)

# Draw filled circles around control points
gc.line(100,200, 100,100)
gc.line(400,200, 400,100)
gc.fill_opacity(1)
gc.fill('gray50')
gc.circle(100,100, 103,103)
gc.circle(400,100, 403,103)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(107,200, "'100,200'")
gc.text(107,100, "'100,100'")
gc.text(407,200, "'400,200'")
gc.text(407,100, "'400,100'")
gc.draw(i)
i.write('cbezier1.gif')
exit(0)

