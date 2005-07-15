require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(4.cm, 4.cm).viewbox(0, 0, 400, 400) do |canvas|
    canvas.title = "Example triangle01 - simple example of a 'path'"
    canvas.desc = 'A path that draws a triangle'

    canvas.background_fill = 'white'
    canvas.rect(395, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue')

    canvas.path('M 100 100 L 300 100 L 200 300 z').styles(:fill=>'red', :stroke=>'blue', :stroke_width=>3)
end

rvg.draw.write('triangle01.gif')

