#! /usr/local/bin/ruby -w

require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(500,180, Magick::HatchFill.new('white','lightcyan2'))
gc = Magick::Draw.new

gc.stroke_width(5)
gc.fill('transparent')

gc.stroke_dasharray(30,10, 10,10)
gc.stroke('green')
gc.line(10, 20, 490, 20)

gc.stroke_dasharray(5,10,5)
gc.stroke_dashoffset(10)
gc.stroke('blue')
gc.line(10, 80, 490, 80)

gc.stroke_dasharray(10,10)
gc.stroke_dashoffset(0)
gc.stroke('red')
gc.line(10, 140, 490, 140)

gc.fill('black')
gc.stroke('transparent')

gc.gravity(Magick::CenterGravity)
gc.text( 0, -60, "'gc.stroke_dasharray(30, 10, 10, 10)'")
gc.text( 0,   0, "'gc.stroke_dasharray(5, 10, 5)'")
gc.text( 0,  12, "'gc.stroke_dashoffset(10)'")
gc.text( 0,  60, "'gc.stroke_dasharray(10, 10)'")

gc.draw(imgl)

imgl.border!(1,1,"lightcyan2")
#imgl.display
imgl.write("stroke_dasharray.gif")
exit

