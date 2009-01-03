#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(250, 250, Magick::HatchFill.new('white','LightCyan2'))

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
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.fill('black')
gc.stroke('transparent')
gc.text(132,125, "'125,125'")
gc.text(32,125, "'25,125'")
gc.draw(imgl)

imgl.border!(1,1, 'lightcyan2')
imgl.write("circle.gif")
