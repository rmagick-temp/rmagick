#! /usr/local/bin/ruby -w
require 'RMagick'

i = Magick::ImageList.new
i.new_image 500, 400 do self.background_color = "white" end

primitives = Magick::Draw.new

# Draw path
primitives.fill_opacity 0
primitives.stroke 'red'
primitives.stroke_width 3
primitives.path "M100,200 C100,100 250,100 250,200 S400,300 400,200"

# Annotate
# Show end points
primitives.fill_opacity 0
primitives.stroke 'gray50'
primitives.stroke_width 1
primitives.circle 100,200, 103, 203
primitives.circle 250,200, 253, 203
primitives.circle 400,200, 403, 203

# Show control points
primitives.fill_opacity 1
primitives.circle 100,100, 103, 103
primitives.circle 250,100, 253, 103
primitives.circle 400,300, 403, 303

# Show connector lines
primitives.line 100,200, 100,100
primitives.line 250,300, 250,100
primitives.line 400,200, 400,300

# Show auto control point
primitives.fill_opacity 0
primitives.stroke 'blue'
primitives.stroke_width 3
primitives.circle 250,300, 253,303

# Add labels
primitives.stroke "#000000ff"       # unset stroke color
primitives.fill 'black'

# Add end point labels
primitives.text 110,205, "'100,200'"
primitives.text 260,205, "'250,200'"
primitives.text 410,205, "'400,200'"

# Add control point labels
primitives.text 110,105, "'100,100'"
primitives.text 260,105, "'250,100'"
primitives.text 410,305, "'400,300'"

# Add auto control point label
primitives.text 260,305, "'auto ctl point'"

# Outline
primitives.stroke('lavender')
primitives.stroke_width(1)
primitives.fill_opacity(0)
primitives.rectangle(0,0, 499, 399)

primitives.draw i

#i.display
i.write 'path.gif'
