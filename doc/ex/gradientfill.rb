#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the GradientFill class

Rows = 100
Cols = 500

fill = Magick::GradientFill.new(-Cols, Rows, -Rows, Cols, "blue", "black")
img = Magick::Image.new(Cols, Rows, fill);

# Annotate the filled image with the code that created the fill.

ann = Magick::Draw.new
ann.annotate(img, 0,0,0,0, "Magick::GradientFill.new(-Cols, Rows, -Rows, Cols, \"blue\", \"black\")") {
	self.gravity = Magick::CenterGravity
    self.fill = 'white'
    self.stroke = 'transparent'
    self.pointsize = 14
    }

#img.display
img.write("gradientfill.gif")
exit
