#! /usr/local/bin/ruby -w

require 'RMagick'

# Demonstrate the Image#vignette method.
# Compare this example with the vignette.rb script in the examples directory.

img = Magick::Image.read('images/Flower_Hat.jpg').first
vignette = img.vignette
vignette.write('vignette.jpg')

exit
