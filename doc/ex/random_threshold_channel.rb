#!/home/software/ruby-1.6.8/bin/ruby -w

# Demonstrate the random_channel_threshold method

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first

begin
    geom = Geometry.new(MaxRGB/2)
    img2 = img.random_threshold_channel(g, RedChannel)
rescue NotImplementedError
    img2 = Image.read('images/notimplemented.gif').first
end

img2.write('random_threshold_channel.jpg')
exit
