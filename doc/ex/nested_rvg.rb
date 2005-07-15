#! /usr/local/bin/ruby -w

require 'rvg/rvg'

target = Magick::RVG.new.viewbox(0,0,200,200) do |targ|
    targ.g.styles(:stroke_width=>20, :stroke=>'#ff5600', :fill=>'#abd600') do |grp|
        grp.circle(90, 100, 100)
        grp.circle(60, 100, 100)
        grp.circle(30, 100, 100)
    end
end

rvg = Magick::RVG.new(258, 100) do |canvas|
    canvas.background_fill = '#51396b'
    canvas.use(target, 0, 0, 100, 100)
    canvas.use(target, 100, 16.6667, 66.7, 66.7)
    canvas.use(target, 166.6667, 25, 50, 50)
    canvas.use(target, 216.6667, 30, 40, 40)
end

rvg.draw.write('nested_rvg.gif')
