#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(615, 100)

gc = Magick::Draw.new
gc.stroke('black').stroke_width(30)

# Draw thick line with "butt" linecap
gc.stroke_linecap('butt')
gc.line(25,50, 175,50)

# Draw thick line with "round" linecap
gc.stroke_linecap('round')
gc.line(225,50, 375,50)

# Draw thick line with "square" linecap
gc.stroke_linecap('square')
gc.line(425,50, 575,50)

# Show line endpoints in pink
gc.fill('lightpink')
gc.stroke('lightpink').stroke_width(3)
gc.line(25,50, 175,50)
gc.circle(25,50, 27,52).circle(175,50,177,52)
gc.line(225,50, 375,50)
gc.circle(225,50, 227,52).circle(375,50,377,52)
gc.line(425,50, 575, 50)
gc.circle(425,50, 427,52).circle(575,50,577,52)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.pointsize(14)
gc.font_weight(Magick::BoldWeight)

gc.text(55,90, "\"'butt' cap\"")
gc.text(250,90, "\"'round' cap\"")
gc.text(450,90, "\"'square' cap\"")

gc.draw(imgl)

imgl.write("stroke_linecap.gif")
