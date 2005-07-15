require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm).viewbox(0, 0, 1200, 400) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example ellipse01 - examples of ellipses"

    # Show outline of canvas using rect method
    canvas.rect(1195, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>2)

    canvas.g.translate(300, 200) do |grp|
        grp.ellipse(250, 100).styles(:fill=>'red')
    end

    canvas.g.translate(900, 200).rotate(-30) do |grp|
        grp.ellipse(250, 100).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>20)
    end
end

rvg.draw.write('ellipse01.gif')

