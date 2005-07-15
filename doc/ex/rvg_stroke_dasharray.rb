#! /usr/local/bin/ruby -w
require 'rvg/rvg'

rvg = Magick::RVG.new(200, 100) do |canvas|
            canvas.background_fill = 'white'
            canvas.rect(150, 50, 25, 25).round(6).
                    styles(:fill=>'none', :stroke=>'purple', :stroke_width=>10, :stroke_dasharray=>[10,5])
            canvas.rect(199, 99).styles(:fill=>'none', :stroke=>'blue')
      end

rvg.draw.write('rvg_stroke_dasharray.gif')

