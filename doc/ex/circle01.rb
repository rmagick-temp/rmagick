require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm) do |canvas|
    canvas.viewbox(0, 0, 1200, 400)
    canvas.background_fill = 'white'
    canvas.desc = "Example circle01 - circle filled with red and stroked with blue"

    # Show outline of canvas using the 'rect' method
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.circle(100, 600, 200).styles(:fill=>'red', :stroke=>'blue', :stroke_width=>10)
end

rvg.draw.write('circle01.gif')

