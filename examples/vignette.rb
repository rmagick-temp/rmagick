require 'RMagick'
include Magick

puts <<END_INFO

    This example adds a soft-edged vignette to a picture of a ballerina.
    It uses the get_pixels and store_pixels methods to change the
    transparency levels of individual pixels.

    The example takes a few seconds to run. Be patient.

END_INFO

MIN_TRANSPARENCY = Integer(TransparentOpacity * 0.15)
FILL_COLOR = Pixel.new(MIN_TRANSPARENCY, MIN_TRANSPARENCY, MIN_TRANSPARENCY, OpaqueOpacity).to_color

ballerina = Image.read("../doc/ex/images/Ballerina.jpg")[0]

# Note: this technique won't work with every image. To make a pretty
# vignette you need an image with a uniform, fairly dark background.

# Create the vignette by drawing a gray oval on a white background.
# The gray center color is chosen specifically to provide a full
# range of transparency from (nearly) opaque to 100% transparency.
# The result is a very smooth transition from image to white border.
# See below for how the transparency is computed.

# The size of the oval is arbitrary - in this case it's 90% of the
# size of the image.

oval = Image.new(ballerina.columns, ballerina.rows) {self.background_color = 'white'}
gc = Draw.new
gc.stroke(FILL_COLOR)
gc.fill(FILL_COLOR)
gc.ellipse(ballerina.columns/2, ballerina.rows/2, ballerina.columns/2-(ballerina.columns*0.10), ballerina.rows/2-(ballerina.rows*0.10), 0, 360)
gc.draw(oval)

# Add a lot of blurring to the oval. I use blur_image because it's
# much faster than the gaussian_blur method and produces no observable
# difference. The exact amount of blurring is a judgment call. The
# higher the 2nd argument, the more blurring, although increasing the
# value above 25 doesn't add significant additional blurriness.

oval = oval.blur_image(0, 25)

# Use the "grayness" of the pixels to compute their transparency. The
# grayer the pixels, the more transparent. The center pixels are entirely
# transparent. The outside pixels (entirely white) are 15% transparent.

oval.rows.times do |y|
    pixels = oval.get_pixels(0, y, oval.columns, 1)
    pixels.each do |p|

        # For gray pixels, R=G=B, so we can simply use the
        # R value to determine the "grayness" of the pixel.

        p.opacity = (TransparentOpacity - p.red) + MIN_TRANSPARENCY
    end
    oval.store_pixels(0, y, oval.columns, 1, pixels)
end

# Composite the oval over the image to produce the final oval.

vignette = ballerina.composite(oval, CenterGravity, OverCompositeOp)
vignette.display
exit



