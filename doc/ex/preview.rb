#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read("images/Blonde_with_dog.jpg").first

begin
    preview = img.preview(Magick::SolarizePreview)
rescue NotImplementedError
    img = Image.read('images/notimplemented.gif').first
    img.write('preview.jpg')
    exit
end
preview.minify.write('preview.jpg')
exit

