#! /usr/local/bin/ruby -w

require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(500,80, Magick::HatchFill.new('white','lightcyan2'))

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

gc.draw(imgl)
imgl.border!(1,1,"lightcyan2")

imgl.write('stroke_width.gif')
exit
