require 'rvg/rvg'

def example(cols, rows)
    rvg = Magick::RVG.new(cols, rows) do |canvas|
        canvas.background_fill = 'white'
        canvas.desc = <<-'END_DESC'
            Example ViewBox - uses the viewBox attribute to automatically create an
            initial user coordinate system which causes the graphic to scale to fit
            into the viewport no matter what size the viewport is.
            END_DESC

        canvas.viewbox(0, 0, 1500, 1000)
        canvas.preserve_aspect_ratio('none')

        # This rectangle goes from (0,0) to (1500,1000) in user space.
        # Because of the viewBox attribute above,
        # the rectangle will end up filling the entire area
        # reserved for the SVG content.
        canvas.rect(1500, 1000).styles(:fill=>'yellow',:stroke=>'blue',:stroke_width=>12)

        # A large, red triangle
        canvas.path("M 750,100 L 250,900 L 1250,900 z").styles(:fill=>'red')

        # A text string that spans most of the viewport
        canvas.text(100, 600, 'Stretch to fit').styles(:font_size=>200, :font_style=>'normal', :font_weight=>'normal', :font_family=>'Verdana')

    end
    return rvg.draw
end

example(300, 200).write('ViewBox_300x200.gif')
example(150, 200).write('ViewBox_150x200.gif')

