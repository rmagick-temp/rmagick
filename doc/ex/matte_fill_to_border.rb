#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#matte_fill_to_border method.
# Same as the matte_replace example, but relies on
# the border color to deliniate the opaque parts.

Rows = 100
Cols = 425

# Create a background layer with a gradient fill
fill = Magick::GradientFill.new(0, Rows, Cols, Rows, 'MediumOrchid', 'MistyRose')
bg = Magick::Image.new(Cols, Rows, fill)

# Create a black foreground layer
foreground = Magick::Image.new(Cols, Rows) { self.background_color = 'black' }

# Make a "mask" by annotating the foreground using medium-gray text.
# It doesn't matter what color the text is, as long as it's not black.
# Make it plenty big so it fills most of the foreground.
text = Magick::Draw.new
text.annotate(foreground, 0,0,0,0, 'RMagick') {
    self.gravity = Magick::CenterGravity
    self.pointsize = 70
    self.font_weight = Magick::BoldWeight
    self.fill = 'gray50'
    self.stroke = 'gray50'
}

# Set the border color to the text color. The matte_fill_to_border
# method will make all the contiguous pixels starting at 0,0 transparent.
foreground.border_color = 'gray50'
foreground = foreground.matte_fill_to_border(0,0)

# Composite the foreground over the background.
composite = bg.composite(foreground, Magick::CenterGravity, Magick::OverCompositeOp)
#composite.display
composite.write('matte_fill_to_border.gif')

exit


