#! /usr/local/bin/ruby -w

require 'RMagick'

i = Magick::ImageList.new
i.new_image(500, 400, Magick::HatchFill.new('seashell'))

gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(2)
gc.fill_opacity(0)
gc.bezier(100,200, 100,100, 250,100, 250,200, 250,300, 400,300, 400,200)

# Draw filled circles for the control points
gc.fill('gray50')
gc.stroke('gray50')
gc.fill_opacity(1)
gc.circle(100,100, 103,103)
gc.circle(250,100, 253,103)
gc.circle(250,300, 253,303)
gc.circle(400,300, 403,303)

# Draw circles on the points the curve passes through
gc.fill_opacity(0)
gc.circle(100,200, 103,203)
gc.circle(250,200, 253,203)
gc.circle(400,200, 403,203)

# Draw the gray lines between points and control points
gc.line(100,100, 100,200)
gc.line(250,100, 250,300)
gc.line(400,200, 400,300)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(80,220, "'100,200'")
gc.text(80, 90, "'100,100'")
gc.text(230,90, "'250,100'")
gc.text(260,205,"'250,200'")
gc.text(230,320,"'250,300'")
gc.text(380,320,"'400,300'")
gc.text(380,190,"'400,200'")
gc.draw(i)
#i.display
i.write("cbezier6.gif")
exit
