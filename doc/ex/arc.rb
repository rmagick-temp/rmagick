#! /usr/local/bin/ruby -w
require 'RMagick'

TEXT_COLOR = "#a6a6b6"

i = Magick::Image.new(300, 220, Magick::HatchFill.new("white","#e0ffff"))
gc = Magick::Draw.new

# Draw the border rectangle.
gc.fill_opacity(0)
gc.stroke_width(1)
gc.stroke(TEXT_COLOR)
gc.rectangle(40, 50, 250, 180)

# Draw the circles around the rectangle corners and
# arc endpoints. All the circles have a 3-pixel radius.
gc.circle(40,   50,  40+3,  50)
gc.circle(250, 180, 250+3, 180)
gc.circle(250, 114, 250+3, 114)
gc.circle(146,  50, 146+3,  50)

# Annotate
gc.stroke('transparent')
gc.fill(TEXT_COLOR)
gc.fill_opacity(1)

# xMagick recognizes the braces as delimiters.
gc.gravity(Magick::NorthWestGravity)
gc.text(43, 45, "{40, 50}")
gc.text(195, 118, "{0 degrees}")

gc.gravity(Magick::SouthEastGravity)
gc.text(300-250, 220-185, "{250, 180}")

gc.gravity(Magick::NorthGravity)
gc.text(0, 67, "{270 degrees}")

# Draw the arc
gc.fill_opacity(0)
gc.stroke('red').stroke_width(3)
gc.arc(40, 50, 250,180, 0, 270)

# Draw on the canvas
gc.draw(i)

i.border!(1,1, "gray50")

#i.display
i.write("arc.gif")
