#!/home/software/ruby-1.6.8/bin/ruby -w

# Demonstrate the random_channel_threshold method

require 'RMagick'
include Magick

img = Image.read('images/Balloon_Girl.jpg').first
img.resize!(200.0/img.rows)

begin
    img2 = img.random_channel_threshold('intensity', '35%')
    img2.crop!(img2.columns/2, 0, img2.columns/2, img2.rows)
    result = img.composite(img2, EastGravity, OverCompositeOp)
rescue NotImplementedError
    result = Image.read('images/notimplemented.gif').first
end

#result.display
result.write('random_channel_threshold.jpg')
exit
