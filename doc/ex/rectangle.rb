#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(300, 200, Magick::HatchFill.new('lightcyan'))

gc = Magick::Draw.new

gc.fill_opacity(0)
gc.stroke('red')
gc.stroke_width(3)

# Draw rectangle
gc.rectangle(20,20, 280, 180)

# Outline corners
gc.stroke_width(1)
gc.stroke('gray50')
gc.circle(20,20, 23,23)
gc.circle(280, 180, 283, 183)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(30,35, "'20,20'")
gc.text(230,175, "'280,180'")

gc.draw(i)
i = i.border(1,1, '#e0ffff')
#i.display
i.write("rectangle.gif")
exit

