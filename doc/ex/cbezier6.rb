#! /usr/local/bin/ruby -w

require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(400, 300, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new

# Draw Bezier curve
gc.stroke('red')
gc.stroke_width(2)
gc.fill_opacity(0)
gc.bezier(50,150, 50,50, 200,50, 200,150, 200,250, 350,250, 350,150)

# Draw filled circles for the control points
gc.fill('gray50')
gc.stroke('gray50')
gc.fill_opacity(1)
gc.circle(50,50, 53,53)
gc.circle(200,50, 203,53)
gc.circle(200,250, 203,253)
gc.circle(350,250, 353,253)

# Draw circles on the points the curve passes through
gc.fill_opacity(0)
gc.circle(50,150, 53,153)
gc.circle(200,150, 203,153)
gc.circle(350,150, 353,153)

# Draw the gray lines between points and control points
gc.line(50,50, 50,150)
gc.line(200,50, 200,250)
gc.line(350,150, 350,250)

# Annotate
gc.fill('black')
gc.stroke('transparent')
gc.text(30,170, "'50,150'")
gc.text(30, 40, "'50,50'")
gc.text(180,40, "'200,50'")
gc.text(210,155,"'200,150'")
gc.text(180,270,"'200,250'")
gc.text(330,270,"'350,250'")
gc.text(330,140,"'350,150'")
gc.draw(imgl)

imgl.border!(1,1, 'lightcyan2')

imgl.write("cbezier6.gif")
exit
