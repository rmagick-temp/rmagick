#! /usr/local/bin/ruby -w

require 'RMagick'

# Read the snake image file and scale to 200 pixels high.
begin
    snake = Magick::ImageList.new("images/Snake.wmf")
    snake.scale!(200.0/snake.rows)

    # Read the coffee cup image and scale to 200 pixels high.
    coffee = Magick::ImageList.new("images/Coffee.wmf")
    coffee.scale!(200.0/coffee.rows)

    # We want the "no" symbol to be a little smaller.
    # Read and scale to 150 pixels high.
    sign = Magick::ImageList.new("images/No.wmf")
    sign.scale!(150.0/sign.rows)

    # Change the white pixels in the sign to transparent.
    sign = sign.matte_replace(0,0)

    # Create a "nosnake" draw object. Add a composite
    # primitive that composites the "no" symbol over
    # the snake. Draw it.
    nosnake = Magick::Draw.new
    nosnake.composite((snake.columns-sign.columns)/2,
                      (snake.rows-sign.rows)/2, 0, 0, sign)
    nosnake.draw(snake)

    # Repeat for the coffee cup.
    nocoffee = Magick::Draw.new
    nocoffee.composite((coffee.columns-sign.columns)/2,
                       (coffee.columns-sign.columns)/2, 0, 0, sign)
    nocoffee.draw(coffee)

    coffee.write("drawcomp1.gif")
    snake.write("drawcomp2.gif")

rescue Magick::ImageMagickError
    puts "#{$0}: ImageMagickError - #{$!}"
end
exit
