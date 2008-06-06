require 'RMagick'

source = Magick::Image.read('images/Flower_Hat.jpg').first
background = Magick::Image.read('images/Hot_Air_Balloons.jpg').first
mask = Magick::Image.new(background.columns, background.rows)

# Make a mask
gc = Magick::Draw.new
gc.annotate(mask, 0, 0, 0, 0, "Ruby") do
  gc.gravity = Magick::CenterGravity
  gc.pointsize = 100
  gc.rotation = 90
  gc.font_weight = Magick::BoldWeight
end

background.add_compose_mask(mask)
result = background.composite(source, Magick::CenterGravity, Magick::OverCompositeOp)
result.write "compose_mask_example.jpg"
mask.write "compose_mask.jpg"

