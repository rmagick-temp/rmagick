#! /usr/local/bin/ruby -w
require 'RMagick'

img = Magick::Image.new(200,200) { self.background_color = "#ffffcc" }

# Draw a blue circle.
gc = Magick::Draw.new
gc.stroke_width(5)
gc.stroke("blue")
gc.fill_opacity(0)
gc.circle(100,100, 100,150)
gc.draw(img)

# Get the bounding box. Use the values to draw
# a gray square surrounding the circle. Highlight
# the corners with tiny red circles.

bb = img.bounding_box
gc = Magick::Draw.new
gc.stroke("gray50")
gc.fill_opacity(0)
gc.rectangle(bb.x, bb.y, bb.x+bb.width, bb.y+bb.height)
gc.stroke("red")
gc.circle(bb.x, bb.y, bb.x+2, bb.y+2)
gc.circle(bb.x+bb.width, bb.y, bb.x+bb.width+2, bb.y+2)
gc.circle(bb.x, bb.y+bb.height, bb.x+2, bb.y+bb.height+2)
gc.circle(bb.x+bb.width, bb.y+bb.height, bb.x+bb.width+2, bb.y+bb.height+2)


gc.fill("black")
gc.stroke("transparent")
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.pointsize(9)
gc.text(bb.x-15, bb.y-5, "\'#{bb.x},#{bb.y}\'")
gc.text(bb.x+bb.width-15, bb.y-5, "\'#{bb.x+bb.width},#{bb.y}\'")
gc.text(bb.x-15, bb.y+bb.height+15, "\'#{bb.x},#{bb.y+bb.height}\'")
gc.text(bb.x+bb.width-15, bb.y+bb.height+15, "\'#{bb.x+bb.width},#{bb.y+bb.height}\'")


gc.draw(img)

img.write("bounding_box.gif")
exit
