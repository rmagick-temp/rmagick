#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Magick::TextureFill class.

granite = Magick::Image.read('granite:').first
fill = Magick::TextureFill.new(granite)
img = Magick::ImageList.new
img.new_image(300, 100, fill)

# Annotate the filled image with the code that created the fill.

ann = Magick::Draw.new
ann.annotate(img, 0,0,0,0, "TextureFill.new(granite)") {
    self.gravity = Magick::CenterGravity
    self.fill = 'white'
    self.font_weight = Magick::BoldWeight
    self.stroke = 'transparent'
    self.pointsize = 14
    }

#img.display
img.write("texturefill.gif")
exit

