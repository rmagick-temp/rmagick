#! /usr/local/bin/ruby -w

require 'RMagick'

pr = Magick::Draw.new

# Outline the image
pr.stroke('lavender')
pr.fill_opacity(0)
pr.stroke_width(1)
pr.rectangle(0,0,249,199)

# Define a clip-path.
pr.define_clip_path('example') {    # The name of the clip-path is "example"
    pr.polygon(125,37.5,    139.5,80.5, 184.5,80.5,  148.5,107.5,
               161.5,150.5, 125,125,    88.5,150.5,  101.5,107.5,
               65.5,80.5,   110.5,80.5)
    }                               # End the clip-path definition

pr.clip_path('example')             # Enable the clip-path

# Composite the Balloon Girl image over
# the background using the clip-path
bg = Magick::ImageList.new "images/Balloon_Girl.jpg"
pr.composite(0,0, 250,200, bg)

# Just for illustration, outline the clip-path in blue.
pr.stroke('blue')
pr.stroke_width(2)
pr.fill_opacity(0)
pr.polygon(125,37.5,    139.5,80.5, 184.5,80.5,  148.5,107.5,
           161.5,150.5, 125,125,    88.5,150.5,  101.5,107.5,
           65.5,80.5,   110.5,80.5)

# Create a canvas to draw on
img = Magick::ImageList.new
img.new_image(250,200) { self.background_color = 'transparent' }

# Execute the primitives
pr.draw(img)
#img.display
img.write("clip_path.gif")

exit
