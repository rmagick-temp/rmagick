require 'rvg/rvg'

rvg = Magick::RVG.new(525, 270) do |canvas|
    canvas.background_fill = 'white'
    canvas.rect(524, 269).styles(:fill=>'none', :stroke=>'blue', :stroke_width=>1)

    hat = ::Magick::Image.read('images/Flower_Hat.jpg').first

    canvas.image(hat, 100, 75, 25, 40).preserve_aspect_ratio('none')
    canvas.rect(100, 75, 25, 40).styles(:fill => 'none', :stroke => 'blue')

    canvas.image(hat, 100, 75, 150, 40).preserve_aspect_ratio('xMinYMin', 'meet')
    canvas.rect(100, 75, 150, 40).styles(:fill => 'none', :stroke => 'blue')

    canvas.image(hat, 100, 75, 275, 40).preserve_aspect_ratio('xMidYMid', 'meet')
    canvas.rect(100, 75, 275, 40).styles(:fill => 'none', :stroke => 'blue')

    canvas.image(hat, 100, 75, 400, 40).preserve_aspect_ratio('xMaxYMax', 'meet')
    canvas.rect(100, 75, 400, 40).styles(:fill => 'none', :stroke => 'blue')

    canvas.image(hat, 100, 75, 150, 155).preserve_aspect_ratio('xMinYMin', 'slice')
    canvas.rect(100, 75, 150, 155).styles(:fill => 'none', :stroke => 'blue')

    canvas.image(hat, 100, 75, 275, 155).preserve_aspect_ratio('xMidYMid', 'slice')
    canvas.rect(100, 75, 275, 155).styles(:fill => 'none', :stroke => 'blue')

    canvas.image(hat, 100, 75, 400, 155).preserve_aspect_ratio('xMaxYMax', 'slice')
    canvas.rect(100, 75, 400, 155).styles(:fill => 'none', :stroke => 'blue')

    canvas.line(150, 20, 305, 20)
    canvas.line(345, 20, 500, 20)
    canvas.line(150, 250, 305, 250)
    canvas.line(340, 250, 500, 250)
    canvas.g.styles(:stroke=>'none') do |t|
        t.text(310, 23, 'meet')
        t.text(60, 140, 'none')
        t.text(170, 140, 'xMinYMin')
        t.text(300, 140, 'xMidYMid')
        t.text(425, 140, 'xMaxYMax')
        t.text(310, 253, 'slice')
    end

end

rvg.draw.write('image.gif')

