#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(300, 200, Magick::HatchFill.new('white','LightCyan2'))

gc = Magick::Draw.new

gc.fill_opacity(0)
gc.stroke('red')
gc.stroke_width(3)

# Draw rounded rectangle
gc.roundrectangle(20,20, 280,180, 8, 8)

gc.stroke('gray50')
gc.stroke_width(1)

gc.circle(20,20, 23,23)
gc.circle(280,180, 283,183)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(30,35, "'20,20'")
gc.text(230, 175, "'280,180'")

gc.draw(imgl)
imgl.border!(1,1, "lightcyan2")

imgl.write("roundrect.gif")

