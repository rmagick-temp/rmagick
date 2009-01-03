#! /usr/local/bin/ruby -w
require 'rvg/rvg'


rvg = Magick::RVG.new(200, 250) do |canvas|
    canvas.background_fill = 'white'

    canvas.g do |grp|
        grp.g.styles(:font_weight=>'normal', :font_style=>'normal') do |grp2|
          grp2.text(10, 30, "default size")
          grp2.text(10, 50, ":font_size=>14").styles(:font_size=>14)
          grp2.text(10, 70, ":font_size=>16").styles(:font_size=>16)
          grp2.text(10,100, ":font_size=>24").styles(:font_size=>24)
        end
    end

    canvas.g.styles(:font_size=>14, :font_weight=>'normal', :font_style=>'normal') do |grp|
      if RUBY_PLATFORM =~ /mswin32/
         grp.text( 8, 120, ":font_family=>'Courier-New'").styles(:font_family=>'Courier-New')
      else
         grp.text( 8, 120, ":font_family=>'Courier'").styles(:font_family=>'Courier')
      end
        grp.text(10, 140, ":font_weight=>'bold'").styles(:font_weight=>'bold')
        grp.text(10, 160, ":font_stretch=>'normal'").styles(:font_stretch=>'normal')
        grp.text(10, 180, ":font_stretch=>'condensed'").styles(:font_stretch=>'condensed')
        grp.text(10, 200, ":font_style=>'italic'").styles(:font_style=>'italic')
        grp.text(10, 220, ":font_weight=>900").styles(:font_weight=>900)
    end

    canvas.rect(199, 249).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('font_styles.gif')

