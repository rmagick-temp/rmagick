#! /usr/local/bin/ruby -w
require 'rvg/rvg'

FONT_STYLES = {:font_size=>20, :font_weight=>'bold', :fill=>'white'}

rvg = Magick::RVG.new(450, 150) do |canvas|
    canvas.background_fill = 'white'
    canvas.rect(400, 50, 25, 50)
    canvas.circle(40, 100, 75).styles(:opacity=>0.25, :stroke=>'blue', :fill=>'#00ff00',:stroke_width=>8)
    canvas.text(83, 83, '0.25').styles(FONT_STYLES)
    canvas.circle(40, 225, 75).styles(:opacity=>0.50, :stroke=>'blue', :fill=>'#00ff00',:stroke_width=>8)
    canvas.text(208, 83, '0.50').styles(FONT_STYLES)
    canvas.circle(40, 350, 75).styles(:opacity=>0.75, :stroke=>'blue', :fill=>'#00ff00',:stroke_width=>8)
    canvas.text(333, 83, '0.75').styles(FONT_STYLES)
    canvas.rect(449,149).styles(:fill=>'none',:stroke=>'blue')
end

rvg.draw.write('rvg_opacity.gif')

