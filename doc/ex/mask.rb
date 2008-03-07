#! /usr/local/bin/ruby -w

require 'RMagick'

# This example demonstrates the mask attribute. The mask image must
# be the same size as the image being masked. Since this mask image does
# not have an alpha channel, the intensity of each pixel is used to define the
# mask. White pixels are more intense than black pixels, so the area of the
# image masked by white pixels will remain unchanged, while the area of the
# image masked by black pixels is affected by any transformations.

# In this example the mask is simply the words "Flower Hat" in black text
# positioned near the bottom of the white clip mask image.

img = Magick::Image.read("images/Flower_Hat.jpg").first
q = Magick::Image.new(img.columns, img.rows)

gc = Magick::Draw.new
gc.annotate(q, 0, 0, 0, 0, "Flower Hat") do
    gc.gravity = Magick::SouthGravity
    gc.pointsize = 36
    gc.font_weight = Magick::BoldWeight
end

# Set the matte attribute to false, indicating the absence of an alpha channel
# in the mask image. Assign the mask image to the mask= attribute of the image
# being masked.

q.matte = false
img.mask q

# Use the #level method to darken the image under the black part of the mask.

img = img.level(0, Magick::MaxRGB, 0.50)
img.write('mask.jpg')

