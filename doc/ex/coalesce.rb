#! /usr/local/bin/ruby -w
require 'RMagick'

buttons = Magick::ImageList.new

# Read 25 alphabet image files, scale to 1/4 size
letter = 'A'
26.times {
    if letter != 'M'            # "M" is not the same size as the other letters
        tiny = Magick::Image.read('images/Button_' + letter + '.gif').first
        tiny.scale! 0.25
        buttons << tiny
    end
    letter.succ!
    }

# Create a image that will hold the alphabet images in 5 rows and 5 columns.
cells = Magick::ImageList.new
cells.new_image buttons.columns*5, buttons.rows*5 do
    self.background_color = "#000000ff"     # transparent
    end
cells.matte = true

offset = Magick::Rectangle.new(0,0,0,0)

# Create 2 arrays from which we can randomly choose row,col pairs
row = [0]*5 + [1]*5 + [2]*5 + [3]*5 + [4]*5
col = (0..4).to_a * 5

# The coalesce method composites the 2nd image over the 1st, the 3rd image
# over the result, and so forth, respecting the page offset of the images.

srand 1234
25.times { |i|
    # Randomly select a row and column for this copy of the "tinya" image.
    # Compute the x,y position of this copy in pixels and store the
    # result in the image's page attribute. Append a copy of the image
    # to the image array in "a".
    n = rand row.length
    x = row.delete_at n
    y = col.delete_at n

    button = buttons[i]
    offset.x = x*button.columns
    offset.y = y*button.rows
    button.page = offset
    button.matte = true
    cells << button
    }

puts "This may take a few seconds..."
cells.delay = 10
cells.iterations = 10000
res = cells.coalesce
res.write "coalesce_anim.gif"
res[25].write "coalesce.gif"
exit



