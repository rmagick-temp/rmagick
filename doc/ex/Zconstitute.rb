#! /usr/local/bin/ruby -w
require 'RMagick'

$stdout.sync = true
if RUBY_VERSION < '1.8.0'
    puts 'This example can take several minutes to run...'
else
    puts 'This may take a few seconds...'
end
load 'pixels-array'

image = Magick::Image.constitute(Width, Height, "RGB", Pixels)

image.write("constitute.jpg")
exit
