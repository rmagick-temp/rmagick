#! /usr/local/bin/ruby -w
require 'RMagick'

load 'pixels-array'

image = Magick::Image.constitute(Width, Height, "RGB", Pixels)

image.write("constitute.jpg")
exit
