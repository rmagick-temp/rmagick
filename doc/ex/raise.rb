#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Demonstrate the Image#raise method.
img = Image.read('images/Flower_Hat.jpg').first

img = img.raise

img.write('raise.jpg')
exit
