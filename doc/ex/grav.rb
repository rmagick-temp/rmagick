#! /usr/local/bin/ruby -w

require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(400,200, Magick::HatchFill.new('white', 'lightcyan2'))

gc = Magick::Draw.new

# Draw blue lines to indicate positioning
gc.stroke('blue')
gc.fill('transparent')
gc.rectangle(20,20, 380,180)
gc.line(200,20, 200,180)
gc.line(20,100, 380,100)

# Draw text at all 9 compass points.
gc.stroke('transparent')
gc.fill('black')
gc.gravity(Magick::NorthWestGravity)
gc.text(20,20, 'NorthWestGravity')
gc.gravity(Magick::NorthGravity)
gc.text( 0,20, 'NorthGravity')
gc.gravity(Magick::NorthEastGravity)
gc.text(20,20, 'NorthEastGravity')
gc.gravity(Magick::WestGravity)
gc.text(20, 0, 'WestGravity')
gc.gravity(Magick::CenterGravity)
gc.text( 0, 0, 'CenterGravity')
gc.gravity(Magick::EastGravity)
gc.text(20, 0, 'EastGravity')
gc.gravity(Magick::SouthWestGravity)
gc.text(20,20, 'SouthWestGravity')
gc.gravity(Magick::SouthGravity)
gc.text( 0,20, 'SouthGravity')
gc.gravity(Magick::SouthEastGravity)
gc.text(20,20, 'SouthEastGravity')
gc.draw(imgl)

imgl.border!(1,1, "lightcyan2")

imgl.write("grav.gif")
exit

