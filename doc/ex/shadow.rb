require 'RMagick'
include Magick

img = Image.new(340, 120) {self.background_color = 'none'}
gc = Draw.new
gc.fill('none')
gc.stroke('red')
gc.stroke_linecap('round')
gc.stroke_width(10)
gc.bezier(20, 60, 20,-90, 320,210, 320,60)
gc.draw(img)

img.write('shadow_before.gif')
shadow = img.shadow()
shadow = shadow.composite(img, NorthWestGravity, OverCompositeOp)
bg = Image.new(shadow.columns, shadow.rows) {self.background_color='white'}
shadow = bg.composite(shadow, CenterGravity, OverCompositeOp)
shadow.write('shadow_after.gif')
