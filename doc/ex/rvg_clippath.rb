require 'rvg/rvg'
include Magick

hat = Image.read('images/Flower_Hat.jpg').first
rvg = RVG.new(hat.columns, hat.rows) do |canvas|
    keyhole = RVG::ClipPath.new do |path|
        path.circle(60, canvas.width/2, 80)
        path.polygon(canvas.width/2-10, 60, 40, 230, 160, 230, canvas.width/2+10, 60)
    end
    canvas.image(hat, nil, nil, 20, 20).styles(:clip_path=>keyhole)

end

rvg.draw.write('rvg_clippath.gif')

