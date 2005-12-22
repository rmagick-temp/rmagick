#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the crop_resize method

img = Magick::Image.read('images/Flower_Hat.jpg')[0]
thumbnail = img.crop_resized(76, 76)
thumbnail.write("crop_resized.jpg")


