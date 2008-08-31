#! /usr/local/bin/ruby -w
require 'RMagick'

img = Magick::Image.read("images/Flower_Hat.jpg").first
rose = Magick::Image.read("images/Yellow_Rose.miff").first
img.affinity(rose)
img.write("affinity.jpg")

