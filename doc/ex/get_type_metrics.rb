#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Add a method for drawing braces. Each brace
# is drawn as a pair of quadratic bezier curves.
class Draw

    # (x,y) - corner of rectangle enclosing brace
    # (w,h) - width & height of rectangle enclosing brace
    # The orientation is affected by the current user coordinate system.
    def brace(x, y, w, h)
        raise(ArgumentError, "width must be > 0") unless w > 0
        raise(ArgumentError, "height must be > 0") unless h > 0
        qx = x + w
        qy = y
        x0 = x + w / 2
        y0 = y + h/4
        tx = x + w
        ty = y + h/2
        path("M#{x},#{y}Q#{qx},#{qy} #{x0},#{y0}T#{tx},#{ty}")
        qy = y + h
        y0 = y + 3*h/4
        path("M#{x},#{y+h}Q#{qx},#{qy} #{x0},#{y0}T#{tx},#{ty}")
    end

end     # class Draw




Origin_x = 110
Origin_y = 220
Glyph = 'g'

canvas = Image.new(410,300,HatchFill.new('white', 'lightcyan2'))

# Draw a big lowercase 'g' on the canvas.
# Leave room on all sides for the labels.
# Use 'undercolor' to set off the glyph.
glyph = Draw.new
glyph.annotate(canvas, 0, 0, Origin_x, Origin_y, Glyph) do |opts|
    opts.pointsize = 124
    opts.stroke = 'none'
    opts.fill = 'black'
    opts.font_family = 'Times'
    opts.undercolor = '#ffff00c0'
end

# Call get_type_metrics. This is what this example's all about.
metrics = glyph.get_type_metrics(canvas, Glyph)

# Draw all the dotted lines.
gc = Draw.new

# Draw the origin as a big red dot.
gc.stroke('red')
gc.fill('red')
gc.stroke_dasharray()
gc.circle(Origin_x, Origin_y, Origin_x, Origin_y+2)

# All our lines will be thin, dashed, and gray.
gc.stroke('gray50')
gc.stroke_dasharray(5,2)
gc.stroke_width(1)
gc.fill('none')

# Draw the baseline
gc.line(Origin_x-10, Origin_y, Origin_x+metrics.width+20, Origin_y)

# Draw a vertical line through the origin
gc.line(Origin_x, 30, Origin_x, 270)

# Draw a line showing the value of 'descent'
gc.line(Origin_x-10, Origin_y-metrics.descent, Origin_x+metrics.width+20, Origin_y-metrics.descent)

# Draw a line showing the value of 'ascent'
gc.line(Origin_x-10, Origin_y-metrics.ascent, Origin_x+metrics.width+20, Origin_y-metrics.ascent)

# Draw a line showing the value of 'height'
gc.line(Origin_x-10, Origin_y-metrics.height, Origin_x+metrics.width+10, Origin_y-metrics.height)

# Draw a line showing the value of 'width'
gc.line(Origin_x+metrics.width, 30, Origin_x+metrics.width, 270)

# Draw a line showing the value of 'max_advance'
gc.line(Origin_x+metrics.max_advance, Origin_y-metrics.descent-10,
        Origin_x+metrics.max_advance, 270)

gc.draw(canvas)

# The rest of this script is just annotation.
# Draw all the braces
gc = Draw.new
gc.font_family('Times')
gc.pointsize(13)
gc.fill('none')
gc.stroke('black')
gc.stroke_width(1)

# Draw a brace between the origin and descent
gc.brace(Origin_x+metrics.width+23, Origin_y, 10, -metrics.descent)

# Draw a brace between the origin and ascent
gc.brace(Origin_x+metrics.width+23, Origin_y-metrics.ascent, 10, metrics.ascent)

# Draw a brace between the origin and height
gc.push
gc.translate(Origin_x-13, Origin_y-metrics.height/2)
gc.rotate(180)
gc.translate(-(Origin_x-13), -(Origin_y-metrics.height/2))
gc.brace(Origin_x-13, Origin_y-metrics.height, 10, metrics.height)
gc.pop

# Draw a brace between the origin and the width
gc.push
gc.translate(Origin_x, 27.0)
gc.rotate(-90)
gc.translate(-Origin_x, -27.0)
gc.brace(Origin_x, 27, 10, metrics.width)
gc.pop

# Draw a brace between the origin and max_advance
gc.push
gc.translate(Origin_x, Origin_y-metrics.descent+13)
gc.scale(-1, 1)
gc.rotate(90)
gc.translate(-Origin_x, -(Origin_y-metrics.descent+13))
gc.brace(Origin_x, Origin_y-metrics.descent+13, 10, metrics.max_advance)
gc.pop

# Add the labels
gc.stroke('none')
gc.fill('black')
gc.text(Origin_x+metrics.width+40, Origin_y-(metrics.ascent/2)+4, 'ascent')
gc.text(Origin_x+metrics.width+40, Origin_y-(metrics.descent/2)+4, 'descent')
gc.text(Origin_x-60, Origin_y-metrics.height/2+4, 'height')
gc.text(Origin_x+(metrics.width/2)-15, 15, 'width')
gc.text(Origin_x+(metrics.max_advance)/2-38, 290, "max_advance")

gc.draw(canvas)

canvas.border!(1,1,'blue')

canvas.write('get_type_metrics.gif')

