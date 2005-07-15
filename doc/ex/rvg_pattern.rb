require 'rvg/rvg'

rvg = Magick::RVG.new(300, 300) do |canvas|
    canvas.background_fill = 'white'
    triangles = Magick::RVG::Pattern.new(16, 16).viewbox(0,0, 50,50) do |pat|
        pat.rect(50, 50).styles(:fill=>'darkblue')
        pat.polygon(0,0, 25,50, 50,0, 0,0).styles(:fill=>'yellow', :stroke=>'red')
    end

    canvas.ellipse(130, 60, 150, 75).styles(:stroke_width=>16, :fill=>'none', :stroke=>triangles)

    hat = Magick::Image.read('images/Flower_Hat.jpg').first

    hats = Magick::RVG::Pattern.new(hat.columns/4.0, hat.rows/4.0) do |pat|
        pat.image(hat, hat.columns/4.0, hat.rows/4.0)
    end

    canvas.g.translate(0, 150) do |grp|
        grp.ellipse(130, 60, 150, 75).styles(:fill=>hats)
    end

    canvas.rect(299, 299, 0, 0).styles(:fill=>'none', :stroke=>'blue')
    canvas.line(1, 150, 299, 150)
end

rvg.draw.write('rvg_pattern.gif')
