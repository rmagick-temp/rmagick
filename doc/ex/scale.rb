#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(250, 250, Magick::HatchFill.new('light cyan'))

scale1 = Magick::Draw.new
scale1.stroke('red')
scale1.stroke_width(2)

# Draw y axis
scale1.line(0, 0, 0, 249)
scale1.line(0, 249, 10, 239)

# Draw x axis
scale1.line(0, 0, 249, 0)
scale1.line(249, 0, 239, 10)

# Add labels
scale1.fill('black')
scale1.stroke('transparent')
scale1.text(8, 15, "'0,0'")
scale1.text(233, 16, "x")
scale1.text(12, 237, "y")

# Draw circle at default scale.
scale1.stroke('black')
scale1.stroke_width(1)
scale1.fill('white')
scale1.circle(124.5, 124.5, 124.5, 149.5)

# Label the circle.
scale1.stroke('transparent')
scale1.fill('black')
scale1.text(108, 128.5, "'1.0,1.0'")

scale1.draw(i)

# Make a new draw object for the scale operation
scale2 = Magick::Draw.new

# Change the scale, draw the circle
# and the label at the same coordinates.
scale2.scale(1.5, 1.25)
scale2.stroke('black')
scale2.fill('yellow')
scale2.circle(124.5, 124.5, 124.5, 149.5)
scale2.stroke('transparent')
scale2.fill('black')
scale2.text(108, 128.5, "'1.5,1.25'")

scale2.draw(i)

#i.display
i.write("scale.gif")
exit
