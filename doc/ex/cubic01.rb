require 'rvg/rvg'

Magick::RVG.dpi = 90

Border       = {:fill=>'none', :stroke=>'blue', :stroke_width=>1}
Connect      = {:fill=>'none', :stroke=>'#888', :stroke_width=>2}
SamplePath   = {:fill=>'none', :stroke=>'red',  :stroke_width=>5}
EndPoint     = {:fill=>'none', :stroke=>'#888', :stroke_width=>2}
CtlPoint     = {:fill=>'#888', :stroke=>'none'}
AutoCtlPoint = {:fill=>'none', :stroke=>'blue', :stroke_width=>4}
Label        = {:font_size=>22, :font_family=>'Verdana'}

rvg = Magick::RVG.new(5.cm, 4.cm).viewbox(0, 0, 500, 400) do |canvas|
    canvas.title = "Example cubic01 - cubic Bezier commands in path data"
    canvas.desc = <<-END_DESC
        Picture showing a simple example of path data using both a
        "C" and an "S" command, along with annotations showing the
        control points and end points.
        END_DESC

    canvas.background_fill = 'white'
    canvas.rect(496, 395, 1, 1).styles(Border)

    canvas.polyline(100,200, 100,100).styles(Connect)
    canvas.polyline(250,100, 250,200).styles(Connect)
    canvas.polyline(250,200, 250,300).styles(Connect)
    canvas.polyline(400,300, 400,200).styles(Connect)

    canvas.path("M100,200 C100,100 250,100 250,200 S400,300 400,200").styles(SamplePath)

    canvas.circle(10, 100, 200).styles(EndPoint)
    canvas.circle(10, 250, 200).styles(EndPoint)
    canvas.circle(10, 400, 200).styles(EndPoint)
    canvas.circle(10, 100, 100).styles(CtlPoint)
    canvas.circle(10, 250, 100).styles(CtlPoint)
    canvas.circle(10, 400, 300).styles(CtlPoint)
    canvas.circle(9, 250, 300).styles(AutoCtlPoint)

    canvas.text(25, 70, 'M100,200 C100,100 250,100 250,200').styles(Label)
    canvas.text(225, 350, 'S400,300 400,200').styles(Label)

end

rvg.draw.write('cubic01.gif')

