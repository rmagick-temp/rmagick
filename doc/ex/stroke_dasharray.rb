#! /usr/local/bin/ruby -w

require 'RMagick'

i = Magick::ImageList.new
i.new_image(500,180) {self.background_color = "white"}
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
gc.stroke('red')
gc.line(10, 140, 490, 140)

gc.stroke('thistle')
gc.stroke_width(1)
gc.stroke_dasharray()
gc.rectangle(0,0, 499,179)

gc.fill('black')
gc.stroke('transparent')

gc.text(150, 40, "'draw.stroke_dasharray(30, 10, 10, 10)'")
gc.text(165, 100, "'draw.stroke_dasharray(5, 10, 5)'")
gc.text(175, 115, "'draw.stroke_dashoffset(10)'")
gc.text(170, 160, "'draw.stroke_dasharray(10, 10)'")

gc.draw(i)

#i.display
i.write("stroke_dasharray.gif")
exit

