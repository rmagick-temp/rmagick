require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(10.cm, 3.cm).viewbox(0,0,1000,300) do |canvas|
    canvas.background_fill = 'white'

    canvas.g.translate(100, 60) do |grp|
        grp.text.styles(:font_family=>'Verdana', :font_size=>45) do |txt|
            txt.tspan("Rotation")
            txt.tspan(" propogates").rotate(20).styles(:fill=>'red') do |tsp|
                tsp.tspan(" to descendents").styles(:fill=>'green')
            end
        end
    end
    canvas.rect(997, 297).styles(:fill=>'none', :stroke=>'blue')

end

rvg.draw.write('tspan03.gif')

