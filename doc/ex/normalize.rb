#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#normalize method

before = Magick::Image.read('images/Hot_Air_Balloons.jpg').first
before.scale!(250.0/before.rows)

after = before.normalize

# Make a before-and-after composite
after.crop!(after.columns/2, 0, after.columns/2, after.rows)
composite = before.composite(after, Magick::EastGravity, Magick::OverCompositeOp)
composite.border!(1,1,'black')

#composite.display
composite.write('normalize.jpg')
exit
