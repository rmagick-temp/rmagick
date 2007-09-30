#!/usr/local/bin/ruby -w
#
# Simple demo program for RMagick
#
# Concept and algorithms lifted from Magick++ demo script written
# by Bob Friesenhahn.
#
require 'RMagick'
include Magick

#
#   RMagick version of Magick++/demo/demo.cpp
#

Font = "Helvetica"

begin
    puts "Read images..."

    model = ImageList.new("../doc/ex/images/model.miff")
    model.border_color = "black"
    model.background_color = "black"
    model.cur_image[:Label] = "RMagick"

    smile = ImageList.new("../doc/ex/images/smile.miff")
    smile.border_color = "black"
    smile.cur_image[:Label] = "Smile"

    #
    #   Create image stack
    #
    puts "Creating thumbnails"

    # Construct an initial list containing five copies of a null
    # image. This will give us room to fit the logo at the top.
    # Notice I specify the width and height of the images via the
    # optional "size" attribute in the parm block associated with
    # the read method. There are two more examples of this, below.
    example = ImageList.new
    5.times { example.read("NULL:black") { self.size = "70x70"} }

    puts "   add noise..."
    example << model.add_noise(LaplacianNoise)
    example.cur_image[:Label] = "Add Noise"

    puts "   annotate..."
    example << model.cur_image.copy
    example.cur_image[:Label] = "Annotate"
    draw = Draw.new
    draw.annotate(example, 0, 0, 0, 20, "RMagick") {
        self.pointsize = 18;
        self.font = Font;
        self.stroke = "gold"
        self.fill = "gold"
        self.gravity = NorthGravity
        }

    puts "   blur..."
    example << model.blur_image(0.0, 1.5)
    example.cur_image[:Label] = "Blur"

    puts "   border..."
    example << model.border(6, 6, "gold")
    example.cur_image[:Label] = "Border"

    puts "   channel..."
    example << model.channel(RedChannel)
    example.cur_image[:Label] = "Channel"

    puts "   charcoal..."
    example << model.charcoal
    example.cur_image[:Label] = "Charcoal"

    puts "   composite..."
    example << model.composite(smile, 35, 65, OverCompositeOp)
    example.cur_image[:Label] = "Composite"

    puts "   contrast..."
    example << model.contrast(false)
    example.cur_image[:Label] = "Contrast"

    puts "   convolve..."
    kernel = [ 1, 1, 1, 1, 4, 1, 1, 1, 1 ]
    example << model.convolve(3, kernel)
    example.cur_image[:Label] = "Convolve"

    puts "   crop..."
    example << model.crop(25, 50, 80, 80)
    example.cur_image[:Label] = "Crop"

    puts "   despeckle..."
    example << model.despeckle
    example.cur_image[:Label] = "Despeckle"

    puts "   draw..."
    example << model.cur_image.copy
    example.cur_image[:Label] = "Draw"
    gc = Draw.new
    gc.fill "black"
    gc.fill_opacity 0
    gc.stroke "gold"
    gc.stroke_width 2
    gc.circle 60,90, 60,120
    gc.draw(example)

    puts "   edge..."
    example << model.edge(0)
    example.cur_image[:Label] = "Detect Edges"

    puts "   emboss..."
    example << model.emboss
    example.cur_image[:Label] = "Emboss"

    puts "   equalize..."
    example << model.equalize
    example.cur_image[:Label] = "Equalize"

    puts "   explode..."
    example << model.implode(-1)
    example.background_color = "#000000ff"
    example.cur_image[:Label] = "Explode"

    puts "   flip..."
    example << model.flip
    example.cur_image[:Label] = "Flip"

    puts "   flop..."
    example << model.flop
    example.cur_image[:Label] = "Flop"

    puts "   frame..."
    example << model.frame
    example.cur_image[:Label] = "Frame"

    puts "   gamma..."
    example << model.gamma_correct(1.6)
    example.cur_image[:Label] = "Gamma"

    puts "   gaussian blur..."
    example << model.gaussian_blur(1, 1.5)
    example.cur_image[:Label] = "Gaussian Blur"

    # To add an Image in one of ImageMagick's built-in formats,
    # call the read method. The filename specifies the format and
    # any parameters it needs. The gradient format can be created in
    # any size. Specify the desired size by assigning it, in the form
    # "WxH", to the optional "size" attribute in the block associated
    # with the read method. Here we create a gradient image that is
    # the same size as the model image.
    puts "   gradient..."
    example.read("gradient:#20a0ff-#ffff00") {
        self.size = Geometry.new(model.columns, model.rows)
        }
    example.cur_image[:Label] = "Gradient"

    puts "   grayscale..."
    example << model.cur_image.quantize(256, GRAYColorspace)
    example.cur_image[:Label] = "Grayscale"

    puts "   implode..."
    example << model.implode(0.5)
    example.cur_image[:Label] = "Implode"

    puts "   median filter..."
    example << model.median_filter(0)
    example.cur_image[:Label] = "Median Filter"

    puts "   modulate..."
    example << model.modulate(1.10, 1.10, 1.10)
    example.cur_image[:Label] = "Modulate"

    puts "   monochrome..."
    example << model.cur_image.quantize(2, GRAYColorspace, false)
    example.cur_image[:Label] = "Monochrome"

    puts "   negate..."
    example << model.negate
    example.cur_image[:Label] = "Negate"

    puts "   normalize..."
    example << model.normalize
    example.cur_image[:Label] = "Normalize"

    puts "   oil paint..."
    example << model.oil_paint(3.0)
    example.cur_image[:Label] = "Oil Paint"

    # The plasma format is very similar to the gradient format, above.
    puts "   plasma..."
    example.read("plasma:fractal") {
        self.size = Geometry.new(model.columns, model.rows)
        }
    example.cur_image[:Label] = "Plasma"

    puts "   quantize..."
    example << model.cur_image.quantize
    example.cur_image[:Label] = "Quantize"

    puts "   raise..."
    example << model.raise
    example.cur_image[:Label] = "Raise"

    puts "   reduce noise..."
    example << model.reduce_noise(3.0)
    example.cur_image[:Label] = "Reduce Noise"

    puts "   resize..."
    example << model.resize(0.50)
    example.cur_image[:Label] = "Resize"

    puts "   roll..."
    example << model.roll(20, 10)
    example.cur_image[:Label] = "Roll"

    puts "   rotate..."
    example << model.rotate(45).transparent("black")
    example.cur_image[:Label] = "Rotate"

    puts "   scale..."
    example << model.scale(0.60)
    example.cur_image[:Label] = "Scale"

    puts "   segment..."
    example << model.segment
    example.cur_image[:Label] = "Segment"

    puts "   shade..."
    example << model.shade(false, 30, 30)
    example.cur_image[:Label] = "Shade"

    puts "   sharpen..."
    example << model.sharpen(0.0, 1.0)
    example.cur_image[:Label] = "Sharpen"

    puts "   shave..."
    example << model.shave(10, 10)
    example.cur_image[:Label] = "Shave"

    puts "   shear..."
    example << model.shear(45, 45).transparent("black")
    example.cur_image[:Label] = "Shear"

    puts "   spread..."
    example << model.spread(3)
    example.cur_image[:Label] = "Spread"

    puts "   solarize..."
    example << model.solarize(50.0)
    example.cur_image[:Label] = "Solarize"

    puts "   swirl..."
    temp = model.copy
    temp.background_color = "#000000ff"
    example << temp.swirl(90)
    example.cur_image[:Label] = "Swirl"

    puts "   unsharp mask..."
    example << model.unsharp_mask(0.0, 1.0, 1.0, 0.05)
    example.cur_image[:Label] = "Unsharp Mask"

    puts "   wave..."
    temp = model.copy
    temp.cur_image[:Label] = "Wave"
    temp.matte = true
    temp.background_color = "#000000ff"
    example << temp.wave(25, 150)

    #
    #   Create image montage - notice the optional
    #   montage parameters are supplied via a block
    #

    puts "Montage images..."

    montage = example.montage {
        self.geometry = "130x194+10+5>"
        self.gravity = CenterGravity
        self.border_width = 1
        rows = (example.size + 4) / 5
        self.tile = Geometry.new(5,rows)
        self.compose = OverCompositeOp

        # Use the ImageMagick built-in "granite" format
        # as the background texture.

#       self.texture = Image.read("granite:").first
        self.background_color = "white"
        self.font = Font;
        self.pointsize = 18;
        self.fill = "#600"
        self.filename = "RMagick Demo"
#       self.shadow = true
#       self.frame = "20x20+4+4"
    }

    # Add the ImageMagick logo to the top of the montage. The "logo:"
    # format is a fixed-size image, so I don't need to specify a size.
    puts "Adding logo image..."
    logo = Image.read("logo:").first
    if /GraphicsMagick/.match Magick_version then
        logo.resize!(200.0/logo.rows)
    else
        logo.crop!(98, 0, 461, 455).resize!(0.45)
    end

    # Create a new Image for the composited montage and logo
    montage_image = ImageList.new
    montage_image << montage.composite(logo, 245, 0, OverCompositeOp)

    # Write the result to a file
    montage_image.compression = RLECompression
    montage_image.matte = false
    puts "Writing image ./rm_demo_out.miff"
    montage_image.write "rm_demo_out.miff"

    # Uncomment the following lines to display image to screen
    # puts "Displaying image..."
    # montage_image.display

rescue
    puts "Caught exception: #{$!}"
end

exit
