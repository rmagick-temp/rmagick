#! /usr/local/bin/ruby -w
require 'RMagick'

# Add a method for drawing braces.
module Magick
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
                 "Q0,#{-h/2.0} #{w/2.0},#{-(3.0*h/4.0)} T0,#{-h}")
        end

    end     # class Draw
end



Origin_x = 110
Origin_y = 230
Glyph = 'g'
Face = RUBY_PLATFORM =~ /mswin/ ? "Verdana" : "Times"

canvas = Magick::Image.new(410, 320, Magick::HatchFill.new('white', 'lightcyan2'))

# Draw a big lowercase 'g' on the canvas. Leave room on all sides for
# the labels. Use 'undercolor' to set off the glyph.
glyph = Magick::Draw.new
glyph.annotate(canvas, 0, 0, Origin_x, Origin_y, Glyph) do
    glyph.pointsize = 124
    glyph.stroke = 'none'
    glyph.fill = 'black'
    glyph.font_family = Face
    glyph.undercolor = '#ffff00c0'
end

# Call get_type_metrics. This is what this example's all about.
metrics = glyph.get_type_metrics(canvas, Glyph)

gc = Magick::Draw.new
gc.translate(Origin_x, Origin_y)

# Draw the origin as a big red dot.
gc.stroke('red')
gc.fill('red')
gc.circle(0, 0, 0, 2)

# All our lines will be medium-gray, dashed, and thin.
gc.stroke('gray50')
gc.stroke_dasharray(5,2)
gc.stroke_width(1)
gc.fill('none')

# baseline
gc.line(-10, 0, metrics.width+20, 0)

# a vertical line through the origin
gc.line(0, -metrics.descent-metrics.height-10, 0, -metrics.descent+15)

# descent
gc.line(-10, -metrics.descent, metrics.width+20, -metrics.descent)

# ascent
gc.line(-10, -metrics.ascent, metrics.width+20, -metrics.ascent)

# height
gc.line(-10, -metrics.descent-metrics.height,
        metrics.width+10, -metrics.descent-metrics.height)

# width
gc.line(metrics.width, -metrics.descent-metrics.height-10,
        metrics.width, -metrics.descent+20)

# max_advance
gc.line(metrics.max_advance, -10, metrics.max_advance, -metrics.descent+20)

gc.draw(canvas)

# Draw the braces and labels. Position the braces by transforming the
# user coordinate system with translate and rotate methods.
gc = Magick::Draw.new
gc.font_family('Face')
gc.pointsize(13)
gc.fill('none')
gc.stroke('black')
gc.stroke_width(1)
gc.translate(Origin_x, Origin_y)

# between origin and descent
gc.push
gc.translate(metrics.width+23, 0)
gc.brace(10, metrics.descent)
gc.pop

# between origin and ascent
gc.push
gc.translate(metrics.width+23, 0)
gc.brace(10, metrics.ascent)
gc.pop

# between origin and height
gc.push
gc.translate(-13, -metrics.descent-metrics.height)
gc.rotate(180)
gc.brace(10, metrics.height)
gc.pop

# between origin and width
gc.push
gc.translate(metrics.width, -metrics.descent-metrics.height-10-3)
gc.rotate(-90)
gc.brace(10, metrics.width)
gc.pop

# between origin and max_advance
gc.push
gc.translate(0, -metrics.descent+15)
gc.rotate(90)
gc.brace(10, metrics.max_advance)
gc.pop

# Add labels
gc.font_weight(Magick::NormalWeight)
gc.font_style(Magick::NormalStyle)
gc.stroke('none')
gc.fill('black')
gc.text(metrics.width+40, -(metrics.ascent/2)+4, 'ascent')
gc.text(metrics.width+40, -(metrics.descent/2)+4, 'descent')
gc.text(-60, -metrics.descent-metrics.height/2+4, 'height')
gc.text((metrics.width/2)-15, -metrics.descent-metrics.height-25, 'width')
gc.text((metrics.max_advance)/2-38, -metrics.descent+35, "max_advance")

gc.draw(canvas)
canvas.border!(1,1,'blue')
canvas.write('get_type_metrics.gif')

