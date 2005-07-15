require 'rvg/rvg'

Magick::RVG.dpi = 90
Fill = %w{yellow pink green blue cyan red purple brown}

rvg = Magick::RVG.new(6.cm, 6.cm).viewbox(0,0,600,600) do |canvas|
    canvas.background_fill = 'white'

    ref_text = Magick::RVG::Tspan.new("Referenced").styles(:font_size=>52, :font_weight=>'bold')

    canvas.g.translate(300,270) do |grp|
        angle = 0
        8.times do |n|
            grp.text do |txt|
                txt.tref(ref_text).d(0,30).rotate(angle).styles(:fill=>Fill[n])
                angle += 45
            end
        end
    end

    canvas.rect(596,596).styles(:fill=>'none',:stroke=>'blue')

end

rvg.draw.write('tref01.gif')
