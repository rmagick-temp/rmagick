#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(600, 200) { self.background_color = 'white' }

gc = Magick::Draw.new
gc.stroke('blue').stroke_width(3)
gc.fill_opacity(0)

gc.polyline( 25,187.5,  75,187.5, 75, 162.5, 125,162.5, 125,187.5,
            175,187.5, 175,125,   225,125,   225,187.5,
            275,187.5, 275,87.5,  325, 87.5, 325,187.5,
            375,187.5, 375,50,    425, 50,   425,187.5,
            475,187.5, 475,12.5,  525, 12.5, 525,187.5,
            575,187.5)

gc.fill_opacity(0)
gc.stroke('lavender')
gc.rectangle(0,0, 599, 199)
gc.draw(i)
#i.display
i.write("polyline.gif")
