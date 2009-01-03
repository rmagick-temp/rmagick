#! /usr/local/bin/ruby -w
require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(500, 300, Magick::HatchFill.new('white','lightcyan2'))

gc = Magick::Draw.new

# Draw path with quadratic bezier commands
gc.fill_opacity 0
gc.stroke 'red'
gc.stroke_width 3
gc.path "M20,150 Q120,25 220,150 T420,150"

# Annotate
# Show end points
gc.fill_opacity 1
gc.stroke 'grey50'
gc.stroke_width 1
gc.circle 20,150,   23, 153
gc.circle 220,150, 223, 153
gc.circle 420,150, 423, 153

# Show control points
gc.fill 'black'
gc.stroke 'transparent'
gc.circle 120,25,  123, 28
gc.circle 320,275, 323, 278

# Show connector lines
gc.line  20,150, 120,25
gc.line 120,25,  320,275
gc.line 320,275, 420,150

# Add labels
gc.font_weight Magick::NormalWeight
gc.font_style Magick::NormalStyle

# Add end point labels
gc.text  30,155, "'20,150'"
gc.text 230,155, "'220,150'"
gc.text 430,155, "'420,150'"

# Add control point labels
gc.text 130,30, "'120,25'"
gc.text 330,280, "'320,275'"

gc.draw imgl

imgl.border!(1,1, "lightcyan2")
#imgl.display
imgl.write "qbezierpath.gif"
