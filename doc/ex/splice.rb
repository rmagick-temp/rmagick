#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Demonstrate the splice method.

img = Image.read('images/Flower_Hat.jpg').first

geom = Geometry.new(20,20, img.columns/2, img.rows/2)

spliced_img = img.splice(geom, 'red')
spliced_img.write('splice.jpg')

