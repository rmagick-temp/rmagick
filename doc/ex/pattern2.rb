#! /usr/local/bin/ruby -w

require 'RMagick'

# "Read" the ImageListMagick built-in rose image.
rose = Magick::ImageList.new("rose:")

# Construct a pattern using the rose
gc = Magick::Draw.new
gc.pattern('rose', 0, 0, rose.columns, rose.rows) {
    gc.composite(0, 0, 0, 0, rose)
}

# Set the fill to the rose pattern. Draw an ellipse
gc.fill('rose')
gc.ellipse(150, 75, 140, 70, 0, 360)

# Create a canvas to draw on
i = Magick::ImageList.new
i.new_image(300, 150, Magick::HatchFill.new('white','gray90',8))

# Draw the ellipse using the rose fill
gc.draw(i)

#i.display
i.write('pattern2.gif')
exit
