#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#rotate method

dog = Magick::Image.read('images/Dog2.jpg').first
dog.resize!(250.0/dog.rows)

dog.rotate!(45)

# Make the corners transparent
dog = dog.matte_replace(0,0)
#dog.display
dog.write('rotate_f.jpg')
exit
