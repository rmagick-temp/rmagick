#! /usr/local/bin/ruby -w
require 'RMagick'

f = Magick::Image.read("../doc/ex/images/Flower_Hat.jpg").first
pixels = f.dispatch(0,0,f.columns, f.rows, "RGB")
image = Magick::Image.constitute(f.columns, f.rows, "RGB", pixels)
image.write("constitute.miff")
