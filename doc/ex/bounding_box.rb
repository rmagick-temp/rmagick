#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(200, 200) { self.background_color = "#ffffcc" }

# Draw a blue circle.
primitives = Magick::Draw.new
primitives.stroke_width 5
primitives.stroke "blue"
primitives.fill_opacity 0
primitives.circle 100,100, 100,150
primitives.draw i

# Get the bounding box. Use the values to draw
# a gray square surrounding the circle. Highlight
# the corners with tiny red circles.

bb = i.bounding_box
primitives = Magick::Draw.new
primitives.stroke "gray50"
primitives.fill_opacity 0
primitives.rectangle bb.x, bb.y, bb.x+bb.width, bb.y+bb.height
primitives.stroke "red"
primitives.circle bb.x, bb.y, bb.x+2, bb.y+2
primitives.circle bb.x+bb.width, bb.y, bb.x+bb.width+2, bb.y+2
primitives.circle bb.x, bb.y+bb.height, bb.x+2, bb.y+bb.height+2
primitives.circle bb.x+bb.width, bb.y+bb.height, bb.x+bb.width+2, bb.y+bb.height+2


primitives.pointsize 10
primitives.fill "black"
primitives.stroke "transparent"
primitives.gravity Magick::NorthWestGravity
primitives.text bb.x-15, bb.y-5, "\'#{bb.x},#{bb.y}\'"
primitives.gravity Magick::NorthEastGravity
primitives.text bb.x-15, bb.y-5, "\'#{bb.x+bb.width},#{bb.y}\'"
primitives.gravity Magick::SouthWestGravity
primitives.text bb.x-15, bb.y-12, "\'#{bb.x},#{bb.y+bb.height}\'"
primitives.gravity Magick::SouthEastGravity
primitives.text bb.x-15, bb.y-12, "\'#{bb.x+bb.width},#{bb.y+bb.height}\'"


primitives.draw i

#i.display
i.write "bounding_box.gif"
exit
