#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Magick::TextureFill class.

rose = Magick::Image.read('rose:').first
fill = Magick::TextureFill.new(rose)
img = Magick::ImageList.new
img.new_image(7*rose.columns, 2*rose.rows, fill)

# Annotate the filled image with the code that created the fill.

ann = Magick::Draw.new
ann.annotate(img, 0,0,0,0, "Magick::TextureFill.new(rose)") {
	self.gravity = Magick::CenterGravity
    self.fill = 'white'
    self.stroke = 'transparent'
    self.pointsize = 14
    }

#img.display
img.write("texturefill.gif")
exit

