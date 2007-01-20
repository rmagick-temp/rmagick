#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(390, 240, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new

# Draw path
gc.fill_opacity 0
gc.stroke 'red'
gc.stroke_width 3
gc.path "M20,120 C20,20 170,20 170,120 S320,220 320,120"

# Annotate
# Show end points
gc.fill_opacity 0
gc.stroke 'gray50'
gc.stroke_width 1
gc.circle  20,120,  23, 123
gc.circle 170,120, 173, 123
gc.circle 320,120, 323, 123

# Show control points
gc.fill_opacity 1
gc.circle  20, 20,  23,  23
gc.circle 170, 20, 173,  23
gc.circle 320,220, 323, 223

# Show connector lines
gc.line  20,120,  20, 20
gc.line 170, 20, 170,220
gc.line 320,220, 320,120

# Show auto control point
gc.fill_opacity 0
gc.stroke 'blue'
gc.stroke_width 3
gc.circle 170,220, 173,223

# Add labels
gc.stroke "none"       # unset stroke color
gc.fill 'black'

# Add end point labels
gc.text  30,125, "'20,120'"
gc.text 180,125, "'170,120'"
gc.text 330,125, "'320,120'"

# Add control point labels
gc.text  30, 25, "'20,20'"
gc.text 180, 25, "'170,20'"
gc.text 330,225, "'320,220'"

# Add auto control point label
gc.text 180,225, "'auto ctl point'"

gc.draw imgl

imgl.border!(1,1, 'lightcyan2')
imgl.write 'path.gif'

