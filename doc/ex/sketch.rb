#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Convert to grayscale
img = img.quantize(256, Magick::GRAYColorspace)

# Apply histogram equalization
img = img.equalize

# Sketch
img = img.sketch(0, 20, 135)

img.write('sketch.jpg')

