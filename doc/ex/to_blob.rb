#! /usr/local/bin/ruby -w
require 'RMagick'

before = Magick::Image.read("images/Flower_Hat.jpg").first
before.resize!(0.50)  # make it small so this example will run fast

blob = before.to_blob
# We could pass the format, depth, and geometry as optional
# arguments to `from_blob' but that is not required.
after = Magick::Image.from_blob(blob)

after[0].write("to_blob.gif")
exit

