#! /usr/local/bin/ruby -w
require 'RMagick'

# Add a drop shadow to a text string

Rows = 110
Cols = 435

ex = Magick::Image.new(Cols, Rows)

# Create a Draw object to draw the text with. Most of the text
# attributes are shared between the shadow and the foreground.

text = Magick::Draw.new
text.gravity = Magick::CenterGravity
text.pointsize = 70
text.font_weight = Magick::BoldWeight
text.font_style = Magick::ItalicStyle
text.stroke = 'transparent'

# Draw the shadow text first. The color is a very light gray.
# Position the text to the right and down.
text.annotate(ex, 0,0,3,3, 'RMagick') {
    self.fill = 'gray70'
    }

# Blur the shadow.
ex = ex.gaussian_blur(0,5)

# Add the foreground text in solid black. Position it
# to the left and up from the shadow text.
text.annotate(ex, 0,0,-3,-3, 'RMagick') {
    self.fill = 'black'
    }

#ex.display
ex.write('drop_shadow.gif')
exit
