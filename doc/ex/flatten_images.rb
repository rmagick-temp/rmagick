#! /usr/local/bin/ruby -w

# Demonstrate flatten_images method. Create an image with a drop-shadow effect.

require 'RMagick'

RMagick = 'RMagick'

i = Magick::ImageList.new

# Create a background image with a gradient fill
i.new_image(200, 100, Magick::GradientFill.new(100,50, 100, 50, "khaki1", "turquoise"))

# Create a transparent image for the text shadow
i.new_image(200, 100) { self.background_color = 'transparent' }
primitives = Magick::Draw.new
primitives.annotate i, 0, 0, 2, 2, RMagick do
    self.pointsize = 32
    self.fill = "gray50"
    self.gravity = Magick::CenterGravity
    end

# Create another transparent image for the text itself
i.new_image(200, 100) { self.background_color = 'transparent' }
primitives = Magick::Draw.new
primitives.annotate i, 0, 0, -2, -2, RMagick do
    self.pointsize = 32
    self.fill = "red"
    self.stroke = "black"
    self.gravity = Magick::CenterGravity
    end

# Flatten all 3 into a single image.
# i.display
i.flatten_images.write "flatten_images.gif"
exit
