require 'rvg/rvg'

Magick::RVG.dpi = 90
rvg = Magick::RVG.new(12.cm, 4.cm) do |canvas|
    canvas.viewbox(0, 0, 1200, 400)
    canvas.background_fill = 'white'
    canvas.desc = "Example rect01 - rectangle with sharp corners"
    # Show outline of canvas using 'rect' element
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.rect(400, 200, 400, 100).styles(:fill=>'yellow', :stroke=>'navy', :stroke_width=>10)
end

rvg.draw.write('rect01.gif')

