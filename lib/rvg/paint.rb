#--
# $Id: paint.rb,v 1.1 2005/03/12 17:02:00 rmagick Exp $
# Copyright (C) 2005 Timothy P. Hunter
#++
# Defines paint server classes.
# Eventually this will include gradients.

class Magick::RVG

    class Pattern
        include StructureConstructors
        include UseConstructors
        include ShapeConstructors
        include TextConstructors
        include ImageConstructors
        include Stretchable
        include Duplicatable
        include Stylable

        # Create a pattern that can be used with the :fill or :stroke styles.
        # The +width+ and +height+ arguments define the viewport.
        # The pattern will be repeated at <tt>x+m*width</tt> and <tt>y+n*height</tt>
        # offsets.
        #
        # Define the pattern in the block.
        # The pattern can be composed of shapes (rectangle, circles, etc.), text,
        # raster images and container objects. You can include graphic objects by
        # referring to them with #use.
        def initialize(width=0, height=0, x=0, y=0)
            super()
            @width, @height, @x, @y = RVG.convert_to_float(width, height, x, y)
            @content = Content.new
            yield(self) if block_given?
        end

        def add_primitives(gc, style)       #:nodoc:
            name = __id__.to_s
            gc.pattern(name, @x, @y, @width, @height) do
                add_viewbox_primitives(@width, @height, gc)
                @content.each { |element| element.add_primitives(gc) }
            end
            gc.__send__(style, name)
        end

    end     # class Pattern

end     # module Magick::RVG

