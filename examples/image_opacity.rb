
# Ccreate a semi-transparent title for an image.

require 'RMagick'
include Magick

puts <<END_INFO

This example uses a semi-transparent background color to create a title.
View the resulting image by entering the command: display image_opacity.miff

END_INFO

balloons = Image.read('../doc/ex/images/Hot_Air_Balloons_H.jpg').first
legend = Image.new(160, 50) { self.background_color = '#ffffffc0' }

gc = Draw.new
gc.annotate(legend, 0, 0, 0, 0, "Balloon Day!\\nFri May 2 2003") {
  self.gravity = CenterGravity
  self.stroke = 'transparent'
  self.fill = 'white'
  self.pointsize = 18
}

result = balloons.composite(legend, SouthGravity, OverCompositeOp)

puts "...Writing image_opacity.miff"
result.write 'image_opacity.miff'
exit
