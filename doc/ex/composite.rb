#! /usr/local/bin/ruby -w
require 'RMagick'

CompositeOps = {Magick::OverCompositeOp => 'OverCompositeOp',
                Magick::OutCompositeOp => 'OutCompositeOp',
                Magick::PlusCompositeOp => 'PlusCompositeOp',
                Magick::DifferenceCompositeOp => 'DifferenceCompositeOp',
                Magick::CopyRedCompositeOp => 'CopyRedCompositeOp',
                Magick::DisplaceCompositeOp => 'DisplaceCompositeOp'}

bg = Magick::Image.read('images/Hot_Air_Balloons_H.jpg').first
bg.resize!(0.50)
fg = Magick::Image.new(bg.columns, bg.rows)

text = Magick::Draw.new
text.pointsize = 48
text.fill = 'blue'
text.stroke = 'transparent'
text.text_antialias = false
text.font_weight = Magick::BoldWeight
text.gravity = Magick::CenterGravity

text.annotate(fg, 0,0,0,0, 'RMagick')
fg = fg.matte_replace(0,0)

examples = Magick::ImageList.new

CompositeOps.each { |composite_op, op_name|
    composite = bg.composite(fg.copy, Magick::CenterGravity, composite_op)
    composite['Label'] = op_name
    examples << composite
}

montage = examples.montage {
    self.tile = '2x3'
    self.geometry = "#{examples.columns}x#{examples.rows}+10+5"
    self.background_color = 'black'
    self.fill = 'white'
    self.stroke = 'transparent'
}

#montage.display

# Write a full-size version.
montage.write('composite_big.jpg')

# Write a half-size version for the web page.
montage.minify.write('composite.jpg')
exit

exit
