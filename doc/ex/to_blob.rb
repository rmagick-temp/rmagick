#! /usr/local/bin/ruby -w
require 'RMagick'

f = Magick::Image.read("images/small.tif").first
blob = f.to_blob
format = f.format
rows = f.rows
cols = f.columns
depth = f.depth

f2 = Magick::Image.from_blob(blob) {
    self.format = format
    self.size = Magick::Geometry.new(rows, cols)
    self.depth = depth
    }
#f2[0].display
f2[0].write("to_blob.gif")
exit

