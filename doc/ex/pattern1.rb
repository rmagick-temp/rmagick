#! /usr/local/bin/ruby -w

require 'RMagick'

gc = Magick::Draw.new
gc.pattern('checkerboard', 0, 0, 16, 16) {
    gc.fill('red')
    gc.stroke('black')
    gc.rectangle(0,0,7,7)
    gc.rectangle(8,8,15,15)
    gc.fill('black')
    gc.rectangle(8,0,15,7)
    gc.rectangle(0,8,7,15)
}

gc.stroke('checkerboard')
gc.stroke_width(16)
gc.fill_opacity(0)
gc.ellipse(150, 75, 130, 60, 0, 360)

i = Magick::ImageList.new
i.new_image(300, 150, Magick::HatchFill.new('white','gray90',8))
gc.draw(i)

#i.display
i.write('pattern1.gif')
exit
