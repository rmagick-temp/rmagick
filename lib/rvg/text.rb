#--
# $Id: text.rb,v 1.6 2008/02/24 18:26:37 rmagick Exp $
# Copyright (C) 2008 Timothy P. Hunter
#++
# Text-related classes

module Magick
    class RVG

        # Base class for Tspan, Tref and Text.
        class TextBase
            include Stylable
            include Duplicatable

          private

            def initialize(text, &block)    #:nodoc:
                super()
                @text = text.to_s if text
                @dx = @dy = 0
                @rotation = 0
                @tspans = Content.new
                yield(self) if block_given?
            end

          public

            # Create a new text chunk. Each chunk can have its own initial position and styles.
            # If <tt>x</tt> and <tt>y</tt> are omitted the text starts at the current text
            # position.
            def tspan(text, x=nil, y=nil)
                tspan = Tspan.new(text, x, y)
                tspan.parent = self
                @tspans << tspan
                return tspan
            end

            # Add <tt>x</tt> and <tt>y</tt> to the current text position.
            def d(x, y=0)
                @dx, @dy = Magick::RVG.convert_to_float(x, y)
                yield(self) if block_given?
                self
            end

            # Rotate the text about the current text position.
            def rotate(degrees)
                @rotation = Magick::RVG.convert_to_float(degrees)[0]
                yield(self) if block_given?
                self
            end

            # We do our own transformations.
            def add_primitives(gc)  #:nodoc:
                if @text || @tspans.length > 0
                    gc.push
                    x = self.cx + @dx
                    y = self.cy + @dy
                    if @rotation != 0
                        gc.translate(x, y)
                        gc.rotate(@rotation)
                        gc.translate(-x, -y)
                    end
                    add_style_primitives(gc)
                    if @text
                        x2, y2 = gc.text(x, y, @text)
                        self.cx = x + x2
                        self.cy = y + y2
                    end
                    @tspans.each do |tspan|
                        tspan.add_primitives(gc)
                    end
                    gc.pop
                end
            end

        end     # class TextBase

        # Tspan and Tref shared methods - read/update @cx, @cy in parent Text object.
        module TextLink     #:nodoc:

            def add_primitives(gc)
                @parent.cx = @x if @x
                @parent.cy = @y if @y
                super
            end

            def cx()
                @parent.cx
            end

            def cy()
                @parent.cy
            end

            def cx=(x)
                @parent.cx = x
            end

            def cy=(y)
                @parent.cy = y
            end

        end     # module TextLink


        class Tref < TextBase  #:nodoc:
            include TextLink

            def initialize(obj, x, y, parent)
                @x, @y = Magick::RVG.convert_to_float(x, y, :allow_nil)
                super(nil)
                @tspans << obj
                @parent = parent
            end

        end     # class Tref

        class Tspan < TextBase  #:nodoc:
            include TextLink

            attr_accessor :parent

            # Define a text segment starting at (<tt>x</tt>, <tt>y</tt>).
            # If <tt>x</tt> and <tt>y</tt> are omitted the segment starts
            # at the current text position.
            #
            # Tspan objects can contain Tspan objects.
            def initialize(text=nil, x=nil, y=nil, &block)
                @x, @y = Magick::RVG.convert_to_float(x, y, :allow_nil)
                super(text, &block)
            end

        end     # class Tspan

        class Text < TextBase

            attr_accessor :cx, :cy  #:nodoc:

            # Define a text string starting at [<tt>x</tt>, <tt>y</tt>].
            # Use the RVG::TextConstructors#text method to create Text objects in a container.
            #
            #  container.text(100, 100, "Simple text").styles(:font=>'Arial')
            #
            # Text objects can contain Tspan objects.
            #
            #  container.text(100, 100).styles(:font=>'Arial') do |t|
            #     t.tspan("Red text").styles(:fill=>'red')
            #     t.tspan("Blue text").styles(:fill=>'blue')
            #  end
            def initialize(x=0, y=0, text=nil, &block)
                @cx, @cy = Magick::RVG.convert_to_float(x, y)
                super(text, &block)
            end

            # Reference a Tspan object. <tt>x</tt> and <tt>y</tt> are just
            # like <tt>x</tt> and <tt>y</tt> in RVG::TextBase#tspan
            def tref(obj, x=nil, y=nil)
                if ! obj.kind_of?(Tspan)
                    raise ArgumentError, "wrong argument type #{obj.class} (expected Tspan)"
                end
                obj = obj.deep_copy
                obj.parent = self
                tref = Tref.new(obj, x, y, self)
                @tspans << tref
                return tref
            end

        end     # class Text



        # Methods that construct text objects within a container
        module TextConstructors

            # Draw a text string at (<tt>x</tt>,<tt>y</tt>). The string can
            # be omitted. Optionally, define text chunks within the associated
            # block.
            def text(x=0, y=0, text=nil, &block)
                t = Text.new(x, y, text, &block)
                @content << t
                return t
            end

        end     # module TextConstructors

    end # class RVG
end # module Magick
