#! /usr/local/bin/ruby -w

# Demonstrate the map, append, and composite methods by
# mapping the colors in three separate images into the
# 216 "Web-safe" colors.

require 'RMagick'

group = Magick::ImageList.new

# Read three images.
snapshots = Magick::ImageList.new("images/Hot_Air_Balloons.jpg","images/Violin.jpg","images/Polynesia.jpg")

# Scale each image to 250 pixels high & proportionately wide
snapshots.each { |shot|
    group << shot.scale(250.0/shot.rows)
    }

# "Read" the Netscape 216-color cube
map = Magick::ImageList.new "netscape:"

# Map the group of snapshots into the Netscape colors
puts "Mapping... This may take a few seconds..."
mapped = group.map map, false

# Use the append method to arrange the unmapped images
# side-by-side into a single image. Repeat for the mapped images.
old = group.append false
new = mapped.append false

# Show before & after on the same image.
# Crop the top half of the "after mapping" images away.
half_height = old.rows / 2
new.crop! 0, half_height, new.columns, half_height

# Composite the "after" images over the "before" images.
demo = old.composite new, 0, half_height, Magick::OverCompositeOp

# Draw a black line across the middle to help
# distinquish "before" (top) and "after" (bottom)
line = Magick::Draw.new
line.line 0, demo.rows/2, demo.columns, demo.rows/2
line.stroke "black"
line.draw demo

#demo.display
puts "Writing map.jpg..."
demo.write "map.jpg"
exit
