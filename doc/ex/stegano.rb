#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#stegano method.

# Create a small watermark from the Snake image by
# shrinking it and converting it to B&W.
begin
    watermark = Magick::Image.read('images/Snake.wmf').first
    watermark.scale!(64.0/watermark.rows)
    watermark = watermark.quantize(256, Magick::GRAYColorspace)
    wmrows = watermark.rows
    wmcols = watermark.columns

    # Read the image in which we'll hide the watermark.
    img = Magick::Image.read('images/Flower_Hat.jpg').first
    img.scale!(250.0/img.rows)

    # Embed the watermark starting at offset 91.
    puts 'Embedding watermark...'
    stegano = img.stegano(watermark, 91)

    # Write the watermarked image in MIFF format. Note that
    # the format must be lossless - Image#stegano doesn't work
    # with lossy formats like JPEG.
    stegano.write('img.miff')

    # Read the image and retrieve the watermark. The size
    # attribute describes the size and offset of the watermark.

    # This can take some time. Keep track of how far along we are.
    monitor = Proc.new { |text, quantum, span|
        printf("%s %3.0f%% complete\n", text, ((1.0-(quantum/span.to_f))*100.0))
        }
    Magick.set_monitor(monitor)
    stegano = Magick::Image.read('stegano:img.miff') {
        self.size = Magick::Geometry.new(wmcols, wmrows, 91)
    }
    Magick.set_monitor(nil)

    # We don't need this any more.
    File.delete('img.miff')

    stegano[0].write('stegano.gif')

rescue Magick::ImageMagickError
    puts "#{$0}: ImageMagickError - #{$!}"
end

exit
