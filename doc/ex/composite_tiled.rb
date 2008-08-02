require 'RMagick'

# Create a transparent image to tile over the background image.
wm = Magick::Image.read("xc:none") { self.size = "100x50" }.first

# Draw "RMagick" in semi-transparent text on the transparent image.
gc = Magick::Draw.new
gc.fill '#ffffff7f'
gc.font_weight Magick::BoldWeight
gc.font_size 18
gc.rotate 15
gc.gravity Magick::CenterGravity
gc.text 0, 0, "RMagick"
gc.draw wm

# Read the background image.
img = Magick::Image.read("images/Flower_Hat.jpg").first

# Composite the tile image over the background image.
img.composite_tiled! wm
img.write "composite_tiled.jpg"


