#! /usr/local/bin/ruby -w

require 'RMagick'

canvas = Magick::ImageList.new
canvas.new_image(270, 100) { self.background_color = "#000000ff" }

draw = Magick::Draw.new
draw.annotate(canvas, 0, 0, 0, 0, "RMagick") {
    self.font = "Helvetica"
    self.decorate = Magick::OverlineDecoration
    self.fill = "red"
    self.gravity = Magick::CenterGravity
    self.pointsize = 48
    self.stroke = "black"
    self.undercolor = "cyan"
    }

#canvas.display
canvas.write "drawattrs.gif"
exit

