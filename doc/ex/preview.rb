#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read("images/Flower_Hat.jpg").first

begin
    preview = img.preview(Magick::SolarizePreview)
rescue NotImplementedError
    img = Magick::Image.read('images/notimplemented.gif').first
    img.write('preview.jpg')
    exit
end
preview.minify.write('preview.jpg')
exit

