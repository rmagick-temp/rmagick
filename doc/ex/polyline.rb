#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(300, 100, Magick::HatchFill.new('white','lightcyan2'))

points = [12,93.75,   37,93.75,  37,81.25,  62,81.25,
          62,93.75,   87,93.75,  87,62,    112,62,    112,93.75,
          137,93.75, 137,43.75, 162,43.75, 162,93.75,
          187,93.75, 187,25,    212,25,    212,93.75, 237,93.75,
          237,6.25,  262,6.25,  262,93.75, 287,93.75]

gc = Magick::Draw.new
gc.stroke('blue').stroke_width(3)
gc.fill_opacity(0)
gc.polyline(*points)

gc.draw(imgl)

imgl.border!(1,1, "lightcyan2")

imgl.write("polyline.gif")

