require 'rvg/rvg'

Magick::RVG.dpi = 90

rvg = Magick::RVG.new(10.cm, 3.cm).viewbox(0,0,1000,300) do |canvas|
    canvas.background_fill = 'white'
    canvas.desc = "Example text01 - 'Hello, out there' in blue"
    canvas.text(250, 150, "Hello, out there").
                styles(:font_family=>'Verdana', :font_size=>55, :fill=>'blue')
    canvas.circle(5, 250, 150).styles(:fill=>'red')

    # Show outline of canvas using 'rect' element
    canvas.rect(997, 297).styles(:fill=>'none', :stroke=>'blue')
end

rvg.draw.write('text01.gif')

