#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(290, 200, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new
gc.stroke('blue').stroke_width(3)

# Draw red 5-pointed star
gc.fill('red')
gc.polygon( 75,37.5,     89.5,80.5, 134.5,80.5,   98.5,107.5,
           111.5,150.5,  75,125,    38.5,150.5,   51.5,107.5,
           15.5,80.5,    60.5,80.5)
# Draw green hexagon
gc.fill('lime')
gc.polygon(225,37.5,  279,68.75, 279,131.25,
           225,162.5, 171,131.3, 171,68.75)

gc.draw(imgl)
imgl.border!(1,1, "lightcyan2")

imgl.write("polygon.gif")

