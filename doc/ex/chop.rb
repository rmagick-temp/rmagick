#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#chop method

lighthouse = Magick::Image.read('images/Lighthouse.jpg')[0]
lighthouse = lighthouse.scale(250.0/lighthouse.rows)

# Chop the specified rectangle out of the lighthouse.
chopped = lighthouse.chop(0, 0, lighthouse.columns/2, lighthouse.rows/2)

# Go back to the original and draw a semi-transparent rectangle
# corresponding to the chopped rectangle.
rect = Magick::Draw.new
rect.stroke('transparent')
rect.fill_opacity(0.40)
rect.rectangle(0,0,(lighthouse.columns/2)-1, lighthouse.rows/2)
rect.draw(lighthouse)

# Create a image to use as a background for
# the "before & after" images.
bg = Magick::Image.new(lighthouse.columns+chopped.columns, lighthouse.rows) {
    self.background_color = 'gray50'
    }

# Composite the "before" image on the left side
# and the "after" (chopped) image on the right.
bg = bg.composite(lighthouse, Magick::WestGravity, Magick::OverCompositeOp)
bg = bg.composite(chopped, Magick::SouthEastGravity, Magick::OverCompositeOp)

# Draw a line between the before & after images.
line = Magick::Draw.new
line.line(lighthouse.columns, 0, lighthouse.columns, bg.rows-1)
line.draw(bg)

bg.write('chop.jpg')
#bg.display
exit
