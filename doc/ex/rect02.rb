require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm) do |canvas|
    canvas.viewbox(0, 0, 1200, 400)
    canvas.background_fill = 'white'
    canvas.desc = "Example rect02 - rounded rectangles"

    # Show outline of canvas using 'rect' method
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.rect(400, 200, 100, 100).round(50).styles(:fill=>'green')

    canvas.g.translate(700, 210).rotate(-30) do |grp|
        grp.rect(400, 200, 0, 0).round(50).styles(:fill=>'none', :stroke=>'purple', :stroke_width=>30)
    end

end

rvg.draw.write('rect02.gif')

