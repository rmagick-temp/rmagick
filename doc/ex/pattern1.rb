#! /usr/local/bin/ruby -w

require 'RMagick'

gc = Magick::Draw.new
gc.pattern('triangles', 0, 0, 16, 16) {
    gc.fill('darkblue')
    gc.rectangle(0,0, 16,16)
    gc.fill('yellow')
    gc.stroke('red')
    gc.polygon(0,0, 8,16, 16,0, 0,0)
}

gc.stroke('triangles')
gc.stroke_width(16)
gc.fill('none')
gc.ellipse(150, 75, 130, 60, 0, 360)

img = Magick::Image.new(300, 150, Magick::HatchFill.new('white','lightcyan2',8))
gc.draw(img)

img.border!(1,1, "lightcyan2")

img.write('pattern1.gif')
exit
