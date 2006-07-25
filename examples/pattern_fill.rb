
# Demonstrate ImageMagick's new (5.5.7-3 and later) built-in patterns.
# Create a Fill class that can be reused to fill in new Image backgrounds.

# Usage: pattern_fill.rb <name-of-pattern>
# Try 'checkerboard' or 'verticalsaw'

require 'RMagick'
include Magick

puts <<END_INFO

This example demonstrates the PATTERN: image format, which is
new in ImageMagick 5.5.7. Specify the name of any of the
supported patterns as an argument. For example, try "checkerboard".

END_INFO

class PatternFill < Magick::TextureFill
    def initialize(name='bricks')
        @pat_img = Magick::Image.read("pattern:#{name}").first
        super(@pat_img)
    end
end

if ARGV[0]
    pattern = ARGV[0]
else
    $stderr.puts "Defaulting to checkerboard pattern."
    pattern = 'checkerboard'
end

# Create a sample image that is 100x bigger than the pattern.
attrs = Image.ping("pattern:#{pattern}").first

tryit = Image.new(10*attrs.columns, 10*attrs.rows, PatternFill.new(pattern))
tryit.write("pattern_fill.gif")
exit
