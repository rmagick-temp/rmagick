require 'rvg/rvg'

rvg = Magick::RVG.new(400, 150) do |canvas|
    canvas.desc = "Example OrigCoordSys - Simple transformations: original picture"
    canvas.background_fill = 'white'

    canvas.g.styles(:fill=>'none', :stroke=>'black', :stroke_width=>3) do |grp|
        # Draw the axes of the original coordinate system
        grp.line(0, 1.5, 400, 1.5)
        grp.line(1.5, 0, 1.5, 150)
    end

    canvas.text(30, 30, 'ABC (orig coord system)').styles(:font_size=>20, :font_family=>'Verdana')

end

rvg.draw.write('OrigCoordSys.gif')

