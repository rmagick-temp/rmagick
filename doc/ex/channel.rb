#! /usr/local/bin/ruby -w

require 'RMagick'

# Extract the red, green, and blue channels
# from a picture of a vase of roses. Display
# the original image and the 3 channels as
# a 2x2 grid.

# We'll need a legend for each
# channel so create a Draw object now
legend = Magick::Draw.new
legend.stroke = 'transparent'
legend.gravity = Magick::SouthGravity
legend.fill = 'white'

# Read the image and change it to a manageable size.
roses = Magick::Image.read('images/roses.jpg').first
roses.resize!(200.0/roses.rows)

channels = Magick::ImageList.new

# Append each channel to the imagelist.
channels << roses.channel(Magick::RedChannel)
legend.annotate(channels, 0, 0, 10, 20, 'Red Channel')
channels << roses.channel(Magick::GreenChannel)
legend.annotate(channels, 0, 0, 10, 20, 'Green Channel')
channels << roses.channel(Magick::BlueChannel)
legend.annotate(channels, 0, 0, 10, 20, 'Blue Channel')

# Finally, insert the original at the beginning of the list.
channels.unshift(roses)
legend.annotate(channels, 0, 0, 10, 20, 'Original')

# ImageList#montage makes a 2x2 grid easy.
combined = channels.montage {
    self.geometry = "#{roses.columns}x#{roses.rows}+5+5"
    self.tile = "2x2"
    self.background_color = 'white'
    }

#combined.display
combined.write('channel.jpg')
exit
