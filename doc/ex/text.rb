#! /usr/local/bin/ruby -w

require 'RMagick'

imgl = Magick::ImageList.new
imgl.new_image(190,190)

sample = Magick::Draw.new
sample.stroke('transparent')
if RUBY_PLATFORM =~ /mswin32/
	sample.font_family('Georgia')
else
	sample.font_family('times')
end
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

sample.draw(imgl)

imgl.write('text.gif')
exit
