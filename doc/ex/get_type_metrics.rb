#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Add a method for drawing braces. Each brace
# is drawn as a pair of quadratic bezier curves.
class Draw

    # (w,h) - width & height of rectangle enclosing brace.
    # Normally the brace is drawn with its opening to the
    # left and its lower point on the origin.
    #
    # Set w < 0 to draw right-opening brace. Set h < 0 to
    # position top point at origin.
    #
    # The placement & orientation is affected by the
    # current user coordinate system.
    def brace(w, h)
        raise(ArgumentError, "width must be != 0") unless w != 0
        raise(ArgumentError, "height must be != 0") unless h != 0
        path("M0,0 Q#{w},0 #{w/2.0},#{-h/4.0} T#{w},#{-h/2.0}" +
             "Q0,#{-h/2.0}, #{w/2.0},#{-(3.0*h/4.0)} T0,#{-h}")
    end

    def rotate_about(degrees, x, y)
        translate(x, y)
        rotate(degrees)
        translate(-x, -y)
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
gc.push
gc.translate(Origin_x+metrics.width+23, Origin_y)
gc.brace(10, metrics.descent)
gc.pop

# Draw a brace between the origin and ascent
gc.push
gc.translate(Origin_x+metrics.width+23, Origin_y)
gc.brace(10, metrics.ascent)
gc.pop

# Draw a brace between the origin and height
gc.push
gc.translate(Origin_x-13, Origin_y-metrics.height)
gc.rotate(180)
gc.brace(10, metrics.height)
gc.pop

# Draw a brace between the origin and the width
gc.push
gc.translate(Origin_x+metrics.width, 27.0)
gc.rotate(-90)
gc.brace(10, metrics.width)
gc.pop

# Draw a brace between the origin and max_advance
gc.push
gc.translate(Origin_x, Origin_y-metrics.descent+14)
gc.rotate(90)
gc.brace(10, metrics.max_advance)
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

