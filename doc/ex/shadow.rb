require 'RMagick'
include Magick

# Draw a big red Bezier curve on a transparent background.
img = Image.new(340, 120) {self.background_color = 'none'}
gc = Draw.new
gc.fill('none')
gc.stroke('red')
gc.stroke_linecap('round')
gc.stroke_width(10)
gc.bezier(20, 60, 20,-90, 320,210, 320,60)
gc.draw(img)

# Composite it onto a white background, draw a border around the result,
# write it to the "before" file.
before = img.copy
bg = Image.new(before.columns, before.rows) {self.background_color='white'}
before = bg.composite(before, CenterGravity, OverCompositeOp)
before.border!(1,1,'gray80')
before.write('shadow_before.gif')

begin
    # Create the shadow.
    shadow = img.shadow()
    # Composite the original image over the shadow, composite the result
    # onto a white background, add a border, write it to the "after" file.
    shadow = shadow.composite(img, NorthWestGravity, OverCompositeOp)
    bg = Image.new(shadow.columns, shadow.rows) {self.background_color='white'}
    after = bg.composite(shadow, CenterGravity, OverCompositeOp)
    after.border!(1,1,'gray80')
rescue NotImplementedError
    after = Image.read('images/notimplemented.gif').first
    after.resize!(before.columns, before.rows)
end

after.write('shadow_after.gif')

