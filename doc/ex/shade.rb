#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate Image#shade

ballerina = Magick::Image.read('images/Ballerina.jpg').first
ballerina.crop!(0,116,416,468)
ballerina.resize!(250.0/ballerina.rows)
ballerina = ballerina.shade(true)
#ballerina.display
ballerina.write('shade.jpg')
exit
