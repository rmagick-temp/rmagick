require 'rvg/rvg'

Magick::RVG::dpi = 90


rvg = Magick::RVG.new(10.cm, 3.cm).viewbox(0,0,1000,300) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example tspan02 - using tspan's dx and dy attributes for incremental positioning adjustments"
    canvas.g.styles(:font_family=>'Verdana', :font_size=>45) do |_g|
        _g.text(200, 150, "But you").styles(:fill=>'blue') do |txt|
            txt.tspan("are").d(100, -50).styles(:font_weight=>'bold', :fill=>'red')
            txt.tspan(" a peach!").d(0, 100)
        end
    end
    canvas.rect(996, 296, 1, 1).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('tspan02.gif')

