#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(450, 200) { self.background_color = "white" }

gc = Magick::Draw.new
gc.stroke('blue').stroke_width(3)

# Draw red 5-pointed star
gc.fill('red')
gc.polygon(125,37.5,    139.5,80.5, 184.5,80.5,  148.5,107.5,
           161.5,150.5, 125,125,    88.5,150.5,  101.5,107.5,
           65.5,80.5,   110.5,80.5)
# Draw green hexagon
gc.fill('lime')
gc.polygon(325,37.5,  379,68.75, 379,131.25,
           325,162.5, 271,131.3, 271,68.75)

# Outline image
gc.stroke('lavender')
gc.fill('transparent')
gc.rectangle(0,0, 449, 199)
gc.draw(i)
#i.display
i.write("polygon.gif")
