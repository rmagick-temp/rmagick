#!/home/software/ruby-1.6.8/bin/ruby -w

# Demonstrate the random_channel_threshold method

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first

begin
    img2 = img.random_channel_threshold('intensity', '35%')
rescue NotImplementedError
    img2 = Image.read('images/notimplemented.gif').first
end

img2.write('random_channel_threshold.jpg')
exit
