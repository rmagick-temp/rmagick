#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Magick::HatchFill class

Cols = 500
Rows = 100

fill = Magick::HatchFill.new('blue', 'black')
img = Magick::ImageList.new
img.new_image(Cols, Rows, fill)

# Annotate the filled image with the code that created the fill.

ann = Magick::Draw.new
ann.annotate(img, 0,0,0,0, "Magick::HatchFill.new('blue', 'black')") {
	self.gravity = Magick::CenterGravity
    self.fill = 'white'
    self.stroke = 'transparent'
    self.pointsize = 14
    }
#img.display
img.write('hatchfill.gif')
exit
