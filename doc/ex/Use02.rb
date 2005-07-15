require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(10.cm, 3.5.cm).viewbox(0, 0, 100, 35) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example Use02 - Chain 'styles' to 'use'"
    r = Magick::RVG::Group.new do |grp|
            grp.rect(60, 10).styles(:fill=>'yellow')
        end
    canvas.rect(99.6, 34.6, 0.1, 0.1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>0.2)

    # Since the rectangle specified the fill color the :fill style is ignored here.
    # However, since the rectangle did not specify a stroke color, the :stroke style
    # specified here is respected.
    canvas.use(r, 20, 5).styles(:fill=>'green', :stroke=>'red')
    canvas.use(r, 20, 20).styles(:fill=>'green', :stroke=>'blue')
end

rvg.draw.write('Use02.gif')

