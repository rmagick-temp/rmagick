#! /usr/local/bin/ruby -w
require 'RMagick'

# Add a drop shadow to a text string. This example
# uses a 3-image animation to show each step of the
# process

Rows = 60
Cols = 250
Text = 'Ruby rocks!'

# This imagelist will contain the animation frames
anim = Magick::ImageList.new

ex = Magick::Image.new(Cols, Rows)

# Create a Draw object to draw the text with. Most of the text
# attributes are shared between the shadow and the foreground.

text = Magick::Draw.new
text.gravity = Magick::CenterGravity
text.pointsize = 36
text.font_weight = Magick::BoldWeight
text.font_style = Magick::ItalicStyle
text.stroke = 'transparent'

# Draw the shadow text first. The color is a very light gray.
# Position the text to the right and down.
text.annotate(ex, 0,0,2,2, Text) {
    self.fill = 'gray60'
    }

# Save the first frame of the animation.
anim << ex.copy

# Blur the shadow. Save a copy of the image as the 2nd frame.
ex = ex.blur_image(0,3)
anim << ex.copy

# Add the foreground text in solid black. Position it
# to the left and up from the shadow text.
text.annotate(ex, 0,0,-1,-1, Text) {
    self.fill = 'maroon'
    }

# Save yet another copy of the image as the 3rd frame.
anim << ex.copy

# Set the delay between frames to 1 second.
anim.delay = 100

# Set the delay after the last frame to 3 seconds.
anim.cur_image.delay = 300

# Iterate forever.
anim.iterations = 0

#anim.animate
anim.write('drop_shadow.gif')
exit
