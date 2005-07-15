require 'rvg/rvg'

rvg = Magick::RVG.new(400, 120) do |canvas|
    canvas.desc = 'Example Skew - Show effects of skewX and skewY'
    canvas.background_fill = 'white'

    canvas.g.styles(:fill=>'none',:stroke=>'black',:stroke_width=>3) do |grp|
        # Draw the axes of the original coordinate system
        grp.line(0, 1.5, 400, 1.5)
        grp.line(1.5, 0, 1.5, 120)
    end

    # Establisha new coordinate system whose origin is at (30,30)
    # in the initial coord. system and which is skewed in X by 30 degrees.
    canvas.g.translate(30,30) do |grp|
        grp.skewX(30) do |grp2|
            grp2.g.styles(:fill=>'none', :stroke=>'red', :stroke_width=>3) do |grp3|
                grp3.line(0, 0, 50, 0)
                grp3.line(0, 0, 0, 50)
            end
            grp.text(0, 0, 'ABC (skewX)').styles(:font_size=>20, :fill=>'blue', :font_family=>'Verdana')
        end
    end

    # Establish a new coordinate system whose origin is at (200,30)
    # in the initial coord. system and which is skewed in Y by 30 degrees.
    canvas.g.translate(200,30) do |grp|
        grp.skewY(30) do |grp2|
            grp2.g.styles(:fill=>'none', :stroke=>'red', :stroke_width=>3) do |grp3|
                grp3.line(0, 0, 50, 0)
                grp3.line(0, 0, 0, 50)
            end
            grp.text(0, 0, 'ABC (skewY)').styles(:font_size=>20, :fill=>'blue', :font_family=>'Verdana')
        end
    end
end

rvg.draw.write('Skew.gif')
