#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

puts("Creating colors.miff. This may take a few seconds...")

colors = ImageList.new

# Add a row of "null" colors so we'll have
# room to add the title at the top.
4.times { colors.read('null:') }

puts("\tCreating color swatches...")

# Create a 200x25 image for each named color.
# Label with the name, RGB values, and compliance type.
colors { |c|
    if c.name !~ /grey/ then    # omit SVG 'grays'
        colors.new_image(200, 25) {
            self.background_color = c.color
            self.border_color = 'gray50'
            }
        rgb  = sprintf('#%02x%02x%02x', c.color.red&0xff,  c.color.green&0xff, c.color.blue&0xff)
        rgb += sprintf('%02x', c.color.opacity&0xff) if c.color.opacity != 0
        m = /(.*?)Compliance/.match c.compliance.to_s
        colors.cur_image['Label'] = "#{c.name} (#{rgb}) #{m[1]}"
    end
}

puts("\tCreating montage...")

# Montage. Each image will have 40 tiles.
# There will be 16 images.
montage = colors.montage {
    self.geometry = '200x25+10+5'
    self.gravity = CenterGravity
    self.tile = '4x10'
    self.background_color = 'black'
    self.border_width = 1
    self.fill = 'white'
    self.stroke = 'transparent'
}

# Add the title at the top, over the 'null:'
# tiles we added at the very beginning.
title = Draw.new
title.annotate(montage, 0,0,0,20, 'Named Colors') {
    self.fill = 'white'
    self.stroke = 'transparent'
    self.pointsize = 32
    self.font_weight = BoldWeight
    self.gravity = NorthGravity
}

puts("\tWriting ./colors.miff")
montage.each { |f| f.compression = ZipCompression }
montage.write('colors.miff')

# Make a small sample of the full montage to display in the HTML file.
sample = montage[8].crop(55, 325, 495, 110)
sample.page = Rectangle.new(495,110)
sample.write('colors.gif')

exit
