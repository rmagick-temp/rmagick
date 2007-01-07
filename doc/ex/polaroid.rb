#! /usr/local/bin/ruby -w
require 'RMagick'
require 'date'

# Demonstrate the Image#polaroid method

img = Magick::Image.read('images/Flower_Hat.jpg').first
img[:label] = "\nLosha\n" + Date.today.to_s

picture = img.polaroid

# Composite it on a white background so the result is opaque.
background = Magick::Image.new(picture.columns, picture.rows)
result = background.composite(picture, Magick::CenterGravity, Magick::OverCompositeOp)

result.write('polaroid.jpg')
