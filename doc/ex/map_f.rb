#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#map method

img = Magick::Image.read('images/Flower_Hat.jpg')[0]

# "Read" the builtin Netscape format, which
# contains the 216 colors in the Netscape palette.
nsmap = Magick::Image.read('netscape:')[0]

# Map the original image colors into the Netscape colors.
after = img.map(nsmap)

after.write('map_f.jpg')
