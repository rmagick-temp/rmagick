
# Use the Image#opacity attribute to create 
# a semi-transparent title for an image.
 
require 'RMagick'
include Magick

puts <<END_INFO

This example uses the opacity attribute to create a semi-transparent title.
View the resulting image by entering the command: display image_opacity.miff

END_INFO

balloons = Image.read('../doc/ex/images/Hot_Air_Balloons_H.jpg').first
legend = Image.new(160, 50) { self.background_color = 'white' }

gc = Draw.new
gc.annotate(legend, 0, 0, 0, 0, "Balloon Day!\\nFri May 2 2003") {
  self.gravity = CenterGravity
  self.stroke = 'transparent'
}

legend.opacity = 0.50*TransparentOpacity
result = balloons.composite(legend, NorthWestGravity, OverCompositeOp)

puts "...Writing image_opacity.miff"
result.write 'image_opacity.miff'
exit
