#! /usr/local/bin/ruby -w
require 'rvg/rvg'

Magick::RVG.dpi = 90

BUTT   = {:stroke=>'black', :stroke_width=>70, :stroke_linecap=>'butt'}
ROUND  = {:stroke=>'black', :stroke_width=>70, :stroke_linecap=>'round'}
SQUARE = {:stroke=>'black', :stroke_width=>70, :stroke_linecap=>'square'}
THIN   = {:stroke=>'#ffcccc', :stroke_width=>5}
TEXT   = {:text_anchor=>'middle', :font_size=>50, :font_family=>'Verdana'}
CIRCLE = {:fill=>'#ffcccc', :stroke=>'none'}

rvg = Magick::RVG.new(12.cm, 2.cm).viewbox(0, 0, 1200, 200) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example linecap - demonstrates three stroke-linecap values"

    canvas.g.translate(200, 75) do |butt|
        butt.line(-125, 0, 125, 0).styles(BUTT)
        butt.line(-125, 0, 125, 0).styles(THIN)
        butt.circle(8, -125, 0).styles(CIRCLE)
        butt.circle(8,  125, 0).styles(CIRCLE)
        butt.text(0, 90, "'butt' cap").styles(TEXT)
    end
    canvas.g.translate(600, 75) do |round|
        round.line(-125, 0, 125, 0).styles(ROUND)
        round.line(-125, 0, 125, 0).styles(THIN)
        round.circle(8, -125, 0).styles(CIRCLE)
        round.circle(8,  125, 0).styles(CIRCLE)
        round.text(0, 90, "'round' cap").styles(TEXT)
    end

    canvas.g.translate(1000, 75) do |square|
        square.line(-125, 0, 125, 0).styles(SQUARE)
        square.line(-125, 0, 125, 0).styles(THIN)
        square.circle(8, -125, 0).styles(CIRCLE)
        square.circle(8,  125, 0).styles(CIRCLE)
        square.text(0, 90, "'square' cap").styles(TEXT)
    end
    canvas.rect(1192, 195, 1, 1).styles(:stroke=>'blue', :fill=>'none')
end

rvg.draw.write('rvg_linecap.gif')

