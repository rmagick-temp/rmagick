#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Magick::Draw#fill_pattern and #stroke_pattern attributes.

temp = Magick::ImageList.new
temp << Magick::Image.new(5, 10) {self.background_color = "black"}
temp << Magick::Image.new(5, 10) {self.background_color = "gold"}
stroke_pattern = temp.append(false)

img = Magick::Image.new(280, 250) {self.background_color = "none"}

gc = Magick::Draw.new
gc.annotate(img, 0, 0, 0, 0, "PATT\nERNS") do
    self.gravity = Magick::CenterGravity
    self.font_weight = Magick::BoldWeight
    self.pointsize = 100
    self.fill_pattern = Magick::Image.read("images/Flower_Hat.jpg").first
    self.stroke_width = 5
    self.stroke_pattern = stroke_pattern
end

img.write("fill_pattern.gif")



