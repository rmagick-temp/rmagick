#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#matte_replace method.
# This program is inspired by a thing we did in 3rd-grade "art" class.

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

# The point 130,35 is within one of the letters we just drew.
# The matte_replace method identifies the color (gray50) and
# makes all the same-colored pixels transparent. The result
# is that our mask is now transparent.
foreground = foreground.matte_replace(130,35)

# Composite the foreground over the background.
composite = bg.composite(foreground, Magick::CenterGravity, Magick::OverCompositeOp)
#composite.display
composite.write('matte_replace.gif')

exit


