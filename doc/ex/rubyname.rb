#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the annotate method

Text = 'RMagick'

granite = Magick::ImageList.new('granite:')
canvas = Magick::ImageList.new
canvas.new_image(300, 100, Magick::TextureFill.new(granite))

text = Magick::Draw.new
text.pointsize = 52
text.gravity = Magick::CenterGravity

text.annotate(canvas, 0,0,2,2, Text) {
    self.fill = 'gray83'
}

text.annotate(canvas, 0,0,-1.5,-1.5, Text) {
    self.fill = 'gray40'
}

text.annotate(canvas, 0,0,0,0, Text) {
    self.fill = 'darkred'
}

#canvas.display
canvas.write('rubyname.gif')
exit
