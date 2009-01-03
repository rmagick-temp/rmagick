require 'rvg/rvg'

rvg = Magick::RVG.new(400, 150) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = 'Example NewCoordSys - New user coordinate system'
    canvas.g.styles(:fill=>'none', :stroke=>'black', :stroke_width=>3) do |grp|
        # Draw the axes of the original coordinate system
        grp.line(0, 1.5, 400, 1.5)
        grp.line(1.5, 0, 1.5, 150)
    end

    canvas.g do |grp|
        grp.text(30, 30, 'ABC (orig coord system)').styles(:font_size=>20, :font_family=>'Verdana', :font_weight=>'normal', :font_style=>'normal')
    end

    # Establish a new coordinate system, which is
    # shifted (i.e., translated) from the initial coordinate
    # system by 50 user units along each axis.
    canvas.g.translate(50, 50) do |grp|
        grp.g.styles(:fill=>'none', :stroke=>'red', :stroke_width=>3) do |grp2|
            # Draw lines of length 50 user units along
            # the axes of the new coordinate system
            grp2.line(0, 0, 50, 0)
            grp2.line(0, 0, 0, 50)
        end
        grp.text(30, 30, 'ABC (translated coord system)').styles(:font_size=>20, :font_family=>'Verdana', :font_weight=>'normal', :font_style=>'normal')
    end

end

rvg.draw.write('NewCoordSys.gif')

