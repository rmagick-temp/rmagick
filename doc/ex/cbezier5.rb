#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(500, 300, Magick::HatchFill.new('seashell'))
gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(3)
gc.fill_opacity(0)
gc.bezier(100,200, 175,100, 325, 100, 400,200)

# Draw circles around endpoints
gc.fill_opacity(0)
gc.stroke('gray50').stroke_width(1)
gc.circle(100,200, 103, 203)
gc.circle(400,200, 403, 203)

# Draw filled circles around control points
gc.line(100,200, 175,100)
gc.line(400,200, 325,100)
gc.fill_opacity(1)
gc.fill('gray50')
gc.circle(175,100, 178,103)
gc.circle(325,100, 328,103)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(109,200, "'100,200'")
gc.text(184,100,  "'175,100'")
gc.text(334,100, "'325,100'")
gc.text(409,200, "'400,200'")

gc.draw(i)
i.write('cbezier5.gif')
exit(0)

