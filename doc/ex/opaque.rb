#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Demonstrate the Image#opaque method.

img = Image.read('images/Flower_Hat.jpg').first

# Allow non-exact matches
img.fuzz = 25
img = img.opaque('white', 'red')

img.write('opaque.jpg')

