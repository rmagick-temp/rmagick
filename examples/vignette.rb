require 'RMagick'
include Magick

puts <<END_INFO

    vi-gnette (n.) vin-'yet
    A picture that shades off gradually into the surrounding paper.
                                Merriam-Webster Dictionary

    Vignettes are frequently used in formal portraiture and advertising
    images. This example creates a vignette from a picture of a ballerina.
    It uses the get_pixels and store_pixels methods to change the
    transparency levels of individual pixels.

    This example takes a few seconds to run. Be patient.

END_INFO

ballerina = Image.read("../doc/ex/images/Ballerina.jpg")[0]

# Note: this technique won't work with every image. To make a pretty
# vignette you need an image with a uniform, fairly dark background.

# Start by drawing a black oval on a white background.

# The size of the oval is arbitrary - in this case it's 90% of the
# size of the image.

oval = Image.new(ballerina.columns, ballerina.rows) {self.background_color = 'white'}
gc = Draw.new
gc.stroke('black')
gc.fill('black')
gc.ellipse(ballerina.columns/2, ballerina.rows/2, ballerina.columns/2-(ballerina.columns*0.10), ballerina.rows/2-(ballerina.rows*0.10), 0, 360)
gc.draw(oval)

# Add a lot of blurring to the oval. I use blur_image because it's
# much faster than the gaussian_blur method and produces no observable
# difference. The exact amount of blurring is a judgment call. The
# higher the 2nd argument, the more blurring, although increasing the
# value above 20 doesn't add significant additional blurriness.

puts 'Blurring...'
oval = oval.blur_image(0, 20)
oval_copy = oval.copy               # We'll need this later.

# Use the "grayness" of the pixels to compute their transparency. The
# grayer the pixels, the more transparent. The black center pixels are
# entirely transparent. The outside white pixels are entirely opaque.

puts 'Computing opacity...'
oval.rows.times do |y|
    pixels = oval.get_pixels(0, y, oval.columns, 1)
    pixels.each do |p|

        # For gray pixels, R=G=B, so we can simply use the
        # R value to determine the "grayness" of the pixel.

        p.opacity = TransparentOpacity - p.red
    end
    oval.store_pixels(0, y, oval.columns, 1, pixels)
end

# Composite the oval over the image to produce the vignette. We
# could stop here with a perfectly fine vignette with solid white
# borders.

vignette = ballerina.composite(oval, CenterGravity, OverCompositeOp)

# However, let's go one step farther and turn the white borders
# transparent. That way we can overlay the vignette over any background.
# This is where the copy of the oval we made earlier comes in. Use the
# "whiteness" of the pixels in the copy to determine the transparency
# of the pixels in the vignette. All-white pixels become transparent,
# gray pixels less so. The black center pixels are entirely opaque.

vignette.matte = true
puts 'Computing opacity again...'

oval_copy.rows.times do |y|
    copy_pixels = oval_copy.get_pixels(0, y, oval_copy.columns, 1)
    vignette_pixels = vignette.get_pixels(0, y, vignette.columns, 1)
    copy_pixels.each_with_index do |p, x|

        # Again, we need only inspect one of the RGB values to determine
        # the "grayness" of the pixel. Use that value to set the opacity
        # of the corresponding pixel in the vignette.

        vignette_pixels[x].opacity = p.red
    end
    vignette.store_pixels(0, y, vignette.columns, 1, vignette_pixels)
end

# Since the vignette has multiple levels of transparency, we can't
# save it as a GIF or a JPEG. The PNG format can handle it, though.

begin
    puts "Writing ``vignette.png'..."
    vignette.write("vignette.png")
rescue ImageMagickError
    # In case PNG support isn't installed, just ignore the exception.
end

# At this point the vignette is complete. However, the `display'
# method only supports 1`level of transparency. Therefore,
# composite the vignette over a pretty gradient. The resulting
# image will be 100% opaque.

puts 'Preparing for display...'
gradient = Image.new(vignette.columns, vignette.rows,
                        GradientFill.new(0, 0, 0, vignette.rows, '#d8ad7f', '#856a4e'))

vignette = gradient.composite(vignette, CenterGravity, OverCompositeOp)
vignette.display

exit

