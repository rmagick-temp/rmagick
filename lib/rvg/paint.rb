#--
# $Id: paint.rb,v 1.5 2008/02/24 18:26:36 rmagick Exp $
# Copyright (C) 2008 Timothy P. Hunter
#++
# Defines paint server classes.
# Eventually this will include gradients.

module Magick
    class RVG

        class Pattern
            include StructureConstructors
            include UseConstructors
            include ShapeConstructors
            include TextConstructors
            include ImageConstructors
            include Stretchable
            include Duplicatable
            include Stylable

            # Return upper-left corner, width, height of viewport in user coordinates.
            # Usually these are the values specified when the Pattern object is
            # created, but they can be changed by a call to the viewbox method.
            attr_reader :x, :y, :width, :height

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
                @width, @height, @x, @y = Magick::RVG.convert_to_float(width, height, x, y)
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

    end # class RVG
end # module Magick

