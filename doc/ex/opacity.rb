#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(400, 400) { self.background_color = "white" }

gc = Magick::Draw.new

gc.font('Helvetica').pointsize(72)

# Draw large black percentages
gc.text( 10,120,"'100%%'")
gc.text(235,120,"'75%%'")
gc.text( 35,320,"'50%%'")
gc.text(235,320,"'25%%'")

# Establish the stroke and fill parameters
gc.stroke("'blue'").stroke_width(10).stroke_linejoin('round')
gc.fill('yellow')

# For each of the 4 opacity levels, draw a blue-rimmed
# yellow triangle over the corresponding number.
# Note that the opacity argument can be either a number
# between 0 and 1 or a string 'NN%'.
gc.opacity('100%') #  or 1.00
gc.polygon(25,175, 175, 25, 25,25)
gc.opacity('75%')  #  or 0.75
gc.polygon(225,175, 375,25, 225,25)
gc.opacity(0.50)   #  or '50%'
gc.polygon( 25,375, 175, 225, 25,225)
gc.opacity(0.25)   #  or '25%'
gc.polygon(225,375, 375,225, 225,225)

gc.draw(i)
#i.display
i.write("opacity.gif")
