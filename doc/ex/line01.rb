require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm).viewbox(0, 0, 1200, 400) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example line01 - lines expressed in user coordinates"

    # Show outline of canvas using the 'rect' method
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.g.styles(:stroke=>'green') do |grp|
        grp.line(100, 300, 300, 100).styles(:stroke_width=>5)
        grp.line(300, 300, 500, 100).styles(:stroke_width=>10)
        grp.line(500, 300, 700, 100).styles(:stroke_width=>15)
        grp.line(700, 300, 900, 100).styles(:stroke_width=>20)
        grp.line(900, 300, 1100, 100).styles(:stroke_width=>25)
    end

end

rvg.draw.write('line01.gif')

