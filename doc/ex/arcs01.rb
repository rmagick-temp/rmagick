require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 5.25.cm).viewbox(0, 0, 1200, 400) do |canvas|
    canvas.title = "Example arcs01 - arc commands in path data"
    canvas.desc = <<-END_DESC
        Picture of a pie chart with two pie wedges and a picture
        of a line with arc blips
    END_DESC
    canvas.background_fill = 'white'
    canvas.rect(1196, 395, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>1)

    canvas.path("M300,200 h-150 a150,150 0 1,0 150,-150 z").
                styles(:fill=>'red', :stroke=>'blue', :stroke_width=>5)
    canvas.path("M275,175 v-150 a150,150 0 0,0 -150,150 z").
                styles(:fill=>'yellow', :stroke=>'blue', :stroke_width=>5)
    canvas.path(<<-END_PATH
           M600,350 l 50,-25
           a25,25 -30 0,1 50,-25 l 50,-25
           a25,50 -30 0,1 50,-25 l 50,-25
           a25,75 -30 0,1 50,-25 l 50,-25
           a25,100 -30 0,1 50,-25 l 50,-25
           END_PATH
           ).styles(:fill=>'none', :stroke=>'red', :stroke_width=>5)
end

rvg.draw.write('arcs01.gif')
