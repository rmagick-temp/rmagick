#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(360, 250, Magick::HatchFill.new('white','LightCyan2'))

gc = Magick::Draw.new

gc.fill_opacity(0)
gc.stroke('red').stroke_width(3)

# Draw ellipse
gc.ellipse(180,125, 150,75, 0, 270)

# Draw horizontal width line
gc.stroke('gray50').stroke_width(1)
gc.line(180-150, 125, 180, 125)

# Draw vertical height line
gc.line(180, 125-75, 180, 125)
gc.fill_opacity(0)

# Draw arcStart circle
gc.circle(180+150, 125, 180+150+3, 125+3)

# Draw arcEnd circle
gc.circle(180, 125-75, 180+3, 125-75+3)

# Annotate
gc.fill_opacity(1)
gc.circle(180,125, 183,128)
gc.fill('black')
gc.stroke('transparent')
gc.text(187,125, "'180,125'")
gc.text(253, 118, "'Start 0 degrees'")
gc.text(187, 50, "'End 270 degrees'")
gc.text(120, 100, "'Height=75'")
gc.text(85, 140, "'Width=150'")
gc.draw(imgl)

imgl.border!(1,1, "LightCyan2")

imgl.write("ellipse.gif")
