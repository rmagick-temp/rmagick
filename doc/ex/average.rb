#! /usr/local/bin/ruby -w
# Demonstrate ImageList#average method
require 'RMagick'

images = Magick::ImageList.new("images/Button_A.gif", "images/Button_B.gif", "images/Button_C.gif")

group = images.append false
group.compression = Magick::LZWCompression
group.write "average_before.gif"

average_image = images.average

average_image.compression = Magick::LZWCompression
average_image.write "average_after.gif"
exit
