require 'rvg/rvg'

Magick::RVG.dpi = 90

TEXT_STYLES = {:writing_mode=>'tb',
               :glyph_orientation_vertical=>0,
               :fill=>'red4',
               :font_weight=>'bold',
               :font_size=>16}

TEXT_STYLES2 = {:writing_mode=>'tb',
               :glyph_orientation_vertical=>90,
               :fill=>'green',
               :font_weight=>'bold',
               :font_size=>16}

rvg = Magick::RVG.new(1.25.in, 7.in).viewbox(0,0,125,700) do |canvas|
    canvas.background_fill = 'white'

    canvas.text(40, 15, ":glyph_orientation_vertical=0").styles(TEXT_STYLES)
    canvas.text(80, 25, ":glyph_orientation_vertical=90").styles(TEXT_STYLES2)

    canvas.rect(124, 698).styles(:fill=>'none',:stroke=>'blue')
end

rvg.draw.write('writing_mode01.gif')

