#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#border method

ballerina = Magick::Image.read('images/Ballerina.jpg').first
ballerina.scale!(250.0/ballerina.rows)
bordered = ballerina.border(10,10,'HotPink2')
#bordered.display
bordered.write('border.jpg')
exit
