#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the mosaic method

a = Magick::ImageList.new

letter = 'A'
26.times do
    # 'M' is not the same size as the other letters.
    if letter != 'M'
        a.read("images/Button_"+letter+".gif")
    end
    letter.succ!
end

# Make a copy of "a" with all the images quarter-sized
b = Magick::ImageList.new
page = Magick::Rectangle.new(0,0,0,0)
a.scene = 0
5.times do |i|
    5.times do |j|
        b << a.scale(0.25)
        page.x = j * b.columns
        page.y = i * b.rows
        b.page = page
        (a.scene += 1) rescue a.scene = 0
    end
end

# Make a 5x5 mosaic
mosaic = b.mosaic
mosaic.write("mosaic.gif")
# mosaic.display
exit
