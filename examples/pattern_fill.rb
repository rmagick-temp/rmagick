#! /usr/local/bin/ruby -w

# Demonstrate ImageMagick's new (5.5.7-3 and later) built-in patterns.
# Create a Fill class that can be reused to fill in new Image backgrounds.

# Usage: pattern_fill.rb <name-of-pattern>
# Try 'checkerboard' or 'verticalsaw'

require 'RMagick'
include Magick

class PatternFill < Magick::TextureFill
    def initialize(name='bricks')
        @pat_img = Magick::Image.read("pattern:#{name}").first
        super(@pat_img)
    end
end

raise(ArgumentError, "No pattern name specified") unless ARGV[0]

# Create a sample image that is 100x bigger than the pattern.
attrs = Image.ping("pattern:#{ARGV[0]}").first

tryit = Image.new(10*attrs.columns, 10*attrs.rows, PatternFill.new(ARGV[0]))
tryit.display
exit
