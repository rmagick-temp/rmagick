#! /usr/local/bin/ruby -w
# Demonstrate ImageList#average method
require 'RMagick'

i = Magick::ImageList.new("images/Button_A.gif", "images/Button_B.gif", "images/Button_C.gif")
f = i.average
f.write("average.gif")
exit
