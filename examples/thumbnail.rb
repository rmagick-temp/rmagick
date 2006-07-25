require 'RMagick'
include Magick

puts <<END_INFO

This example demonstrates how to make a thumbnail of an image.
An image is resized to the target size (retaining its original
aspect ratio, of course) and then "mounted" on a square background
with raised edges.

Usage:

    ruby thumbnail.rb <filename <size>>

where `filename' is the name of an image file and `size' is the
size of the thumbnail in pixels. The default size is 120 pixels.
If you don't specify any arguments this script uses a default
image.

END_INFO

DEFAULT_SIZE = 120

case ARGV.length
    when 2
        size = ARGV[1].to_i
        image = ARGV[0]
    when 1
        image = ARGV[0]
        size = DEFAULT_SIZE
    else
        size = DEFAULT_SIZE
        image = "../doc/ex/images/Flower_Hat.jpg"
end

geom = "#{size}x#{size}"

# Read the image and resize it. The `change_geometry' method
# computes the new image geometry and yields to a block. The
# return value of the block is the return value of the method.

img = Image.read(image)[0]
img.change_geometry!(geom) { |cols, rows| img.thumbnail! cols, rows }

# We need a background to display the thumbnail.
# Create a square, neutral gray background with raised edges.
# Make this background slightly larger than the image to allow
# for the raised border. A 3-pixel raised edge means that the
# background needs to be 6 pixels larger in each dimension.

bg = Image.new(size+6, size+6) { self.background_color = "gray75" }
bg = bg.raise(3,3)

# Just for the purposes of this example, display the thumbnail background on
# a larger white background.

white_bg = Image.new(size+50, size+50) {self.background_color = "white"}
white_bg = white_bg.composite(bg, CenterGravity, OverCompositeOp)

# Finally, center the thumbnail on the gray background.
thumbnail = white_bg.composite(img, CenterGravity, OverCompositeOp)

thumbnail.write("thumbnail.gif")
exit

