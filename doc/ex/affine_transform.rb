#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#affine_transform method

img = Magick::Image.read("images/Flower_Hat.jpg").first

# Construct a simple affine matrix
flipflop = Magick::AffineMatrix.new(1, Math::PI/6, Math::PI/6, 1, 0, 0)

# Apply the transform
img = img.affine_transform(flipflop)

# Scale the image, make the background transparent,
# and write it to a JPEG file.
img.scale!(250.0/img.rows)
img = img.matte_replace(0,0)
img.write("affine_transform.jpg")

exit
