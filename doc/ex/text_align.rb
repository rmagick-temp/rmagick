#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Draw#text_align method

canvas = Magick::Image.new(200, 100)
gc = Magick::Draw.new

# Draw three samples of text, each using
# a different alignment constant. Put the
# samples on different points on the y-axis
# so we can tell them apart.
gc.stroke('transparent')
gc.pointsize(16)
gc.fill('red')
gc.text_align(Magick::RightAlign)
gc.text(100, 30,  'RightAlign')
gc.fill('blue')
gc.text_align(Magick::CenterAlign)
gc.text(100, 50, 'CenterAlign')
gc.fill('green')
gc.text_align(Magick::LeftAlign)
gc.text(100, 70, 'LeftAlign')

# Now draw lines to show the points
# to which each sample is aligned.
gc.stroke('gray50')
gc.line(100, 10, 100, 90)
gc.line( 98, 30, 102, 30)
gc.line( 98, 50, 102, 50)
gc.line( 98, 70, 102, 70)

gc.draw(canvas)
canvas.border!(1,1,'gray50')
canvas.write('text_align.gif')
exit
