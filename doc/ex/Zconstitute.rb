#! /usr/local/bin/ruby -w
require 'RMagick'

require 'pixels.rb'

puts 'This may take a few seconds...'
image = Magick::Image.constitute(Width, Height, "RGB", Pixels)
#image.display
image.write("constitute.gif")
exit
