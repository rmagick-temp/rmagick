#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the "path" drawing primitive.

i = Magick::ImageList.new
i.new_image(600, 200)  {self.background_color = 'white'}

primitives = Magick::Draw.new

# Draw "pie chart"
primitives.fill 'red'
primitives.stroke 'blue'
primitives.stroke_width 2
primitives.path 'M150,100 h-75 a75,75 0 1,0 75,-75 z'
primitives.fill 'yellow'
primitives.path 'M137.5,87.5 v-75 a75,75 0 0,0 -75,75 z'

# Draw wiggly line
primitives.fill_opacity 0
primitives.stroke 'red'
primitives.stroke_width 3
primitives.path 'M300,175 l 25,-12.5 ' +
                'a12.5,12.5 -15 0,1 25,-12.5 l 25,-12.5 ' +
                'a12.5,25   -15 0,1 25,-12.5 l 25,-12.5 ' +
                'a12.5,37.5 -15 0,1 25,-12.5 l 25,-12.5 ' +
                'a12.5,50   -15 0,1 25,-12.5 l 25,-12.5'

# Outline
primitives.stroke('gray50')
primitives.stroke_width(1)
primitives.rectangle(0,0, 599, 199)

primitives.draw i
#i.display
i.write 'arcpath.gif'
