#! /usr/local/bin/ruby -w
require 'RMagick'

include Magick

TEXT = 'get\nmultiline\ntype\nmetrics'

background = Image.new(200, 200)
gc = Draw.new

# Draw the text centered on the background
gc.annotate(background, 0, 0, 0, 0, TEXT) do
    gc.font_family = 'Verdana'
    gc.pointsize = 36
    gc.gravity = CenterGravity
    gc.stroke = 'none'
end

begin

    # Get the metrics
    metrics = gc.get_multiline_type_metrics(background, TEXT)

    # Compute the corners for a rectangle surrounding the text
    x = (background.columns - metrics.width) / 2
    y = (background.rows - metrics.height) / 2

    # Draw 2 rectangles over the text.
    gc = Draw.new
    gc.stroke('red')
    gc.stroke_width(5)
    gc.stroke_linejoin('round')
    gc.fill('cyan')
    gc.fill_opacity(0.10)
    gc.rectangle(x, y, x+metrics.width, y+metrics.height)

    gc.stroke('white')
    gc.stroke_width(1)
    gc.fill('none')
    gc.rectangle(x, y, x+metrics.width, y+metrics.height)
    gc.draw(background)

    background.border!(1,1, 'blue')

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    not_implemented = Magick::Image.read("images/notimplemented.gif").first
    not_implemented.resize!(background.columns, background.rows)
    background = not_implemented
end

background.write('get_multiline_type_metrics.gif')

