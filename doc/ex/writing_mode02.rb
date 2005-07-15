require 'rvg/rvg'

Magick::RVG.dpi = 90

TEXT_STYLES = {:writing_mode=>'lr',
               :glyph_orientation_horizontal=>0,
               :fill=>'red4',
               :font_weight=>'bold',
               :font_size=>16}

TEXT_STYLES2 = {:writing_mode=>'lr',
               :glyph_orientation_horizontal=>180,
               :fill=>'green',
               :font_weight=>'bold',
               :font_size=>16}

rvg = Magick::RVG.new(3.in, 1.in).viewbox(0,0,300,100) do |canvas|
    canvas.background_fill = 'white'

    canvas.text(15, 40, ":glyph_orientation_horizontal=0").styles(TEXT_STYLES)
    canvas.text(15, 80, ":glyph_orientation_horizontal=180").styles(TEXT_STYLES2)

    canvas.rect(299, 99).styles(:fill=>'none',:stroke=>'blue')
end

rvg.draw.write('writing_mode02.gif')
