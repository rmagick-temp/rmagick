#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#shave method

dog = Magick::Image.read('images/Dog2.jpg').first
dog.scale!(250.0/dog.rows)

after = dog.shave(dog.columns/10, dog.rows/10)
#after.display
dog.write('shave1.jpg')
after.write('shave2.jpg')
exit
