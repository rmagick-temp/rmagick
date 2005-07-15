require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(10.cm, 3.cm).viewbox(0, 0, 100, 30) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example Use03 - 'use' with a 'transform' attribute"

    myrect = Magick::RVG::Group.new do |grp|
                 grp.rect(60, 10)
             end
    canvas.rect(99.6, 29.6, 0.1, 0.1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>0.2)
    canvas.use(myrect).translate(20,2.5).rotate(10)
end

rvg.draw.write('Use03.gif')
