#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Convert to grayscale
img = img.quantize(256, Magick::GRAYColorspace)

# Apply histogram equalization
img = img.equalize

# Sketch
img = img.sketch(1, 0.5, 90)

# Add a little color
img = img.colorize(0.0, 0.15, 0.15, '#cc9933')

img.write('sketch.jpg')

