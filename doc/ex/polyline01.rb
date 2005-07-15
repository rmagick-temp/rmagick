require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm).viewbox(0, 0, 1200, 400) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example polyline01 - increasingly larger bars"

    # Show outline of canvas using the 'rect' method
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.polyline(50,375,
                    150,375,150,325,250,325,250,375,
                    350,375,350,250,450,250,450,375,
                    550,375,550,175,650,175,650,375,
                    750,375,750,100,850,100,850,375,
                    950,375,950,25,1050,25,1050,375,
                    1150,375).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>10)

end

rvg.draw.write('polyline01.gif')

