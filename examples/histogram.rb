
# Usage:		ruby histogram.rb [image filename]
# Purpose:	draw an RGB histogram
# Notes:		demonstrates Image#get_pixels method

require 'RMagick'

module Magick
    class Image
        HISTOGRAM_COLS = 256
        HISTOGRAM_ROWS = 200

        # Given a channel name and the frequency data for the
        # channel, draw a histogram for the specified channel
        def channel_histogram(color, freqs, scale, fg, bg)
            histogram = Image.new(HISTOGRAM_COLS, HISTOGRAM_ROWS) {
                self.background_color = bg
                self.border_color = fg
                }
            gc = Draw.new
            gc.stroke(color)
            gc.stroke_width(1)

            HISTOGRAM_COLS.times do |x|
                gc.line(x, HISTOGRAM_ROWS, x, HISTOGRAM_ROWS-(freqs[x]*scale))
            end
            gc.draw(histogram)
            return histogram
        end
        private :channel_histogram
          
        # Draw histograms for each image channel plus a combined
        # RGB histogram. Return the result as a single image.
        def histogram(bg='black',fg='white')

            # Count the level frequencies
            red   = Array.new(HISTOGRAM_COLS, 0)
            green = Array.new(HISTOGRAM_COLS, 0)
            blue  = Array.new(HISTOGRAM_COLS, 0)

            rows.times do |row|
                pixels = get_pixels(0, row, columns, 1)
                pixels.each do |pixel|
                    v = pixel.red   & 0xff
                    red[v]   = red[v].succ
                    v = pixel.green & 0xff
                    green[v] = green[v].succ
                    v = pixel.blue  & 0xff
                    blue[v]  = blue[v].succ
                end
            end

            # Compute the maximum frequency. Scale to chart size.
            max = 0
            HISTOGRAM_COLS.times do |x|
                max = [red[x], green[x], blue[x], max].max
            end

            # When computing the scale, add 5% "air" between
            # the max frequency and the top of the histogram.
            # This makes a prettier chart.
            scale = HISTOGRAM_ROWS / (max*1.05)

            # Draw the R, G, B and combined histograms.
            charts = ImageList.new
            charts << channel_histogram('red', red, scale, fg, bg)
            charts << channel_histogram('rgb(0,255,0)', green, scale, fg, bg)
            charts << channel_histogram('blue', blue, scale, fg, bg)
            rgb = charts[0].composite(charts[1], CenterGravity, PlusCompositeOp)
            rgb = rgb.composite(charts[2], CenterGravity, PlusCompositeOp)
            charts.unshift(rgb)

            # Make a montage.
            histogram = charts.montage {
                self.background_color = bg
                self.border_width = 1
                self.tile         = "2x2"
                self.geometry     = "+10+15"
                 self.stroke       = 'transparent'
            }

            # Add the filename in the middle.
            filename = Draw.new
            filename.annotate(histogram, 0, 0, 0, 0, self.filename) {
                self.stroke  = 'transparent'
                self.fill    = fg
                self.gravity = CenterGravity
            }
            return histogram
        end
    end
end

if !ARGV[0] || ARGV[0] == '-?' then
    puts "Usage: histogram.rb <image-file>"
    exit
end

image = Magick::Image.read(ARGV[0])
if image.length > 1
    puts "Charting 1st image"
end
image = image.first

name = File.basename(ARGV[0])
name.sub!(/\..*$/,'')
$defout.sync = true
printf "Creating #{name}_Histogram.gif..."

timer = Thread.new do
    loop do
        sleep(1)
        printf "."
    end
end

histogram = image.histogram

histogram.write("./#{name}_Histogram.gif")
Thread.kill(timer)
puts "Done!"
exit
