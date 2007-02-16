#!/home/software/ruby-1.6.8/bin/ruby -w

# Demonstrate the random_channel_threshold method

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first

geom = Geometry.new(MaxRGB/2)
img2 = img.random_threshold_channel(geom, RedChannel)

img2.write('random_threshold_channel.jpg')
exit
