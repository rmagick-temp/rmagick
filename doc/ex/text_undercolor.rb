#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Draw#text_undercolor method

canvas = Magick::Image.new(250, 100)  {self.background_color ='white'}

gc = Magick::Draw.new

gc.stroke('transparent')
gc.pointsize(16)
gc.gravity(Magick::CenterGravity)

gc.text_undercolor('cyan')
gc.text(0,-20,"text_undercolor('cyan')")

gc.text_undercolor('yellow')
gc.text(0,0,"text_undercolor('yellow')")

gc.text_undercolor('pink')
gc.text(0,20,"text_undercolor('pink')")

gc.draw(canvas)

canvas.write('text_undercolor.gif')
exit
