#! /usr/local/bin/ruby -w
require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 4.cm).viewbox(0, 0, 1200, 400) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example fillrule - nonzero - demonstrates fill_rule=>'nonzero'"

    canvas.rect(1195, 393, 1, 1).styles(:fill=>'none', :stroke=>'blue')

    triangle = Magick::RVG::Group.new do |defs|
        defs.path("M 16,0 L -8,9 v-18 z").styles(:fill=>'black', :stroke=>'none')
    end

    canvas.g.styles(:clip_rule=>'evenodd', :fill=>'red', :stroke=>'black', :stroke_width=>3) do |grp|
        grp.path("M 250,75 L 323,301 131,161 369,161 177,301 z")
        grp.use(triangle).translate(306.21, 249).rotate(72)
        grp.use(triangle).translate(175.16,193.2).rotate(216)
        grp.use(triangle).translate(314.26,161).rotate(0)
        grp.use(triangle).translate(221.16,268.8).rotate(144)
        grp.use(triangle).translate(233.21,126.98).rotate(288)
        grp.path("M 600,81 A 107,107 0 0,1 600,295 A 107,107 0 0,1 600,81 z" +
                 "M 600,139 A 49,49 0 0,1 600,237 A 49,49 0 0,1 600,139 z")
        grp.use(triangle).translate(600,188).rotate(0).translate(107,0).rotate(90)
        grp.use(triangle).translate(600,188).rotate(120).translate(107,0).rotate(90)
        grp.use(triangle).translate(600,188).rotate(240).translate(107,0).rotate(90)
        grp.use(triangle).translate(600,188).rotate(60).translate(49,0).rotate(90)
        grp.use(triangle).translate(600,188).rotate(180).translate(49,0).rotate(90)
        grp.use(triangle).translate(600,188).rotate(300).translate(49,0).rotate(90)
        grp.path("M 950,81 A 107,107 0 0,1 950,295 A 107,107 0 0,1 950,81 z" +
                 "M 950,139 A 49,49 0 0,0 950,237 A 49,49 0 0,0 950,139 z")
        grp.use(triangle).translate(950,188).rotate(0).translate(107,0).rotate(90)
        grp.use(triangle).translate(950,188).rotate(120).translate(107,0).rotate(90)
        grp.use(triangle).translate(950,188).rotate(240).translate(107,0).rotate(90)
        grp.use(triangle).translate(950,188).rotate(60).translate(49,0).rotate(-90)
        grp.use(triangle).translate(950,188).rotate(180).translate(49,0).rotate(-90)
        grp.use(triangle).translate(950,188).rotate(300).translate(49,0).rotate(-90)
    end
end

rvg.draw.write('evenodd.gif')

