require 'RMagick'

background = Magick::Image.read('images/Flower_Hat.jpg').first
source = Magick::Image.read('pattern:checkerboard') {self.size = "#{background.columns}x#{background.rows}"}.first
mask = Magick::Image.new(background.columns, background.rows) {self.background_color = "black"}

# Make a mask
gc = Magick::Draw.new
gc.annotate(mask, 0, 0, 0, 0, "Ruby") do
  gc.gravity = Magick::CenterGravity
  gc.pointsize = 100
  gc.rotation = 90
  gc.font_weight = Magick::BoldWeight
  gc.fill = "white"
  gc.stroke = "none"
end

background.add_compose_mask(mask)
result = background.composite(source, Magick::CenterGravity, Magick::OverCompositeOp)
result.write "compose_mask_example.jpg"
source.write "compose_mask_source.gif"
mask.write "compose_mask.gif"

