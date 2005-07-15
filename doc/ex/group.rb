require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(6.cm, 4.cm).viewbox(0,0,600,400) do |canvas|
    canvas.background_fill = 'white'
    canvas.rect(595,395,1,1).styles(:stroke=>'blue', :fill=>'none', :stroke_width=>2)

    # Define a stick figure.
    stick = Magick::RVG::Group.new.styles(:stroke=>'black', :fill=>'none', :stroke_width=>6) do |fig|
        fig.circle(42, 50, 50).styles(:stroke=>'red')
        fig.polyline(30,40, 50,40, 45,60, 50,40, 65,40).styles(:stroke_width=>4)
        fig.polyline(10,100, 50,130, 90,100)
        fig.line(50,97, 50, 155)
        fig.polyline(10,200, 50,155, 90,200)
    end

    # Draw 12 copies.
    2.times {|y|
        6.times {|x|
            canvas.use(stick, x*100, y*200)
        }
    }
end

rvg.draw.write('group.gif')
