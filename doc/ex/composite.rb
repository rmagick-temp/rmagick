#! /usr/local/bin/ruby -w

# Demonstrate the effects of various composite operators.
# Based on ImageMagick's composite test.

require 'RMagick'
include Magick

ROWS = 70
COLS = 70
COLOR_A = "#999966"
COLOR_B = "#990066"

img = Image.new(COLS, ROWS)
triangle = Draw.new
triangle.fill(COLOR_A)
triangle.stroke('transparent')
triangle.polygon(0,0, COLS,0, 0,ROWS, 0,0)
triangle.draw(img)
image_A = img.transparent('white', TransparentOpacity)
image_A['Label'] = 'A'

img = Image.new(COLS, ROWS)
triangle = Draw.new
triangle.fill(COLOR_B)
triangle.stroke('transparent')
triangle.polygon(0,0, COLS,ROWS, COLS,0, 0,0)
triangle.draw(img)
image_B = img.transparent('white', TransparentOpacity)
image_B['Label'] = 'B'

list = ImageList.new
null = Image.read("xc:white") { self.size = Geometry.new(COLS,ROWS) }
null = null.first.transparent('white', TransparentOpacity)
null.border_color = 'transparent'
granite =  Image.read("granite:")

list << null.copy
list << image_A
list << image_B
list << null.copy


list << image_B.composite(image_A, CenterGravity, OverCompositeOp)
list.cur_image['Label'] = 'A over B'
list << image_A.composite(image_B, CenterGravity, OverCompositeOp)
list.cur_image['Label'] = 'B over A'

list << image_B.composite(image_A, CenterGravity, InCompositeOp)
list.cur_image['Label'] = 'A in B'
list << image_A.composite(image_B, CenterGravity, InCompositeOp)
list.cur_image['Label'] = 'B in A'

list << image_B.composite(image_A, CenterGravity, OutCompositeOp)
list.cur_image['Label'] = 'A out B'
list << image_A.composite(image_B, CenterGravity, OutCompositeOp)
list.cur_image['Label'] = 'B out A'

list << image_B.composite(image_A, CenterGravity, AtopCompositeOp)
list.cur_image['Label'] = 'A atop B'
list << image_A.composite(image_B, CenterGravity, AtopCompositeOp)
list.cur_image['Label'] = 'B atop A'

list << image_B.composite(image_A, CenterGravity, XorCompositeOp)
list.cur_image['Label'] = 'A xor B'

list << image_B.composite(image_A, CenterGravity, MultiplyCompositeOp)
list.cur_image['Label'] = 'A multiply B'

list << image_B.composite(image_A, CenterGravity, ScreenCompositeOp)
list.cur_image['Label'] = 'A screen B'

list << image_B.composite(image_A, CenterGravity, DarkenCompositeOp)
list.cur_image['Label'] = 'A darken B'

list << image_B.composite(image_A, CenterGravity, LightenCompositeOp)
list.cur_image['Label'] = 'A lighten B'

list << image_B.composite(image_A, CenterGravity, PlusCompositeOp)
list.cur_image['Label'] = 'A plus B'

list << image_B.composite(image_A, CenterGravity, MinusCompositeOp)
list.cur_image['Label'] = 'A minus B'

list << image_B.composite(image_A, CenterGravity, AddCompositeOp)
list.cur_image['Label'] = 'A add B'

list << image_B.composite(image_A, CenterGravity, SubtractCompositeOp)
list.cur_image['Label'] = 'A subtract B'

list << image_B.composite(image_A, CenterGravity, DifferenceCompositeOp)
list.cur_image['Label'] = 'A difference B'

list << image_B.composite(image_A, CenterGravity, HueCompositeOp)
list.cur_image['Label'] = 'A hue B'

list << image_B.composite(image_A, CenterGravity, SaturateCompositeOp)
list.cur_image['Label'] = 'A saturate B'

list << image_B.composite(image_A, CenterGravity, LuminizeCompositeOp)
list.cur_image['Label'] = 'A luminize B'

list << image_B.composite(image_A, CenterGravity, ColorizeCompositeOp)
list.cur_image['Label'] = 'A colorize B'

list << image_B.composite(image_A, CenterGravity, BumpmapCompositeOp)
list.cur_image['Label'] = 'A bumpmap B'

list << image_B.composite(image_A, CenterGravity, DissolveCompositeOp)
list.cur_image['Label'] = 'A dissolve B'

list << image_B.composite(image_A, CenterGravity, ThresholdCompositeOp)
list.cur_image['Label'] = 'A threshold B'

list << image_B.composite(image_A, CenterGravity, ModulateCompositeOp)
list.cur_image['Label'] = 'A modulate B'

list << image_A.composite(image_B, CenterGravity, ModulateCompositeOp)
list.cur_image['Label'] = 'B modulate A'

list << image_B.composite(image_A, CenterGravity, OverlayCompositeOp)
list.cur_image['Label'] = 'A overlay B'

montage = list.montage {
    self.geometry = Geometry.new(COLS, ROWS, 3, 3)
    rows = (list.size+3) / 4
    self.tile = Geometry.new(4, rows)
    self.texture = granite[0]
    self.fill = 'white'
    self.stroke = 'transparent'
}

montage.write('composite.gif')
exit

