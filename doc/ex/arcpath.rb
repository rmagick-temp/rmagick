#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the "path" drawing primitive.

imgl = Magick::ImageList.new
imgl.new_image(450, 200, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new

# Draw "pie chart"
gc.fill('red')
gc.stroke('blue')
gc.stroke_width(2)
gc.path('M110,100 h-75 a75,75 0 1,0 75,-75 z')
gc.fill('yellow')
gc.path('M97.5,87.5 v-75 a75,75 0 0,0 -75,75 z')

# Draw wiggly line
gc.fill_opacity(0)
gc.stroke('#00cd00')
gc.stroke_width(3)
gc.path('M200,175 l 25,-12.5 ' +
                'a12.5,12.5 -15 0,1 25,-12.5 l 25,-12.5 ' +
                'a12.5,25   -15 0,1 25,-12.5 l 25,-12.5 ' +
                'a12.5,37.5 -15 0,1 25,-12.5 l 25,-12.5 ' +
                'a12.5,50   -15 0,1 25,-12.5 l 25,-12.5')

gc.draw imgl
imgl.border!(1,1, "lightcyan2")

imgl.write('arcpath.gif')

