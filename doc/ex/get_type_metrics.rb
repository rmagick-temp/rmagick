#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Draw the braces and labels on the image. Each
# brace is drawn as a pair of quadratic bezier curves.

def draw_labels(image, x, y, metrics)

    gc = Draw.new
    gc.font_family('Times')
    gc.pointsize(13)
    gc.fill('none')
    gc.stroke('black')
    gc.stroke_width(1)

    # Draw a brace between the origin and descent
    h = (-metrics.descent) / 2      # half the height of the brace
    xp = x + metrics.width
    gc.path("M#{xp+23},#{y} Q#{xp+37},#{y} #{xp+33},#{y+h/2} T#{xp+37},#{y+h}")

    gc.path("M#{xp+23},#{y-metrics.descent} " +
            "Q#{xp+37},#{y-metrics.descent} " +
            " #{xp+33},#{y-metrics.descent-h/2} " +
            "T#{xp+37},#{y-metrics.descent-h}")

    # Draw a brace between the origin and ascent
    h = metrics.ascent / 2          # half the height of the brace
    gc.path("M#{xp+23},#{y} " +
            "Q#{xp+37},#{y} " +
            " #{xp+33},#{y-h/2} " +
            "T#{xp+37},#{y-h}")

    gc.path("M#{xp+23},#{y-metrics.ascent} " +
            "Q#{xp+37},#{y-metrics.ascent} " +
            " #{xp+33},#{y-metrics.ascent+h/2} " +
            "T#{xp+37},#{y-metrics.ascent+h}")

    # Draw a brace between the origin and height
    h = metrics.height / 2
    gc.path("M#{x-16},#{y} Q#{x-23},#{y} #{x-16},#{y-h/2} T#{x-23},#{y-h}")

    gc.path("M#{x-16},#{y-metrics.height}" +
            "Q#{x-23},#{y-metrics.height}" +
            " #{x-16},#{y-metrics.height+h/2}" +
            "T#{x-23},#{y-metrics.height+h}")

    # Draw a brace between the origin and the width
    h = metrics.width / 2
    gc.path("M#{x},27 Q#{x},20 #{x+h/2},23 T#{x+h},20")

    gc.path("M#{x+metrics.width},27 "+
            "Q#{x+metrics.width},20 "+
            " #{x+(3*metrics.width)/4},23 " +
            "T#{x+h},20")

    # Draw a brace between the origin and max_advance
    h = metrics.max_advance / 2
    gc.path("M#{x},#{y-metrics.descent+16} " +
            "Q#{x},#{y-metrics.descent+24} " +
            " #{x+h/2},#{y-metrics.descent+20} " +
            "T#{x+h},#{y-metrics.descent+24}")

    gc.path("M#{x+metrics.max_advance},#{y-metrics.descent+16} " +
            "Q#{x+metrics.max_advance},#{y-metrics.descent+24} " +
            " #{x+(3*metrics.max_advance)/4},#{y-metrics.descent+20} " +
            "T#{x+h},#{y-metrics.descent+24}")

    gc.stroke('none')
    gc.fill('black')
    gc.text(xp+40, y-(metrics.ascent/2)+4, 'ascent')
    gc.text(xp+40, y-(metrics.descent/2)+4, 'descent')
    gc.text(x-60, y-metrics.height/2+4, 'height')
    gc.text(x+(metrics.width/2)-15, 15, 'width')
    gc.text(x+(metrics.max_advance)/2-38, 290, "max_advance")

    gc.draw(image)
end



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

# Draw all the braces and labels.
draw_labels(canvas, Origin_x, Origin_y, metrics)

gc.draw(canvas)
canvas.border!(1,1,'blue')

canvas.write('get_type_metrics.gif')

