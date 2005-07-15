#! /usr/local/bin/ruby -w
require 'rvg/rvg'

rvg = Magick::RVG.new(150, 150) do |canvas|
    canvas.background_fill = ''
    canvas.circle(40, 75, 75).styles(:stroke=>'blue', :fill=>'#00ff00',:stroke_width=>8)
    canvas.rect(149,149).styles(:fill=>'none',:stroke=>'blue')
end

rvg.draw.write('stroke_fill.gif')

