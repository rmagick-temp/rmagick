#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#opaque method.

Rows = 100
Cols = 425

# Create a solid black image
ex = Magick::Image.new(Cols, Rows) { self.background_color = 'black' }

# Annotate with "RMagick"
text = Magick::Draw.new
text.annotate(ex, 0,0,0,0, 'RMagick') {
    self.gravity = Magick::CenterGravity
    self.pointsize = 70
    self.font_weight = Magick::BoldWeight
    self.fill = 'gray50'
    self.stroke = 'gray50'
}

ex2 = ex.opaque('gray50','MediumOrchid')
ex2.crop!(ex2.columns/2, 0, ex2.columns/2, ex2.rows)

# Composite the foreground over the background.
composite = ex.composite(ex2, Magick::EastGravity, Magick::OverCompositeOp)
#composite.display
composite.write('opaque.gif')

exit


