require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(10.cm, 3.cm).viewbox(0, 0, 100, 30) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example Use01 - Simple case of 'use' on a 'rect'"
    r = Magick::RVG::Group.new do |grp|
            grp.rect(60, 10)
        end
    canvas.rect(99.6, 29.6, 0.1, 0.1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>0.2)
    canvas.use(r, 20, 10)
end

rvg.draw.write('Use01.gif')

