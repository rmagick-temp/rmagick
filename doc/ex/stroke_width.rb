#! /usr/local/bin/ruby -w

require 'RMagick'

i = Magick::ImageList.new
i.new_image(500,80) { self.background_color = "white" }

gc = Magick::Draw.new

# Draw 5-pixel wide line
gc.stroke('LightPink')
gc.stroke_width(5)
gc.line(10,30, 130,30)

# Draw 10-pixel wide line
gc.stroke('LightSkyBlue2')
gc.stroke_width(10)
gc.line(130,30, 230,30)

# Draw 20-pixel wide line
gc.stroke('GreenYellow')
gc.stroke_width(20)
gc.line(230,30,370,30)

# Draw 40-pixel wide line
gc.stroke('MediumOrchid2')
gc.stroke_width(40)
gc.line(370,30,490,30)

# Draw 1-pixel wide line through the center
gc.stroke('black')
gc.stroke_width(1)
gc.line(10,30,490,30)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(60,70,"'5'")
gc.text(180,70,"'10'")
gc.text(300,70,"'20'")
gc.text(420,70,"'40'")

# Highlight border
gc.fill('transparent')
gc.stroke('thistle')
gc.stroke_width(1)
gc.stroke_dasharray()
gc.rectangle(0,0, 499,79)

gc.draw(i)
#i.display
i.write('stroke_width.gif')
exit
