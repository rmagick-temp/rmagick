require 'rvg/rvg'

Magick::RVG.dpi = 90

MITER = {:stroke=>'black', :stroke_width=>70, :fill=>'none', :stroke_linejoin=>'miter'}
ROUND = {:stroke=>'black', :stroke_width=>70, :fill=>'none', :stroke_linejoin=>'round'}
BEVEL = {:stroke=>'black', :stroke_width=>70, :fill=>'none', :stroke_linejoin=>'bevel'}
THIN  = {:stroke=>'#ffcccc', :stroke_width=>5, :fill=>'none'}
TEXT   = {:text_anchor=>'middle', :font_size=>50, :font_family=>'Verdana'}
CIRCLE = {:fill=>'#ffcccc', :stroke=>'none'}

rvg = Magick::RVG.new(12.cm, 3.5.cm).viewbox(0, 0, 1200, 350) do |canvas|
    canvas.desc = 'Example linecap - demonstrates three stroke-linecap values'
    canvas.background_fill = 'white'

    canvas.g.translate(200, 75) do |miter|
        miter.path("M -125,150 L 0,0 L 125,150").styles(MITER)
        miter.path("M -125,150 L 0,0 L 125,150").styles(THIN)
        miter.circle(8).styles(CIRCLE)
        miter.text(0, 230, "'miter' join").styles(TEXT)
    end

    canvas.g.translate(600, 75) do |round|
        round.path("M -125,150 L 0,0 L 125,150").styles(ROUND)
        round.path("M -125,150 L 0,0 L 125,150").styles(THIN)
        round.circle(8).styles(CIRCLE)
        round.text(0, 230, "'round' join").styles(TEXT)
    end

    canvas.g.translate(1000, 75) do |bevel|
        bevel.path("M -125,150 L 0,0 L 125,150").styles(BEVEL)
        bevel.path("M -125,150 L 0,0 L 125,150").styles(THIN)
        bevel.circle(8).styles(CIRCLE)
        bevel.text(0, 230, "'bevel' join").styles(TEXT)
    end

    canvas.rect(1192, 345, 1, 1).styles(:stroke=>'blue', :fill=>'none')
end

rvg.draw.write('rvg_linejoin.gif')

