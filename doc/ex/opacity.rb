#! /usr/local/bin/ruby -w
require 'RMagick'

canvas = Magick::Image.new(260, 125)
gc = Magick::Draw.new
gc.fill('black')
gc.rectangle(10,20, 250,90)

gc.stroke('blue')
gc.fill('yellow')
gc.stroke_width(10)

gc.opacity('25%')
gc.roundrectangle(20,20, 60,90, 5,5)

gc.opacity('50%')
gc.roundrectangle(80,20, 120,90, 5,5)

gc.opacity(0.75)
gc.roundrectangle(140,20, 180,90, 5,5)

gc.opacity(1)
gc.roundrectangle(200,20, 240,90, 5,5)

gc.stroke('transparent')
gc.fill('black')
gc.gravity(Magick::SouthGravity)
gc.text(-90,20, '"25%%"')
gc.text(-30,20, '"50%%"')
gc.text( 30,20, '"75%%"')
gc.text( 90,20, '"100%%"')

gc.draw(canvas)

canvas.write("opacity.gif")
exit
