#! /usr/local/bin/ruby -w

# Demonstrate the ImageList#quantize method

require 'RMagick'

snapshots = Magick::ImageList.new "images/Ballerina.jpg","images/Gold_Statue.jpg","images/Shorts.jpg"
ssnaps = Magick::ImageList.new          # Scaled snapshots
snapshots.each { |snap|
    ssnaps << snap.scale(250.0/snap.rows)
    }

# Quantize all 3 images to a single set of 16 colors in the RGB colorspace
puts "Quantizing... Please be patient. This may take a couple of seconds."
quant = ssnaps.quantize 16

# Now we show the "before" and "after"...
# Arrange the original images side-by-side into a
# single image. Repeat for the quantized images.
old = ssnaps.append false
new = quant.append false

# Crop the top half of the quantized "after" images
# away. Composite the remainder over the "before" images.
half_height = old.rows / 2
new.crop! 0, half_height, new.columns, half_height

demo = old.composite new, 0, half_height, Magick::OverCompositeOp

# Draw a black line across the middle to help
# distinquish "before" (top) and "after" (bottom)
line = Magick::Draw.new
line.line 0, demo.rows/2, demo.columns, demo.rows/2
line.stroke "black"
line.draw demo

#demo.display
puts "Writing quantize-m.jpg..."
demo.write "quantize-m.jpg"
exit
