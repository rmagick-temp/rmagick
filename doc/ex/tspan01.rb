require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(10.cm, 3.cm).viewbox(0,0,1000,300) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example tspan01 - using tspan to change visual attributes"
    canvas.g.styles(:font_family=>'Verdana', :font_size=>45) do |grp|
        grp.text(200, 150, "You are").styles(:fill=>'blue') do |txt|
            txt.tspan(" not").styles(:font_weight=>'bold', :fill=>'red')
            txt.tspan(" a banana")
        end
    end
    canvas.rect(997, 297).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('tspan01.gif')

