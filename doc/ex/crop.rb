#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#crop method

lighthouse = Magick::Image.read('images/Lighthouse.jpg')[0]
lighthouse.scale!(250.0/lighthouse.rows)

# Crop the specified rectangle out of the lighthouse.
chopped = lighthouse.crop(78, 66, 90, 110)

# Go back to the original and highlight the area
# corresponding to the retained rectangle.
rect = Magick::Draw.new
rect.stroke('transparent')
rect.fill('white')
rect.fill_opacity(0.25)
rect.rectangle(78, 66, 90+78, 110+66)
rect.draw(lighthouse)

# Create a image to use as a background for
# the "before & after" images.
bg = Magick::Image.new(lighthouse.columns*2, lighthouse.rows) {
    self.background_color = 'black'
    }

# Composite the "before" image on the left side
# and the "after" (chopped) image on the right.
bg = bg.composite(lighthouse, Magick::WestGravity, Magick::OverCompositeOp)
bg = bg.composite(chopped, lighthouse.columns+78, 66, Magick::OverCompositeOp)

bg.write('crop.jpg')
#bg.display
exit
