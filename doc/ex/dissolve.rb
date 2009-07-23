#! /usr/local/bin/ruby -w

require 'RMagick'

bgnd = Magick::Image.read('images/Violin.jpg').first
overlay = Magick::Image.read('images/Flower_Hat.jpg').first

# Make the violin image the same size as the hat image
bgnd.resize_to_fill!(overlay.columns, overlay.rows)

composited = bgnd.dissolve(overlay, 0.50)
composited.write('dissolve.jpg')

