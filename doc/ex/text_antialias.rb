#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(275, 170) { self.background_color = "white" }

gc = Magick::Draw.new
gc.fill('black')
gc.stroke('transparent')

gc.font_family('courier')
gc.font_weight(Magick::BoldWeight)
gc.pointsize(200)

# Turn off antialiasing
gc.text_antialias(false)
gc.text(15, 145, 'A')

# Turn it back on
gc.text_antialias(true)
gc.text(145, 145, 'B')

gc.draw(i)

# Blow up the image so we can
# easily see the image, then
# crop to a representative portion.
i.resize!(3).crop!(225,250, 575,200)

gc = Magick::Draw.new
gc.stroke('thistle')
gc.fill_opacity(0)
gc.stroke_width(1)
gc.stroke_dasharray()
gc.rectangle(0,0, i.columns-1,i.rows-1)
gc.draw(i)

#i.display
i.write("text_antialias.jpg")
exit

