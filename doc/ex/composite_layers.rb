#! /usr/local/bin/ruby -w
require 'RMagick'

class Magick::ImageList
  # Create a shadow image for each image in the list
  def shadow(x_offset = 4, y_offset = 4, sigma = 4.0, opacity = 1.0)
    return collect { |frame| frame.shadow(x_offset, y_offset, sigma, opacity) }
  end
end

ruby = Magick::ImageList.new

# Draw a rotating "Ruby" animation
gc = Magick::Draw.new
gc.gravity = Magick::CenterGravity
gc.pointsize = 24
gc.font_weight = Magick::BoldWeight
gc.fill = "darkred"
gc.stroke = "black"
gc.stroke_width = 1

23.times do
  ruby << Magick::Image.new(100, 100) {self.background_color = "none"}
  gc.annotate(ruby, 0, 0, 0, 0, "Ruby")
  gc.rotation = 15
end

# Create a gradient background
bg = Magick::ImageList.new
bg.new_image(99, 99, Magick::GradientFill.new(50, 50, 50, 50, "white", "tan"))
bg.border!(1, 1, "black")

# Create the animated shadows of the rotating "Ruby" animation
shadows = ruby.shadow(2, 5, 3)

# Composite the shadow animation over the background. Since there is only one
# background image, it will replicated for each frame in the shadow animation.
begin
  result = bg.composite_layers(shadows)

  # Composite the "Ruby" animation over the previous composite
  result = result.composite_layers(ruby)
  result.delay = 10
  result.write("composite_layers.gif")
  result[0].write("composite_layers1.gif")

rescue NotImplementedError
    result = Magick::Image.read('images/notimplemented.gif').first
    result.resize!(100, 100)
    result.write("composite_layers.gif")
    result.write("composite_layers1.gif")
end
exit
