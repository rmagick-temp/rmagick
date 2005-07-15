#! /usr/local/bin/ruby -w
require 'rvg/rvg'


rvg = Magick::RVG.new(200, 100) do |canvas|
    canvas.background_fill = 'white'

    canvas.g.styles(:font_size=>16) do |grp|
        grp.text( 30, 30, ":text_anchor=>'start'").styles(:text_anchor=>'start')
        grp.circle(1, 30, 30).styles(:stroke=>'red')
        grp.text(100, 50, ":text_anchor=>'middle'").styles(:text_anchor=>'middle')
        grp.circle(1, 100, 50).styles(:stroke=>'red')
        grp.text(170, 70, ":text_anchor=>'end'").styles(:text_anchor=>'end')
        grp.circle(1, 170, 70).styles(:stroke=>'red')
    end

    canvas.rect(199, 99).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('text_styles.gif')

