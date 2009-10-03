#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#crop method

img = Magick::Image.read('images/Flower_Hat.jpg')[0]

# Crop the specified rectangle out of the img.
chopped = img.crop(23, 81, 107, 139)

# Go back to the original and highlight the area
# corresponding to the retained rectangle.
rect = Magick::Draw.new
rect.stroke('transparent')
rect.fill('white')
rect.fill_opacity(0.25)
rect.rectangle(23, 81, 107+23, 139+81)
rect.draw(img)

img.write('crop_before.png')

# Create a image to use as a background for
# the "after" image.
bg = Magick::Image.new(img.columns, img.rows) {self.background_color="none"}

# Composite the the "after" (chopped) image on the background
bg = bg.composite(chopped, 23, 81, Magick::OverCompositeOp)

bg.write('crop_after.png')

exit
