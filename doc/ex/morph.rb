#! /usr/local/bin/ruby -w

require 'RMagick'

# Demonstrate the morph method

# Read 4 digit image files. Create an
# animated morph sequence by inserting 8
# in-between images between each pair of digits.

i = Magick::ImageList.new
number = '0'
4.times do
    i.read "images/Button_" + number + ".gif"
    number.succ!
    end

puts "This may take a few seconds..."
morph = i.morph 8
morph.delay = 12
morph.iterations = 10000
# Display the resulting sequence as an animation.
# morph.animate(12)
morph.write "morph.gif"
exit

