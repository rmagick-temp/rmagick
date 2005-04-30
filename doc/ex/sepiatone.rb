#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first

begin
    sepiatone = img.sepiatone(MaxRGB * 0.8)
rescue NotImplementedError
    sepiatone = Image.read('images/notimplemented.gif').first
end

sepiatone.write('sepiatone.jpg')

