#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#spread method

grandma = Magick::Image.read('images/Grandma.jpg').first
grandma.scale!(250.0/grandma.rows)

grandma = grandma.spread

#grandma.display
grandma.write('spread.jpg')
exit
