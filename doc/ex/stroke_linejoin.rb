#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(400, 150) { self.background_color = 'white' }

gc = Magick::Draw.new

gc.stroke('black').stroke_width(15)
gc.fill_opacity(0)
gc.stroke_linecap('butt')

# Draw lines with miter join
gc.stroke_linejoin('miter')
gc.polyline(25,100, 75,25, 125,100)

# Draw lines with round join
gc.stroke_linejoin('round')
gc.polyline(150,100, 200,25, 250,100)

# Draw lines with bevel join
gc.stroke_linejoin('bevel')
gc.polyline(275,100, 325,25, 375,100)

# Show line endpoints in pink
gc.fill('lightpink').fill_opacity(0)
gc.stroke('lightpink').stroke_width(2)
gc.stroke_linejoin('miter')
gc.polyline(25,100, 75,25, 125,100)
gc.polyline(150,100, 200,25, 250,100)
gc.polyline(275,100, 325,25, 375,100)
gc.fill_opacity(1)
gc.circle(75,25, 76,26)
gc.circle(200,25, 201,26)
gc.circle(325,25, 326,26)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.pointsize(14)
gc.font_weight(Magick::BoldWeight)
gc.text(35,120, "\"'miter' join\"")
gc.text(160,120, "\"'round' join\"")
gc.text(280,120, "\"'bevel' join\"")

gc.draw(imgl)

imgl.write("stroke_linejoin.gif")
