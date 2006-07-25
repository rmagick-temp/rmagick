# Demonstrate the Draw#rotation= method by producing
# an animated MIFF file showing a rotating text string.


require 'RMagick'
include Magick

puts <<END_INFO
Demonstrate the rotation= attribute in the Draw class
by producing an animated image. View the output image
by entering the command: animate rotating_text.miff
END_INFO

text = Draw.new
text.pointsize = 28
text.font_weight = BoldWeight
text.font_style = ItalicStyle
text.gravity = CenterGravity

# Let's make it interesting. Composite the
# rotated text over a gradient fill background.
fill = GradientFill.new(100,100,100,100,"yellow","red")
bg = Image.new(200, 200, fill)

# The "none" color is transparent.
fg = Image.new(bg.columns, bg.rows) { self.background_color = "none" }

# Here's where we'll collect the individual frames.
animation = ImageList.new

0.step(345,15) { |degrees|
    frame = fg.copy
    text.annotate(frame, 0,0,0,0, "Rotating Text") {
        self.rotation = degrees
    }
    # Composite the text over the gradient filled background frame.
    animation << bg.composite(frame, CenterGravity, DisplaceCompositeOp)
}

animation.delay = 8

#animation.animate
puts "...Writing rotating_text.gif"
animation.write("rotating_text.gif")
exit
