require 'rvg/rvg'

rvg = Magick::RVG.new(300, 100) do |canvas|
    canvas.desc = "Example InitialCoords - SVG's initial coordinate system"
    canvas.background_fill = 'white'
    canvas.g.styles(:fill=>'none', :stroke=>'black', :stroke_width=>3) do |grp|
        grp.line(0, 1.5, 300, 1.5)
        grp.line(1.5, 0, 1.5, 100)
    end
    canvas.g.styles(:fill=>'red', :stroke=>'none') do |grp|
        grp.rect(3, 3)
        grp.rect(3, 3, 297,  0)
        grp.rect(3, 3,   0, 97)
    end
    canvas.g.styles(:font_size=>14, :font_family=>'Verdana', :font_weight=>'normal', :font_style=>'normal') do |grp|
        grp.text(10, 20, '(0,0)')
        grp.text(240, 20, '(300,0)')
        grp.text(10, 90, '(0,100)')
    end
end

rvg.draw.write('InitialCoords.gif')

