# This routine needs the color_histogram method
# from either ImageMagick 6.0.0 or GraphicsMagick 1.1
# Specify an image filename as an argument.

require 'RMagick'

module Magick
    class Image

      private
        HISTOGRAM_COLS = 256
        HISTOGRAM_ROWS = 200
        AIR_FACTOR = 1.025

        # Given a channel name and the frequency data for the
        # channel, draw a histogram for the specified channel.
        def channel_histogram(color, label, freqs, scale, fg, bg)

            # Make a blank image to draw on. Calls AllocateImage.
            histogram = Image.new(HISTOGRAM_COLS, HISTOGRAM_ROWS) {
                self.background_color = bg
                self.border_color = fg
                }

            gc = Draw.new
            gc.stroke(color)
            gc.stroke_width(1)
            gc.affine(1, 0, 0, -scale, 0, HISTOGRAM_ROWS)

            HISTOGRAM_COLS.times do |x|
                gc.line(x, 0, x, freqs[x])
            end

            gc.draw(histogram)
            histogram['Label'] = label

            return histogram
        end

        # The alpha frequencies are shown as white dots.
        def alpha_histogram(freqs, scale, fg, bg)
            histogram = Image.new(HISTOGRAM_COLS, HISTOGRAM_ROWS) {
                self.background_color = bg
                self.border_color = fg
            }

            gc = Draw.new
            gc.affine(1, 0, 0, -scale, 0, HISTOGRAM_ROWS)
            gc.fill('white')

            HISTOGRAM_COLS.times do |x|
                gc.point(x, freqs[x])
            end

            gc.draw(histogram)
            histogram['Label'] = 'Alpha'

            return histogram
        end

        # The RGB histogram must be drawn a pixel at a time. Of course
        # we could've drawn all the histograms this way, but where's
        # the fun in that?
        def rgb_histogram(red, green, blue, scale, fg, bg)
            histogram = Image.new(HISTOGRAM_COLS, HISTOGRAM_ROWS) {
                self.background_color = bg
                self.border_color = fg
            }

            HISTOGRAM_COLS.times do |x|
                column = histogram.get_pixels(x, 0, 1, HISTOGRAM_ROWS-1)
                column.each_with_index do |p, n|
                    p.red = MaxRGB if n >= HISTOGRAM_ROWS - (red[x] * scale)
                    p.green = MaxRGB if n >= HISTOGRAM_ROWS - (green[x] * scale)
                    p.blue = MaxRGB if n >= HISTOGRAM_ROWS - (blue[x] * scale)
                end
                histogram.store_pixels(x, 0, 1, HISTOGRAM_ROWS-1, column)
            end

            histogram['Label'] = 'RGB'
            return histogram
        end

        # Use AnnotateImage to write the stats.
        def info_text(fg, bg)
            klass = class_type == DirectClass ? "DirectClass" : "PsuedoClass"

            text = <<-END_TEXT
Format: #{format}
Geometry: #{columns}x#{rows}
Class: #{klass}
Depth: #{depth} bits-per-pixel component
Colors: #{number_colors}
            END_TEXT

            info = Image.new(HISTOGRAM_COLS, HISTOGRAM_ROWS) {
                self.background_color = bg
                self.border_color = fg
                }

            gc = Draw.new

            gc.annotate(info, 0, 0, 0, 0, text) {
                self.stroke = 'transparent'
                self.fill = fg
                self.gravity = CenterGravity
                }
            info['Label'] = 'Info'

            return info
        end

        # The intensity histogram is drawn as a gradient. Draw the histogram using
        # white lines, then call TransparentImage to make the white lines transparent,
        # then composite the histogram over a gradient background.
        def intensity_histogram(freqs, scale, fg, bg)

            histogram = channel_histogram('white', 'Intensity', freqs, scale, fg, bg)
            histogram = histogram.transparent('white')

            gradient = (Image.read("gradient:#ffff80-#ff9000") { self.size="#{HISTOGRAM_COLS}x#{HISTOGRAM_ROWS}" }).first
            histogram = gradient.composite(histogram, CenterGravity, OverCompositeOp)

            histogram['Label'] = 'Intensity'

            return histogram
        end

        # Returns a value between 0 and 255. Same as the PixelIntensity macro.
        def pixel_intensity(pixel)
            (306*pixel.red + 601*pixel.green + 117*pixel.blue)/1024
        end

        # Make the color histogram. Quantize the image to 256 colors if necessary.
        def color_freq(fg, bg)
            img = number_colors > 256 ? quantize(256) : self

            histogram = Image.new(HISTOGRAM_COLS, HISTOGRAM_ROWS) {
                self.background_color = bg
                self.border_color = fg
                }

            begin
                hist = img.color_histogram
                pixels = hist.keys.sort_by {|pixel| hist[pixel] }

                scale = HISTOGRAM_ROWS / (hist.values.max*AIR_FACTOR)

                gc = Draw.new
                gc.affine(1, 0, 0, -scale, 0, HISTOGRAM_ROWS)
                width = 256.0/img.number_colors
                gc.stroke_width(width.to_i)

                start = 256 - img.number_colors
                pixels.each { |pixel|
                    gc.stroke(pixel.to_color)
                    gc.line(start, 0, start, hist[pixel])
                    start += width
                }

                gc.draw(histogram)
            rescue NotImplementedError
                $stderr.puts "The color_histogram method is not supported by this version "+
                             "of ImageMagick/GraphicsMagick"
            end

            histogram['Label'] = 'Color Frequency'
            return histogram
        end

      public
        # Create the histogram montage.
        def histogram(fg='white', bg='black')

            red   = Array.new(HISTOGRAM_COLS, 0)
            green = Array.new(HISTOGRAM_COLS, 0)
            blue  = Array.new(HISTOGRAM_COLS, 0)
            alpha = Array.new(HISTOGRAM_COLS, 0)
            int   = Array.new(HISTOGRAM_COLS, 0)

            rows.times do |row|
                pixels = get_pixels(0, row, columns, 1)
                pixels.each do |pixel|
                    red[pixel.red] = red[pixel.red].succ
                    green[pixel.green] = green[pixel.green].succ
                    blue[pixel.blue] = blue[pixel.blue].succ

                    # Only count opacity channel if some pixels are not opaque.
                    if !opaque?
                        alpha[pixel.opacity] = alpha[pixel.opacity].succ
                    end
                    v = pixel_intensity(pixel)
                    int[v]   = int[v].succ
                end
            end

            # Scale to chart size. When computing the scale, add some "air" between
            # the max frequency and the top of the histogram. This makes a prettier chart.
            # The RGBA and intensity histograms are all drawn to the same scale.
            max = [red.max, green.max, blue.max, alpha.max, int.max].max
            scale = HISTOGRAM_ROWS / (max*AIR_FACTOR)

            charts = ImageList.new

            # Add the thumbnail.
            thumb = copy
            thumb['Label'] = File.basename(filename)
            charts << thumb

            # Add the channel histograms
            charts << channel_histogram('red', 'Red', red, scale, fg, bg)
            charts << channel_histogram('rgb(0,255,0)', 'Green', green, scale, fg, bg)
            charts << channel_histogram('blue', 'Blue', blue, scale, fg, bg)

            # Add Alpha channel or image stats
            if !opaque?
                charts << alpha_histogram(alpha, scale, fg, bg)
            else
                charts << info_text(fg, bg)
            end

            charts << rgb_histogram(red, green, blue, scale, fg, bg)

            # Add the intensity histogram.
            charts << intensity_histogram(int, scale, fg, bg)

            # Add the color frequency histogram.
            charts << color_freq(fg, bg)

            # Make a montage.
            histogram = charts.montage {
                self.background_color = bg
                self.stroke = 'transparent'
                self.fill = fg
                self.border_width = 1
                self.tile         = "4x2"
                self.geometry     = "#{HISTOGRAM_COLS}x#{HISTOGRAM_ROWS}+10+10"
            }

            return histogram
        end
    end
end

puts <<END_INFO

This example shows how to get pixel-level access to an image.
Usage: histogram.rb <image-filename>

END_INFO

# Get filename from command line.
if !ARGV[0] then
    puts "No filename argument. Defaulting to Flower_Hat.jpg"
    filename = '../doc/ex/images/Flower_Hat.jpg'
else
    filename = ARGV[0]
end

# Only process first frame if multi-frame image
image = Magick::Image.read(filename)
if image.length > 1
    puts "Charting 1st image"
end
image = image.first

# Give the user something to look at while we're working.
name = File.basename(filename).sub(/\..*?$/,'')
$defout.sync = true
printf "Creating #{name}_Histogram.miff"

timer = Thread.new do
    loop do
        sleep(1)
        printf "."
    end
end

# Generate the histograms
histogram = image.histogram

# Write output file
histogram.compression = Magick::ZipCompression
histogram.write("./#{name}_Histogram.miff")

Thread.kill(timer)
puts "Done!"
exit

