#!/usr/local/bin/ruby -w

require 'RMagick'
include Magick

# Demonstrate the Draw#rotation= method by producing
# an animated MIFF file showing a rotating text string.

text = Draw.new
text.pointsize = 28
text.font_weight = BoldWeight
text.font_style = ItalicStyle
text.gravity = CenterGravity

# Let's make it interesting. Composite the
# rotated text over a gradient fill background.
fill = GradientFill.new(100,100,100,100,"yellow","red")
bg = Image.new(200, 200, fill)

# Since antialiasing text on a transparent background doesn't
# look good, write the text on an opaque yellow background.
fg = Image.new(bg.columns, bg.rows) { self.background_color = "yellow" }

# Here's where we'll collect the individual frames.
animation = ImageList.new

0.step(345,15) { |degrees|
    frame = fg.copy
    text.annotate(frame, 0,0,0,0, "Rotating Text") {
        self.rotation = degrees
    }
    # Composite the text over the gradient filled background frame.
    animation << bg.composite(frame, CenterGravity, DisplaceCompositeOp)
}

animation.delay = 8

# ignored if ImageMagick not configured with ZLIB
animation.compression = ZipCompression
#animation.animate
animation.write("rotated_text.miff")
exit
