#! /usr/local/bin/ruby -w
#
# Demonstrate the Image#sample! method

require 'RMagick'

img = Magick::Image.read("images/Cheetah.jpg").first
img.sample!(img.columns*0.25, img.rows*0.25)
#img.display
img.write("sample.jpg")
exit
