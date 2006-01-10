#! /usr/local/bin/ruby -w

require 'RMagick'

# Demonstrate the Image#vignette method.
# Compare this example with the vignette.rb script in the examples directory.

img = Magick::Image.read('images/Flower_Hat.jpg').first

begin
    vignette = img.vignette
rescue NotImplementedError
    vignette = Magick::Image.read('images/notimplemented.gif').first
    vignette.resize!(img.columns, img.rows)
end

vignette.write('vignette.jpg')

exit
