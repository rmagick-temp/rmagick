require 'RMagick'
include Magick

puts <<END_INFO

    vi-gnette (n.) vin-'yet
    A picture that shades off gradually into the surrounding paper.
                                Merriam-Webster Dictionary

    Vignettes are frequently used in formal portraiture and advertising
    images. This example creates a vignette from a picture of a ballerina.
    It takes a few seconds to run. Be patient.

END_INFO

ballerina = Image.read("../doc/ex/images/Ballerina3.jpg")[0]

# Note: this technique won't work with every image. To make a pretty
# vignette you need an image with a uniform, fairly dark background.

# Start by drawing a white oval on a black background. (Although you don't
# have to use an oval at all! Any shape will work. Try a rounded rectangle.)
# The black pixels correspond to pixels in the image that will become
# transparent. The white pixels correspond to pixels in the image that will
# remain unchanged. Gray pixels, introduced by the blurring below, will
# become more or less transparent depending on how dark or light the pixel is.

# The size of the oval is arbitrary - in this case it's 90% of the
# size of the image.

oval = Image.new(ballerina.columns, ballerina.rows) {self.background_color = 'black'}
gc = Draw.new
gc.stroke('white')
gc.fill('white')
gc.ellipse(ballerina.columns/2, ballerina.rows/2,
           ballerina.columns/2-(ballerina.columns*0.10),
           ballerina.rows/2-(ballerina.rows*0.10), 0, 360)
gc.draw(oval)

# Add a lot of blurring to the oval. I use blur_image because it's much faster
# than the gaussian_blur method and produces no observable difference. The
# exact amount of blurring is a judgment call. The higher the 2nd argument, the
# more blurring, although increasing the value above 20 doesn't seem to add any
# additional blurriness.

oval = oval.blur_image(0, 20)

# The CopyOpacityCompositeOp transforms the opacity level of each image pixel
# according to the intensity of the composite image pixels. In this case, the
# black pixels outside the oval become transparent and the white pixels inside
# the oval remain opaque. Each gray pixel around the border of the oval has a
# varying level of transparency depending on how dark or light it is.

ballerina.matte = true  # Ensure the ballerina image's opacity channel is enabled.
oval.matte = false      # Force the CopyOpacityCompositeOp to use pixel intensity
                        # to determine how much transparency to add to the ballerina
                        # pixels.

ballerina = ballerina.composite(oval, CenterGravity, CopyOpacityCompositeOp)

# Since the vignette has multiple levels of transparency, we can't
# save it as a GIF or a JPEG. The PNG format can handle it, though.

begin
    ballerina.write("vignette.png")
rescue ImageMagickError
    puts "Write failed. No PNG support?"
    # In case PNG support isn't installed, just ignore the exception.
end

# At this point the vignette is complete. However, the `display' method only
# supports 1`level of transparency. Therefore, composite the vignette over a
# standard "checkerboard" background. The resulting image will be 100% opaque.

checkerboard = Image.read("pattern:checkerboard") {self.size = "#{ballerina.columns}x#{ballerina.rows}"}
vignette = checkerboard[0].composite(ballerina, CenterGravity, OverCompositeOp)
vignette.display
exit

