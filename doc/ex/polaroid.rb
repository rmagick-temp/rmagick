#! /usr/local/bin/ruby -w
require 'RMagick'
require 'date'

# Demonstrate the Image#polaroid method

img = Magick::Image.read('images/Flower_Hat.jpg').first
img[:Caption] = "\nLosha\n" + Date.today.to_s

begin
    picture = img.polaroid { self.gravity = Magick::CenterGravity }

    # Composite it on a white background so the result is opaque.
    background = Magick::Image.new(picture.columns, picture.rows)
    result = background.composite(picture, Magick::CenterGravity, Magick::OverCompositeOp)

rescue NotImplementedError
    result = Magick::Image.read('images/notimplemented.gif').first
    result.resize!(img.columns, img.rows)
end


result.write('polaroid.jpg')
