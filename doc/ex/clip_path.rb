#! /usr/local/bin/ruby -w

require 'RMagick'

points = [145, 65, 174,151, 264,151, 192,205,
          218,291, 145,240,  72,291,  98,205,
           26,151, 116,151]

pr = Magick::Draw.new

# Define a clip-path.
# The name of the clip-path is "example"
pr.define_clip_path('example') {
    pr.polygon(*points)
    }

# Enable the clip-path
pr.push
pr.clip_path('example')

# Composite the Flower Hat image over
# the background using the clip-path
girl = Magick::ImageList.new
girl.read("images/Flower_Hat.jpg")

cols = rows = nil

# Our final image is about 280 pixels wide, so here
# we widen our picture to fit. The change_geometry
# method will adjust the height proportionately.

girl.change_geometry("280") do |c,r|
    pr.composite(0,0, c, r, girl)
    cols = c
    rows = r
end

pr.pop

# Create a canvas to draw on, a bit bigger than the star.
canvas = Magick::Image.new(cols, rows)

star = Magick::Draw.new
star.stroke('gray50')
star.fill('gray50')
points.map! {|p| p + 8}
star.polygon(*points)
star.draw(canvas)
canvas = canvas.blur_image(0, 3)

# Draw the star shadow over the background
pr.draw(canvas)

# Crop away all the solid white border pixels.
crop = canvas.bounding_box
canvas.crop!(crop.x, crop.y, crop.width, crop.height)

canvas.write("clip_path.gif")

exit
