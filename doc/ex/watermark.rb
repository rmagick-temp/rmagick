#! /usr/local/bin/ruby -w

require "RMagick"

img = Magick::Image.read("images/Flower_Hat.jpg").first

# Make a watermark from the word "RMagick"
mark = Magick::Image.new(140, 40) {self.background_color = "none"}
gc = Magick::Draw.new

gc.annotate(mark, 0, 0, 0, -5, "RMagick") do
    gc.gravity = Magick::CenterGravity
    gc.pointsize = 32
    if RUBY_PLATFORM =~ /mswin32/
        gc.font_family = "Georgia"
    else
        gc.font_family = "Times"
    end
    gc.fill = "white"
    gc.stroke = "none"
end

mark = mark.wave(2.5, 70).rotate(-90)

# Composite the watermark in the lower right (southeast) corner.
img2 = img.watermark(mark, 0.25, 0, Magick::SouthEastGravity)
img2.write("watermark.jpg")

