#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#colorize method by converting
# a full-color image to "sepia-tone"

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Convert the color image to monochrome
mono = img.quantize(256, Magick::GRAYColorspace)

# Colorize with a 30% blend of a brownish-orange color
colorized = mono.colorize(0.30, 0.30, 0.30, '#cc9933')

colorized.write('colorize.jpg')
exit
