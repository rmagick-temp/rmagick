#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(250, 250, Magick::HatchFill.new('LightCyan'))

gc = Magick::Draw.new

# Draw the circle
gc.fill_opacity(0)
gc.stroke('red').stroke_width(3)
gc.circle(125,125, 25, 125)

# Draw a little gray circle on the perimeter
gc.stroke_width(1)
gc.stroke('gray50')
gc.circle(25,125,28,128)

# Draw a dot at the center
gc.fill_opacity(1)
gc.circle(125,125, 128,128)

# Annotate the dots
gc.fill('black')
gc.stroke('transparent')
gc.text(132,125, "'125,125'")
gc.text(32,125, "'25,125'")
gc.draw(i)
i.write("circle.gif")
