#! /usr/local/bin/ruby -w
require 'rvg/rvg'
include Magick


rvg = RVG.new(200, 250) do |canvas|
    canvas.background_fill = 'white'

    canvas.g do |grp|
        grp.text(20, 20, "default size")
        grp.text(20, 40, ":font_size=>14").styles(:font_size=>14)
        grp.text(20, 60, ":font_size=>16").styles(:font_size=>16)
        grp.text(20, 90, ":font_size=>24").styles(:font_size=>24)
    end

    canvas.g.styles(:font_size=>14) do |grp|
        grp.text(18, 110, ":font_family=>'Courier'").styles(:font_family=>'Courier')
        grp.text(20, 130, ":font_weight=>'bold'").styles(:font_weight=>'bold')
        grp.text(20, 150, ":font_stretch=>'normal'").styles(:font_stretch=>'normal')
        grp.text(20, 170, ":font_stretch=>'condensed'").styles(:font_stretch=>'condensed')
        grp.text(20, 190, ":font_style=>'italic'").styles(:font_style=>'italic')
        grp.text(20, 210, ":font_weight=>900").styles(:font_weight=>900)
    end

    canvas.rect(199, 249).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('font_styles.gif')

