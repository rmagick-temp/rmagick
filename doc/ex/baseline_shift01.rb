require 'rvg/rvg'

rvg = Magick::RVG.new(150, 100) do |canvas|
    canvas.background_fill = 'white'
    canvas.text(40, 35).styles(:font_weight=>'bold', :font_size=>28) do |txt|
        txt.tspan('H')
        txt.tspan('2').styles(:font_size=>20, :fill=>'red', :baseline_shift=>'sub')
        txt.tspan('O')
    end
    canvas.text(40, 80).styles(:font_style=>'italic', :font_size=>28) do |txt|
        txt.tspan('e=mc')
        txt.tspan('2').styles(:font_size=>20, :fill=>'red', :baseline_shift=>'super')
    end
    canvas.rect(149, 99).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('baseline_shift01.gif')

