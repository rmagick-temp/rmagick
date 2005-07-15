require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(12.cm, 6.cm).viewbox(0, 0, 1200, 600) do |canvas|
    canvas.title = "Example quad01 - quadratic Bezier commands in path data"
    canvas.desc = <<-END_DESC
        Picture showing a "Q" a "T" command, along with annotations showing the
        control points and end points
        END_DESC
    canvas.background_fill = 'white'
    canvas.rect(1195, 592, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>1)

    canvas.path("M200,300 Q400,50 600,300 T1000,300").
                styles(:fill=>'none', :stroke=>'red', :stroke_width=>5)

    # End points
    canvas.g.styles(:fill=>'black') do |grp|
        grp.circle(10, 200, 300)
        grp.circle(10, 600, 300)
        grp.circle(10, 1000, 300)
    end

    # Control points and lines from end points to control points
    canvas.g.styles(:fill=>'#888') do |grp|
        grp.circle(10, 400, 50)
        grp.circle(10, 800, 550)
    end

    canvas.path("M200,300 L400,50 L600,300 L800,550 L1000,300").
                styles(:fill=>'none', :stroke=>'#888', :stroke_width=>2)

end

rvg.draw.write('quad01.gif')

