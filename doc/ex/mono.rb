#!/usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the ImageListImage#quantize method by converting
# a color image into a monochrome image.

# Read the large color cheetah image and scale it to a third
# of its size.
cheetah = Magick::Image.read("images/Cheetah.jpg").first
cheetah.scale!(0.33)

# Quantize the cheetah image into 256 colors in the GRAY colorspace.
mono_cheetah = cheetah.quantize 256, Magick::GRAYColorspace

# Cut the top off the monochrome cheetah image.
mono_bottom = mono_cheetah.crop 0, mono_cheetah.rows/2, mono_cheetah.columns, mono_cheetah.rows/2

# Composite the half-height mono cheetah onto the bottom of
# the original color cheetah.
before_after = cheetah.composite mono_bottom, 0, cheetah.rows/2, Magick::OverCompositeOp

before_after.write "mono.jpg"
exit
