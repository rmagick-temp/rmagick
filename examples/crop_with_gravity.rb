#! /usr/local/bin/ruby -w

#=======================================================#
# Thanks to Robert Wagner for the idea of allowing a    #
# GravityType instead of the x- and y-offset arguments! #
#=======================================================#

# Demo the use of the GravityType argument to Image#crop.

require 'RMagick'
include Magick

shorts = Image.read('../doc/ex/images/Shorts.jpg').first

regwidth = shorts.columns/2
regheight = shorts.rows/2

mask = Image.new(regwidth, regheight) { self.background_color = 'white'}
mask.opacity = 0.50 * TransparentOpacity

black = Image.new(shorts.columns, shorts.rows) {self.background_color = 'black'}
pairs = ImageList.new

[NorthWestGravity, NorthGravity, NorthEastGravity,
 WestGravity, CenterGravity, EastGravity,
 SouthWestGravity, SouthGravity, SouthEastGravity].each do |gravity|
    pattern = shorts.composite(mask, gravity, OverCompositeOp)
    cropped = shorts.crop(gravity, regwidth, regheight)
    result = black.composite(cropped, gravity, OverCompositeOp)
    result.border_color = "white"
    pairs << pattern
    pairs << result
end

# Montage into a single image
montage = pairs.montage {
    self.geometry = "#{pairs.columns}x#{pairs.rows}+0+0"
    self.tile = "6x3"
    self.border_width = 1
    }
montage.write('crop_with_gravity.miff')
#montage.display




