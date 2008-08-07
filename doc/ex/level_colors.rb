require 'RMagick'

img = Magick::Image.read("images/Flower_Hat.jpg").first
img = img.level_colors("green", "orange", true)
img.alpha(Magick::DeactivateAlphaChannel)
img.write("level_colors.jpg")
