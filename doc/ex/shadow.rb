require 'RMagick'

# Draw a big red Bezier curve on a transparent background.
img = Magick::Image.new(340, 120) {self.background_color = 'none'}
gc = Magick::Draw.new
gc.fill('none')
gc.stroke('red')
gc.stroke_linecap('round')
gc.stroke_width(10)
gc.bezier(20, 60, 20,-90, 320,210, 320,60)
gc.draw(img)

# Composite it onto a white background, draw a border around the result,
# write it to the "before" file.
before = img.copy
bg = Magick::Image.new(before.columns, before.rows) {self.background_color='white'}
before = bg.composite(before, Magick::CenterGravity, Magick::OverCompositeOp)
before.border!(1,1,'gray80')
before.write('shadow_before.gif')

# Create the shadow.
shadow = img.shadow()
# Composite the original image over the shadow, composite the result
# onto a white background, add a border, write it to the "after" file.
shadow = shadow.composite(img, Magick::NorthWestGravity, Magick::OverCompositeOp)
bg = Magick::Image.new(shadow.columns, shadow.rows) {self.background_color='white'}
after = bg.composite(shadow, Magick::CenterGravity, Magick::OverCompositeOp)
after.border!(1,1,'gray80')

after.write('shadow_after.gif')

