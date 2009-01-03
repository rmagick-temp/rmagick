require 'rvg/rvg'

Magick::RVG.dpi = 90

Border       = { :fill=>'none', :stroke=>'blue', :stroke_width=>1 }
Connect      = { :fill=>'none', :stroke=>'#888888', :stroke_width=>2 }
SamplePath   = { :fill=>'none', :stroke=>'red', :stroke_width=>5 }
EndPoint     = { :fill=>'none', :stroke=>'#888888', :stroke_width=>2 }
CtlPoint     = { :fill=>'#888888', :stroke=>'none' }
AutoCtlPoint = { :fill=>'none', :stroke=>'blue', :stroke_width=>4 }
Label        = { :text_anchor=>'middle', :font_size=>22, :font_family=>'Verdana', :font_weight=>'normal', :font_style=>'normal' }

rvg = Magick::RVG.new(10.cm, 10.cm).viewbox(0, 0, 1000, 1000) do |canvas|
    canvas.title = 'Example cubic02 - cubic Bezier commands in path data'
    canvas.desc = <<-END_DESC
        Picture showing examples of "C" and "S" commands,
        along with annotations showing the control points
        and end points
        END_DESC

    canvas.background_fill = 'white'
    canvas.rect(996, 996, 1, 1).styles(Border)

    # Path 1
    canvas.polyline(100,200, 100,100).styles(Connect)
    canvas.polyline(400,100, 400,200).styles(Connect)
    canvas.path('M100,200 C100,100 400,100 400,200').styles(SamplePath)
    canvas.circle(10, 100, 200).styles(EndPoint)
    canvas.circle(10, 400, 200).styles(EndPoint)
    canvas.circle(10, 100, 100).styles(CtlPoint)
    canvas.circle(10, 400, 100).styles(CtlPoint)
    canvas.text(250, 275, 'M100,200 C100,100 400,100 400,200').styles(Label)

    # Path 2
    canvas.polyline(100,500, 25,400).styles(Connect)
    canvas.polyline(475,400, 400,500).styles(Connect)
    canvas.path("M100,500 C25,400 475,400 400,500").styles(SamplePath)
    canvas.circle(10, 100, 500).styles(EndPoint)
    canvas.circle(10, 400, 500).styles(EndPoint)
    canvas.circle(10, 25, 400).styles(CtlPoint)
    canvas.circle(10, 475, 400).styles(CtlPoint)
    canvas.text(250, 575, "M100,500 C25,400 475,400 400,500").styles(Label)

    # Path 3
    canvas.polyline(100,800, 175,700).styles(Connect)
    canvas.polyline(325,700, 400,800).styles(Connect)
    canvas.path("M100,800 C175,700 325,700 400,800").styles(SamplePath)
    canvas.circle(10, 100, 800).styles(EndPoint)
    canvas.circle(10, 400, 800).styles(EndPoint)
    canvas.circle(10, 175, 700).styles(CtlPoint)
    canvas.circle(10, 325, 700).styles(CtlPoint)
    canvas.text(250, 875, "M100,800 C175,700 325,700 400,800").styles(Label)

    # Path 4
    canvas.polyline(600,200, 675,100).styles(Connect)
    canvas.polyline(975,100, 900,200).styles(Connect)
    canvas.path("M600,200 C675,100 975,100 900,200").styles(SamplePath)
    canvas.circle(10, 600, 200).styles(EndPoint)
    canvas.circle(10, 900, 200).styles(EndPoint)
    canvas.circle(10, 675, 100).styles(CtlPoint)
    canvas.circle(10, 975, 100).styles(CtlPoint)
    canvas.text(750, 275, "M600,200 C675,100 975,100 900,200").styles(Label)

    # Path 5
    canvas.polyline(600,500, 600,350).styles(Connect)
    canvas.polyline(900,650, 900,500).styles(Connect)
    canvas.path("M600,500 C600,350 900,650 900,500").styles(SamplePath)
    canvas.circle(10, 600, 500).styles(EndPoint)
    canvas.circle(10, 900, 500).styles(EndPoint)
    canvas.circle(10, 600, 350).styles(CtlPoint)
    canvas.circle(10, 900, 650).styles(CtlPoint)
    canvas.text(750, 575, "M600,500 C600,350 900,650 900,500").styles(Label)

    # Path 6 (C and S command)
    canvas.polyline(600,800, 625,700).styles(Connect)
    canvas.polyline(725,700, 750,800).styles(Connect)
    canvas.polyline(750,800, 775,900).styles(Connect)
    canvas.polyline(875,900, 900,800).styles(Connect)
    canvas.path("M600,800 C625,700 725,700 750,800 S875,900 900,800").styles(SamplePath)
    canvas.circle(10, 600, 800).styles(EndPoint)
    canvas.circle(10, 750, 800).styles(EndPoint)
    canvas.circle(10, 900, 800).styles(EndPoint)
    canvas.circle(10, 625, 700).styles(CtlPoint)
    canvas.circle(10, 725, 700).styles(CtlPoint)
    canvas.circle(10, 875, 900).styles(CtlPoint)
    canvas.circle(10, 775, 900).styles(AutoCtlPoint)
    canvas.text(750, 945, "M600,800 C625,700 725,700 750,800").styles(Label)
    canvas.text(750, 975, "S875,900 900,800").styles(Label)

end

rvg.draw.write('cubic02.gif')


