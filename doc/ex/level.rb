#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#level method

before = Magick::Image.read('images/Jean_Jacket.jpg').first
before.scale!(250.0/before.rows)

# Allow the white-point argument to default.
# This is really a very small adjustment.
after = before.level(0,1.25)

# Make a before-and-after composite
after.crop!(after.columns/2, 0, after.columns/2, after.rows)
result = before.composite(after, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a black line between the before and after parts.
line = Magick::Draw.new
line.line(result.columns/2, 0, result.columns/2, result.rows)
line.draw(result)

#result.display
result.write('level.jpg')
exit
