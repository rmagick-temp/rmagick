#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(275, 170) { self.background_color = "white" }

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
gc.text(145, 145, 'A')

gc.draw(imgl)

# Blow up the image so we can
# easily see the image, then
# crop to a representative portion.
imgl.resize!(3).crop!(235,270, 365,180)
imgl.page = Magick::Rectangle.new(365, 180)
imgl.border!(1,1,"black")
imgl.write("text_antialias.gif")
exit

