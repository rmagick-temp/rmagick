#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.new(40, 40) {self.background_color = 'lightcyan2'}

# The view is 400 pixels square, starting
# column 10, row 5 from the top of the image.
img.view( 10, 5, 20, 20) do |view|

    # Set all the pixels in the view to green.
    view[][] = Magick::Pixel.new(0, Magick::QuantumRange)

    # Change the top and bottom rows to red.
    view[0][] = 'red'
    view[-1,1][] = 'red'

    # Set 6 pixels to black.
    view[[13,15]][[12,14,16]] = 'black'

    # Set 1 pixel to yellow.
    view[5][7] = 'yellow'

    # Change the green channel of all the
    # pixels on row 8.
    view[8][].green = Magick::QuantumRange/2

    # Change the blue channel of 8 pixels
    # on column 10.
    view[4,8][10].blue = Magick::QuantumRange
end

img.scale(5).write("viewex.gif")
exit

