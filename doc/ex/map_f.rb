#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#map method

grandma = Magick::Image.read('images/Grandma.jpg')[0]
grandma.scale!(250.0/grandma.rows)

# "Read" the builtin Netscape format, which
# contains the 216 colors in the Netscape palette.
nsmap = Magick::Image.read('netscape:')[0]

# Map the original image colors into the Netscape colors.
after = grandma.map(nsmap)

# For a change of pace, let's composite the lower half of
# the "after" image atop the lower half of the "before" image.
after.crop!(0, after.rows/2, after.columns, after.rows/2)
grandma = grandma.composite(after, Magick::SouthGravity, Magick::OverCompositeOp)

# Draw a line across the middle to highlight the before and after parts
line = Magick::Draw.new
line.line(0, grandma.rows/2, grandma.columns, grandma.rows/2)
line.draw(grandma)

#grandma.display
grandma.write('map_f.jpg')
