#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Convert to grayscale
img = img.quantize(256, Magick::GRAYColorspace)

# Apply histogram equalization
img = img.equalize

# Sketch
begin
    img = img.sketch(0, 20, 135)
    
rescue NotImplementedError
    not_impl = Magick::Image.read('images/notimplemented.gif').first
    not_impl.resize!(img.columns, img.rows)
    img = not_impl
end

img.write('sketch.jpg')

