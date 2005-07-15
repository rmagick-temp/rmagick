require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm).viewbox(0, 0, 1200, 400) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example polygon01 - star and hexagon"

    # Show outline of canvas using the 'rect' method
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.polygon(350,75,379,161,469,161,397,215,
                    423,301,350,250,277,301,303,215,
                    231,161,321,161).styles(:fill=>'red', :stroke=>'blue', :stroke_width=>10)

    canvas.polygon(850,75,958,137.5,958,262.5,
                    850,325,742,262.6,742,137.5).
                    styles(:fill=>'lime', :stroke=>'blue', :stroke_width=>10)

end

rvg.draw.write('polygon01.gif')

