#! /usr/local/bin/ruby -w
#
# Demonstrate the Image#thumbnail method, new in ImageMagick 5.5.2
# This method is faster than Image#resize when the thumbnail is 
# < 1/10 the size of the original image.

require 'RMagick'

img = Magick::Image.read("images/Cheetah.jpg").first 
begin
    thumbnail = img.thumbnail(img.columns*0.09, img.rows*0.09)

# Substitute the standard "Not Implemented" image
rescue NotImplementedError
    thumbnail = Magick::Image.read("images/notimplemented.gif").first
end
#thumbnail.display
thumbnail.write("thumbnail.jpg")
exit
