#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#segment method.

grandma = Magick::Image.read('images/Grandma.jpg').first
grandma.resize!(250.0/grandma.rows)
grandma = grandma.segment(Magick::YUVColorspace)
#grandma.display
grandma.write('segment.jpg')
exit
