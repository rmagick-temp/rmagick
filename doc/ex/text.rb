#! /usr/local/bin/ruby -w

require 'RMagick'

i = Magick::ImageList.new
i.new_image(190,190) { self.background_color = 'white' }

image = Magick::Draw.new
image.stroke('thistle')
image.fill('transparent')
image.rectangle(0,0, i.columns-1,i.rows-1)
image.draw(i)

sample = Magick::Draw.new
sample.stroke('transparent')
sample.font_family('times')
sample.pointsize(24)

sample.font_style(Magick::NormalStyle)
sample.text(20,40, 'NormalStyle')

sample.font_style(Magick::ItalicStyle)
sample.text(20,70, 'ItalicStyle')

sample.font_style(Magick::ObliqueStyle)
sample.text(20,100, 'ObliqueStyle')

sample.font_style(Magick::NormalStyle)
sample.font_weight(Magick::BoldWeight)
sample.text(20,130, 'BoldWeight')

sample.font_weight(Magick::LighterWeight)
sample.text(20,160, 'LighterWeight')

sample.draw(i)

#i.display
i.write('text.gif')
exit
