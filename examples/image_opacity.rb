#! /usr/local/bin/ruby -w

# Use the Image#opacity attribute to create 
# a semi-transparent title for an image.
 
require 'RMagick'
include Magick

balloons = Image.read('images/Hot_Air_Balloons_H.jpg').first
legend = Image.new(160, 50) { self.background_color = 'white' }

gc = Draw.new
gc.annotate(legend, 0, 0, 0, 0, "Balloon Day!\\nFri May 2 2003") {
  self.gravity = CenterGravity
  self.stroke = 'transparent'
}

legend.opacity = 0.70*TransparentOpacity
result = balloons.composite(legend, NorthWestGravity, OverCompositeOp)

result.write 'test.miff'
exit
