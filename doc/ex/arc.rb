#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(300, 220, Magick::HatchFill.new("seashell"))

gc = Magick::Draw.new

# Draw the arc
gc.fill_opacity(0)
gc.stroke('red').stroke_width(3)
gc.arc(130,30, 200,100, 45,90)

# Draw circles around the endpoints
gc.stroke_width(1)
gc.stroke('gray50')
gc.circle(130,30, 133,33)
gc.circle(200,100,203,103)

# Annotate
gc.fill_opacity(1)
gc.fill('black')
gc.stroke('transparent')
gc.text(137,30, "'130,30'")
gc.text(207,100, "'200,100'")
gc.draw(i)
i.write("arc.gif")
