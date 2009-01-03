require 'rvg/rvg'

Magick::RVG.dpi = 90

PathStyles = {:fill=>'none', :stroke=>'red', :stroke_width=>6}
BaseEllipsesStyles = {:font_size=>20, :font_family=>'Verdana',:fill=>'none', :stroke=>'#888', :stroke_width=>2}

rvg = Magick::RVG.new(12.cm, (5.25).cm).viewbox(0, 0, 1200, 525) do |canvas|
    canvas.title = "Example arcs02 - arc options in paths"
    canvas.desc = <<-END_DESC
        Pictures showing the result of setting
        large-arc-flag and sweep-flag to the four
        possible combinations of 0 and 1.
    END_DESC
    canvas.background_fill = 'white'

    base_ellipses = Magick::RVG::Group.new.styles(BaseEllipsesStyles) do |base|
        base.ellipse(100, 50, 125, 125)
        base.ellipse(100, 50, 225, 75)
        base.text(35, 70, "Arc start")
        base.text(225, 145, "Arc end")
    end

    canvas.rect(1196, 522, 1, 1).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>1)

    canvas.g.styles(:font_size=>30, :font_family=>'Verdana', :font_weight=>'normal', :font_style=>'normal') do |grp|

        grp.use(base_ellipses)

        grp.g.translate(400,0) do |grp2|
            grp2.text(50, 210, 'large-arc-flag=0')
            grp2.text(50, 250, 'sweep-flag=0')
            grp2.use(base_ellipses)
            grp2.path("M 125,75 a100,50 0 0,0 100,50").styles(PathStyles)
        end

        grp.g.translate(800,0) do |grp2|
            grp2.text(50, 210, 'large-arc-flag=0')
            grp2.text(50, 250, 'sweep-flag=1')
            grp2.use(base_ellipses)
            grp2.path("M 125,75 a100,50 0 0,1 100,50").styles(PathStyles)
        end

        grp.g.translate(400, 250) do |grp2|
            grp2.text(50, 210, 'large-arc-flag=1')
            grp2.text(50, 250, 'sweep-flag=0')
            grp2.use(base_ellipses)
            grp2.path("M 125,75 a100,50 0 1,0 100,50").styles(PathStyles)
        end

        grp.g.translate(800, 250) do |grp2|
            grp2.text(50, 210, 'large-arc-flag=1')
            grp2.text(50, 250, 'sweep-flag=1')
            grp2.use(base_ellipses)
            grp2.path("M 125,75 a100,50 0 1,1 100,50").styles(PathStyles)
        end

    end
end

rvg.draw.write('arcs02.gif')
