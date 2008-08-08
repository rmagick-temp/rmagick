require 'RMagick'

img = Magick::Image.read("images/Flower_Hat.jpg").first
begin
  img = img.level_colors("green", "orange", true)
  img.alpha(Magick::DeactivateAlphaChannel)
rescue NotImplementedError
  not_imp = Magick::Image.read('images/notimplemented.gif').first
  img = not_imp.resize(img.columns, img.rows)
end
img.write("level_colors.jpg")
