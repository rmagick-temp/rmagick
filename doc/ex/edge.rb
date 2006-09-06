#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#edge method

img = Magick::Image.read('images/Flower_Hat.jpg').first
img = img.quantize(256, Magick::GRAYColorspace)
img = img.edge(5)

img.write('edge.jpg')
exit
