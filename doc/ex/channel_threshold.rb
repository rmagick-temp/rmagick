#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#channel_threshold method

img = Magick::Image.read('images/Blonde_with_dog.jpg').first
img = img.scale(250.0/img.rows)

# Channel threshold values should be a %-age of MaxRGB
# Let the opacity threshold default to MaxRGB.
img2 = img.channel_threshold(Magick::MaxRGB*0.75,
                             Magick::MaxRGB*0.50)
img2['Label'] = "channel_threshold(\nMagick::MaxRGB*0.75,\nMagick::MaxRGB*0.50)"

img3 = img.channel_threshold(Magick::MaxRGB*0.50,
                             Magick::MaxRGB*0.25,
                             Magick::MaxRGB*0.25)
img3['Label'] = 'channel_threshold(\nMagick::MaxRGB*0.50,\nMagick::MaxRGB*0.25,\nMagick::MaxRGB*0.25)'

# Montage the two sample images
imgs = Magick::ImageList.new
imgs << img2 << img3

montage = imgs.montage {
    self.background_color = "white"
    self.geometry = "#{img.columns}x#{img.rows}+10+10"
    self.tile = "2x1"
    }

# Give the montage a black border
montage.border!(1,1,'black')

#montage.display
montage.write('channel_threshold.jpg')
exit
