#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#colorize method by converting
# a full-color image to "sepia-tone"

polynesia = Magick::Image.read('images/Polynesia.jpg').first
polynesia.scale!(300.0/polynesia.rows)

# Convert the color image to monochrome
mono = polynesia.quantize(256, Magick::GRAYColorspace)

# Colorize with a 25% blend of a brownish-orange color
colorized = mono.colorize(0.25, 0.25, 0.25, '#cc9933')

# Do the usual before-and-after composite. Draw a line down the middle.
colorized.crop!(colorized.columns/2,0,colorized.columns,colorized.rows)
polynesia = polynesia.composite(colorized, Magick::EastGravity, Magick::OverCompositeOp)

line = Magick::Draw.new
line.line(polynesia.columns/2, 0, polynesia.columns/2, polynesia.rows)
line.draw(polynesia)

#polynesia.display
polynesia.write('colorize.jpg')
exit
