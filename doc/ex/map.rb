#! /usr/local/bin/ruby -w

# Demonstrate the map, append, and composite methods by
# mapping the colors in three separate images into the
# 216 "Web-safe" colors.

require 'RMagick'

# Read three images.
unmapped = Magick::ImageList.new("images/Hot_Air_Balloons.jpg","images/Violin.jpg","images/Polynesia.jpg")

# "Read" the Netscape 216-color cube
map = Magick::ImageList.new "netscape:"

# Map the group of unmapped into the Netscape colors
$stdout.sync = true
printf "Mapping... Please be patient, this may take a few seconds... "
mapped = unmapped.map map, false
puts "Done."

# Use the append method to arrange the unmapped images
# side-by-side into a single image. Repeat for the mapped images.
before = unmapped.append false
before.write 'map_before.jpg'

after = mapped.append false
after.write 'map_after.jpg'
exit
