#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#modulate method

blonde = Magick::Image.read('images/Blonde_with_dog.jpg').first
blonde.scale!(300.0/blonde.rows)

after = blonde.modulate(0.85)

# Show before and after.
after.crop!(after.columns/2, 0, after.columns/2, after.rows)
result = blonde.composite(after, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a line down the middle.
line = Magick::Draw.new
line.line(result.columns/2, 0, result.columns/2, result.rows)
line.draw(result)

#result.display
result.write('modulate.jpg')
exit
