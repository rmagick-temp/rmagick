#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the HatchFill class

Cols = 300
Rows = 100

Background = "white"
Foreground = "LightCyan2"

fill = Magick::HatchFill.new(Background, Foreground)
img = Magick::ImageList.new
img.new_image(Cols, Rows, fill)

# Annotate the filled image with the code that created the fill.

ann = Magick::Draw.new
ann.annotate(img, 0,0,0,0, "HatchFill.new('#{Background}', '#{Foreground}')") {
    self.gravity = Magick::CenterGravity
    self.fill = 'black'
    self.stroke = 'transparent'
    self.pointsize = 14
    }
#img.display
img.write('hatchfill.gif')
exit
