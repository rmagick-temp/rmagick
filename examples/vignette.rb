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

ballerina = Image.read("../doc/ex/images/Ballerina3.jpg")[0]
ballerina.matte = true

# Note: this technique won't work with every image. To make a pretty
# vignette you need an image with a uniform, fairly dark background.

# Start by drawing a black oval on a white background. (Although you don't
# have to use an oval at all! Any shape will work. Try a rounded rectangle.)

# The size of the oval is arbitrary - in this case it's 90% of the
# size of the image.

oval = Image.new(ballerina.columns, ballerina.rows) {self.background_color = 'white'}
gc = Draw.new
gc.stroke('black')
gc.fill('black')
gc.ellipse(ballerina.columns/2, ballerina.rows/2,
           ballerina.columns/2-(ballerina.columns*0.10),
           ballerina.rows/2-(ballerina.rows*0.10), 0, 360)
gc.draw(oval)

# Add a lot of blurring to the oval. I use blur_image because it's much faster
# than the gaussian_blur method and produces no observable difference. The
# exact amount of blurring is a judgment call. The higher the 2nd argument, the
# more blurring, although increasing the value above 20 doesn't seem to add any
# additional blurriness.

puts 'Blurring...'
oval = oval.blur_image(0, 20)

# Use the blurred oval to create a transparent border around the ballerina. Use
# the gray level of the pixels in the oval to determine the transparency of the
# pixels in the vignette. All-white pixels outside of the oval cause the image
# borders to become transparent, gray transition pixels cause less transparency.
# In the center of the oval, the pixels are black and so the ballerina pixels
# remain opaque.

puts 'Making border transparent...'

oval.rows.times do |y|
    oval_pixels = oval.get_pixels(0, y, oval.columns, 1)
    ballerina_pixels = ballerina.get_pixels(0, y, ballerina.columns, 1)
    oval_pixels.each_with_index do |p, x|

        # For gray pixels, R=G=B, so we need only inspect one of the
        # RGB values to determine the gray level of the pixel. Use
        # that value to set the opacity of the corresponding pixel
        # in the vignette.

        ballerina_pixels[x].opacity = p.red
    end
    ballerina.store_pixels(0, y, ballerina.columns, 1, ballerina_pixels)
end

# Since the vignette has multiple levels of transparency, we can't
# save it as a GIF or a JPEG. The PNG format can handle it, though.

begin
    puts "Writing `vignette.png'..."
    ballerina.write("vignette.png")
rescue ImageMagickError
    puts "Write failed. No PNG support?"
    # In case PNG support isn't installed, just ignore the exception.
end

# At this point the vignette is complete. However, the `display' method only
# supports 1`level of transparency. Therefore, composite the vignette over a
# standard "checkerboard" background. The resulting image will be 100% opaque.

puts 'Preparing for display...'
checkerboard = Image.read("pattern:checkerboard") {self.size = "#{ballerina.columns}x#{ballerina.rows}"}
vignette = checkerboard[0].composite(ballerina, CenterGravity, OverCompositeOp)
vignette.display
exit

