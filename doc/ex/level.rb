#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#level method

before = Magick::Image.read('images/Flower_Hat.jpg').first

# Allow the white-point argument to default.
# Brighten up the mid-tones a bit.
after = before.level(0,1.50)
after.write('level.jpg')
exit
