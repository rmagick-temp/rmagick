#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#level_channel method

before = Magick::Image.read('images/Jean_Jacket.jpg').first
before.scale!(250.0/before.rows)
                                     
# Let the final argument default
begin
    red   = before.level_channel(Magick::RedChannel,  Magick::MaxRGB,  2)
    green = before.level_channel(Magick::GreenChannel,Magick::MaxRGB/2,2)
    blue  = before.level_channel(Magick::BlueChannel, Magick::MaxRGB/3,2)

    # Composite the green image over the middle of the red image.
    # Composite the blue image over the right-hand side of the red image.
    green.crop!(red.columns/3, 0, red.columns/3, red.rows)
    blue.crop!(red.columns/3*2, 0, red.columns/3, red.rows)
    result = red.composite(green, Magick::CenterGravity, Magick::OverCompositeOp)
    result = result.composite(blue, Magick::EastGravity, Magick::OverCompositeOp)

    # Draw a black line between each of the three images.
    line = Magick::Draw.new
    line.line(result.columns/3, 0, result.columns/3, result.rows)
    line.line(result.columns/3*2, 0, result.columns/3*2, result.rows)
    line.draw(result)
    
# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    result = Magick::Image.read("images/notimplemented.gif").first
end

#result.display
result.write('level_channel.jpg')
exit
