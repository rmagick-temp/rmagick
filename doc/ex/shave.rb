#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#shave method

img = Magick::Image.read('images/Flower_Hat.jpg').first

img = img.shave(20, 25)

# Add a border so the shaved image is the
# same size as the original image.
img.border!(20, 25, 'white')

img.write('shave.jpg')
exit
