#! /usr/local/bin/ruby -w

require 'RMagick'

gc = Magick::Draw.new
gc.pattern('checkerboard', 0, 0, 16, 16) {
    gc.fill('red')
    gc.stroke('transparent')
    gc.rectangle(0,0,7,7)
    gc.rectangle(8,8,15,15)
    gc.fill('rgb(0,255,0)')
    gc.rectangle(8,0,15,7)
    gc.rectangle(0,8,7,15)
}

gc.stroke('checkerboard')
gc.stroke_width(16)
gc.fill_opacity(0)
gc.ellipse(150, 75, 130, 60, 0, 360)

imgl = Magick::ImageList.new
imgl.new_image(300, 150, Magick::HatchFill.new('white','lightcyan2',8))
gc.draw(imgl)

imgl.border!(1,1, "lightcyan2")

imgl.write('pattern1.gif')
exit
