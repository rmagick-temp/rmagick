#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(250, 250, Magick::HatchFill.new('LightCyan'))

gc = Magick::Draw.new

# Draw 3 lines in 3 colors.
# Make each line 2 pixels wide.
gc.stroke('red')
gc.stroke_width(2)
gc.line(50,50, 50,200)
gc.stroke('blue')
gc.line(50,200, 200,200)
gc.stroke('green')
gc.line(200,200, 50,50)

# Identify the endpoints with gray circles
gc.stroke('gray50')
gc.stroke_width(1)
gc.fill_opacity(0)
gc.circle(50,50, 53,53)
gc.circle(50,200, 53,203)
gc.circle(200,200, 203,203)

# Annotate the endpoints
gc.fill('black')
gc.stroke('transparent')
gc.text(30,40, "'50,50'")
gc.text(30,220, "'50,200'")
gc.text(180, 220, "'200,200'")

gc.draw(i)
#i.display
i.write("line.gif")
