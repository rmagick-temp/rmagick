#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the crop_resize method

img = Magick::Image.read('images/Flower_Hat.jpg')[0]
thumbnail = img.resize_to_fill(76, 76)
thumbnail.write("resize_to_fill.jpg")


