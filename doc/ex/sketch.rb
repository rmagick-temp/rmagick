#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first

# Convert to grayscale
sketch = img.quantize(256, Magick::GRAYColorspace)

# Apply histogram equalization
sketch = sketch.equalize

# Sketch, then dissolve 25% of the original back in
sketch = sketch.sketch(0, 10, 135)
img = img.dissolve(sketch, 0.75, 0.25)

img.write('sketch.jpg')

