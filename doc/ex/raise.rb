#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#raise method.
# Make a raised "button" with the b&w cartoon
# image of a snake on a granite background.

# Read the snake image, convert it to b&w with quantize,
# then use matte_replace to make its background transparent.

begin
    snake = Magick::Image.read('images/Snake.wmf').first

    # This call to zoom demonstrates that the zoom doesn't have
    # to be proportional. The snake looks pretty good at this size.
    snake.resize!(snake.columns/2, snake.rows/4)
    snake = snake.quantize(256, Magick::GRAYColorspace)
    snake = snake.matte_replace(0,0)

    # Now make the button background using ImageListMagick's
    # builtin "granite" image format.  Make the background
    # a bit larger than the snake so the snake's edges
    # don't get folded into the raised edges of the background.
    granite = Magick::Image.read('granite:').first
    granite_fill = Magick::TextureFill.new(granite)
    bg = Magick::Image.new(snake.columns+20, snake.rows+20, granite_fill)

    # Here's the call to raise. Take all the defaults: the raised edges
    # will be 6 pixels wide and the result will look raised instead of lowered.
    bg = bg.raise

    # Composite the snake over the background. Since the snake's background
    # is transparent, the granite texture will show through nicely.
    result = bg.composite(snake, Magick::CenterGravity, Magick::OverCompositeOp)
    #result.display
    result.write('raise.gif')

rescue Magick::ImageMagickError
	puts "#{$0}: ImageMagickError - #{$!}"
end
exit
