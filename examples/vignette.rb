require 'RMagick'
include Magick

puts <<END_INFO

    This example adds a soft-edged vignette to a picture of a ballerina.

END_INFO

ballerina = Image.read("../doc/ex/images/Ballerina.jpg")[0]

# Note: this technique won't work with every image. To make a pretty
# vignette you need an image with a uniform, fairly dark background.

# Create the vignette by drawing a gray oval on a white background.
# The gray center color (#606060) is chosen specifically because
# it's roughly the same color as the image background. This causes
# a very smooth transition from image to white border.

# The size of the oval is arbitrary - in this case it's 90% of the
# size of the image.

oval = Image.new(ballerina.columns, ballerina.rows) {self.background_color = 'white'}
gc = Draw.new
gc.fill = '#606060'
gc.ellipse(ballerina.columns/2, ballerina.rows/2, ballerina.columns/2-(ballerina.columns*0.10), ballerina.rows/2-(ballerina.rows*0.10), 0, 360)
gc.draw(oval)

# Add a lot of blurring to the oval. I use blur_image because it's
# much faster than the gaussian_blur method. The exact amount of
# blurring is a judgement call. The higher the 2nd argument, the
# more blurring.

oval = oval.blur_image(0, 25)

# The entire oval needs to be a bit transparent. Here I chose a
# minium of 25% transparency but it's a judgment call. As the oval
# becomes increasingly gray toward the center, also make it increasingly
# transparent. The gray center portion of the oval is essentially
# entirely transparent. (Actually the center is not quite perfectly
# transparent but the residual opacity is invisible.)

oval.rows.times { |y|
    pixels = oval.get_pixels(0, y, oval.columns, 1)
    pixels.each_with_index { |p,x|
         avg = (p.red + p.green + p.blue) / 3
         opacity = (TransparentOpacity - avg) + (TransparentOpacity * 0.25)
         if opacity > TransparentOpacity
             p.opacity = TransparentOpacity
         else
             p.opacity = opacity
         end
        }
    oval.store_pixels(0, y, oval.columns, 1, pixels)
}

# Composite the oval over the image to produce the final oval.

vignette = ballerina.composite(oval, CenterGravity, OverCompositeOp)
vignette.display
exit



