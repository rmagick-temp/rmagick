require 'rvg/rvg'

rvg = Magick::RVG.new(400, 120) do |canvas|
    canvas.desc = 'Example RotateScale - Rotate and scale transforms'
    canvas.background_fill = 'white'
    canvas.g.styles(:fill=>'none', :stroke=>'black', :stroke_width=>3) do |grp|
        # Draw the axes of the original coordinate system
        grp.line(0, 1.5, 400, 1.5)
        grp.line(1.5, 0, 1.5, 120)
    end
    # Establish a new coordinate system whose origin is at (50,30)
    # in the original coord. system and which is rotated by 30 degrees.
    canvas.g.translate(50,30) do |grp|
        grp.g.rotate(30) do |grp2|
            grp2.g.styles(:fill=>'none',:stroke=>'red',:stroke_width=>3) do |grp3|
                grp3.line(0, 0, 50, 0)
                grp3.line(0, 0, 0, 50)
            end
            grp2.text(0, 0, "ABC (rotate)").styles(:font_size=>20, :fill=>'blue', :font_family=>'Verdana')
        end
    end

    # Establish a new coordinate system whose origin is at (200,40)
    # in the original coord. systm and which is scaled by 1.5
    canvas.g.translate(200,40) do |grp|
        grp.g.scale(1.5) do |grp2|
            grp2.g.styles(:fill=>'none',:stroke=>'red',:stroke_width=>3) do |grp3|
                grp3.line(0, 0, 50, 0)
                grp3.line(0, 0, 0, 50)
            end
            grp2.text(0, 0, "ABC (scale)").styles(:font_size=>20, :fill=>'blue', :font_family=>'Verdana')
        end
    end
end

rvg.draw.write('RotateScale.gif')

