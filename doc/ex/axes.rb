#!/usr/local/bin/ruby -w

require 'RMagick'

# Demonstrate the use of RMagick's Draw class
# and show the default coordinate system.

# Create a canvas to draw on. Use the HatchFill class to
# cross-hatch the background with light gray lines every 10 pixels.
canvas = Magick::ImageList.new
canvas.new_image(250, 250, Magick::HatchFill.new('white', 'gray90'))

# Draw a border around the edges of the canvas.
border = Magick::Draw.new
border.stroke('thistle')
border.fill_opacity(0)
border.rectangle(0,0, canvas.columns-1, canvas.rows-1)
border.draw(canvas)

# Draw gold axes with arrow-heads.
axes = Magick::Draw.new
axes.fill_opacity(0)
axes.stroke('gold3')
axes.stroke_width(4)
axes.stroke_linecap('round')
axes.stroke_linejoin('round')
axes.polyline(18,canvas.rows-10, 10,canvas.rows-3, 3,canvas.rows-10,
              10,canvas.rows-10, 10,10, canvas.columns-10,10,
              canvas.columns-10,3, canvas.columns-3,10, canvas.columns-10, 18)
axes.draw(canvas)

# Draw a red circle to show the direction of rotation.
# Make it slightly transparent so the hatching will show through.
circle = Magick::Draw.new
circle.stroke('tomato')
circle.fill_opacity(0)
circle.stroke_opacity(0.75)
circle.stroke_width(6)
circle.stroke_linecap('round')
circle.stroke_linejoin('round')
circle.ellipse(canvas.rows/2,canvas.columns/2, 80, 80, 0, 315)
circle.polyline(180,70, 173,78, 190,78, 191,62)
circle.draw(canvas)

# Label the axes and the circle.
labels = Magick::Draw.new
labels.fill('black')
labels.stroke('transparent')
labels.font_style(Magick::ItalicStyle)
labels.pointsize(14)
labels.gravity(Magick::NorthWestGravity)
labels.text(20,30, "'0,0'")
labels.gravity(Magick::NorthEastGravity)
labels.text(20,30, "'+x'")
labels.gravity(Magick::SouthWestGravity)
labels.text(20,20, "'+y'")
labels.gravity(Magick::CenterGravity)
labels.text(0,0, "Rotation")
labels.draw(canvas)

#canvas.display
canvas.write("axes.gif")
exit
