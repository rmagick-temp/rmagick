#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image(600, 300) {self.background_color = 'white'}

primitives = Magick::Draw.new

# Draw path with quadratic bezier commands
primitives.fill_opacity 0
primitives.stroke 'red'
primitives.stroke_width 3
primitives.path "M100,150 Q200,25 300,150 T500,150"

# Annotate
# Show end points
primitives.fill_opacity 1
primitives.stroke 'grey50'
primitives.stroke_width 1
primitives.circle 100,150, 103, 153
primitives.circle 300,150, 303, 153
primitives.circle 500,150, 503, 153

# Show control points
primitives.fill 'black'
primitives.stroke 'transparent'
primitives.circle 200,25, 203, 28
primitives.circle 400,275, 403, 278

# Show connector lines
primitives.line 100,150, 200,25
primitives.line 200,25, 400,275
primitives.line 400,275, 500,150

# Add labels
# Add end point labels
primitives.text 110,155, "'100,150'"
primitives.text 310,155, "'300,150'"
primitives.text 510,155, "'500,150'"

# Add control point labels
primitives.text 210,30, "'200,25'"
primitives.text 410,280, "'400,275'"

# Outline
primitives.stroke 'gray50'
primitives.stroke_width 1
primitives.fill_opacity 0
primitives.rectangle 0,0, 599, 299

primitives.draw i

#i.display
i.write "qbezierpath.gif"
