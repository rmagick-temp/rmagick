#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#affine_transform method

# Construct a simple affine matrix
flipflop = Magick::AffineMatrix.new(-1, Math::PI/6, Math::PI/6, -1, 0, 0)

einstein = Magick::Image.read("images/Einstein.jpg").first

# Apply the transform
einstein = einstein.affine_transform(flipflop)

# Scale the image, make the background transparent,
# and write it to a JPEG file.
einstein.scale!(250.0/einstein.rows)
einstein = einstein.matte_replace(0,0)
einstein.write("affine_transform.jpg")
#einstein.display
exit
