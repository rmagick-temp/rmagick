#--############################################################################
# $Id: rvg.rb,v 1.10 2009/02/28 23:52:28 rmagick Exp $
#
#                    Copyright (C) 2009 by Timothy P. Hunter
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#
# This software is OSI Certified Open Source Software.
# OSI Certified is a certification mark of the Open Source Initiative.
#
#++############################################################################

require 'RMagick'
require 'rvg/misc'
require 'rvg/describable'
require 'rvg/stylable'
require 'rvg/transformable'
require 'rvg/stretchable'
require 'rvg/text'
require 'rvg/embellishable'
require 'rvg/container'
require 'rvg/pathdata'
require 'rvg/clippath'
require 'rvg/paint'
require 'rvg/units'

require 'pp' if ENV['debug_rvg']

# RVG is the main class in this library. All graphic elements
# must be contained within an RVG object.
module Magick
    class RVG

        include Stylable
        include Transformable
        include Stretchable
        include Embellishable
        include Describable
        include Duplicatable

      private

        # background_fill defaults to 'none'. If background_fill has been set to something
        # else, combine it with the background_fill_opacity.
        def bgfill()
            if @background_fill.nil?
                color = Magick::Pixel.new(0,0,0,Magick::TransparentOpacity)
            else
                color = @background_fill
                color.opacity = (1.0 - @background_fill_opacity) * Magick::TransparentOpacity
            end
            return color
        end

        def new_canvas
            if @background_pattern
                canvas = Magick::Image.new(@width, @height, @background_pattern)
            elsif @background_image
                if @width != @background_image.columns || @height != @background_image.rows
                    canvas = case @background_position
                        when :scaled
                            @background_image.resize(@width, @height)
                        when :tiled
                            Magick::Image.new(@width, @height, Magick::TextureFill.new(@background_image))
                        when :fit
                            width, height = @width, @height
                            bgcolor = bgfill()
                            @background_image.change_geometry(Magick::Geometry.new(width, height)) do |new_cols, new_rows|
                                bg_image = @background_image.resize(new_cols, new_rows)
                                if bg_image.columns != width || bg_image.rows != height
                                    bg = Magick::Image.new(width, height) { self.background_color = bgcolor }
                                    bg_image = bg.composite!(bg_image, Magick::CenterGravity, Magick::OverCompositeOp)
                                end
                                bg_image
                            end
                    end
                else
                    canvas = @background_image.copy
                end
            else
                bgcolor = bgfill()
                canvas = Magick::Image.new(Integer(@width), Integer(@height)) { self.background_color = bgcolor }
            end
            canvas[:desc] = @desc if @desc
            canvas[:title] = @title if @title
            canvas[:metadata] = @metadata if @metadata
            return canvas
        end

        if ENV['debug_prim']
            def print_gc(gc)
                primitives = gc.inspect.split(/\n/)
                indent = 0
                primitives.each do |cmd|
                    indent -= 1 if cmd['pop ']
                    print(('   '*indent), cmd, "\n")
                    indent += 1 if cmd['push ']
                end
            end
        end

      public

        WORD_SEP = / /     # Regexp to separate words

        # The background image specified by background_image=
        attr_reader :background_image
        # The background image layout specified by background_position=
        attr_reader :background_position
        # The background fill color specified by background_fill=
        attr_reader :background_fill
        # The background fill color opacity specified by background_fill_opacity=
        attr_reader :background_fill_opacity
        # The image after drawing has completed
        attr_reader :canvas
        # For embedded RVG objects, the x-axis coordinate of the upper-left corner
        attr_reader :x
        # For embedded RVG objects, the x-axis coordinate of the upper-left corner
        attr_reader :y
        attr_reader :width, :height

        # Sets an image to use as the canvas background. See background_position= for layout options.
        def background_image=(bg_image)
            warn "background_image= has no effect in nested RVG objects" if @nested
            if bg_image && ! bg_image.kind_of?(Magick::Image)
                raise ArgumentError, "background image must be an Image (got #{bg_image.class})"
            end
            @background_image = bg_image
        end

        # Sets an object to use to fill the canvas background.
        # The object must have a <tt>fill</tt> method. See the <b>Fill Classes</b>
        # section in the RMagick doc for more information.
        def background_pattern=(filler)
            warn "background_pattern= has no effect in nested RVG objects" if @nested
            @background_pattern = filler
        end

        # How to position the background image on the canvas. One of the following symbols:
        # [:scaled] Scale the image to the canvas width and height.
        # [:tiled]  Tile the image across the canvas.
        # [:fit] Scale the image to fit within the canvas while retaining the
        #        image proportions. Center the image on the canvas. Color any part of
        #        the canvas not covered by the image with the background color.
        def background_position=(pos)
            warn "background_position= has no effect in nested RVG objects" if @nested
            bg_pos = pos.to_s.downcase
            if ! ['scaled', 'tiled', 'fit'].include?(bg_pos)
                raise ArgumentError, "background position must be `scaled', `tiled', or `fit' (#{pos} given)"
            end
            @background_position = bg_pos.to_sym
        end

        # Sets the canvas background color. Either a Magick::Pixel or a color name.
        # The default fill is "none", that is, transparent black.
        def background_fill=(color)
            warn "background_fill= has no effect in nested RVG objects" if @nested
            if ! color.kind_of?(Magick::Pixel)
                begin
                    @background_fill = Magick::Pixel.from_color(color)
                rescue Magick::ImageMagickError
                    raise ArgumentError, "unknown color `#{color}'"
                rescue TypeError
                    raise TypeError, "cannot convert #{color.class} into Pixel"
                rescue
                    raise ArgumentError, "argument must be a color name or a Pixel (got #{color.class})"
                end
            else
                @background_fill = color
            end
        end

        # Opacity of the background fill color, a number between 0.0 (transparent) and
        # 1.0 (opaque). The default is 1.0 when the background_fill= attribute has been set.
        def background_fill_opacity=(opacity)
            warn "background_fill_opacity= has no effect in nested RVG objects" if @nested
            begin
                @background_fill_opacity = Float(opacity)
            rescue ArgumentError
                raise ArgumentError, "background_fill_opacity must be a number between 0 and 1 (#{opacity} given)"
            end
        end

        # Draw a +width+ x +height+ image. The image is specified by calling
        # one or more drawing methods on the RVG object.
        # You can group the drawing method calls in the optional associated block.
        # The +x+ and +y+ arguments have no meaning for the outermost RVG object.
        # On nested RVG objects [+x+, +y+] is the coordinate of the upper-left
        # corner in the containing canvas on which the nested RVG object is placed.
        #
        # Drawing occurs on a +canvas+ created by the #draw method. By default the
        # canvas is transparent. You can specify a different canvas with the
        # #background_fill= or #background_image= methods.
        #
        # RVG objects are _containers_. That is, styles and transforms defined
        # on the object are used by contained objects such as shapes, text, and
        # groups unless overridden by an inner container or the object itself.
        def initialize(width=nil, height=nil)
            super
            @width, @height = width, height
            @content = Content.new
            @canvas = nil
            @background_fill = nil
            @background_fill_opacity = 1.0  # applies only if background_fill= is used
            @background_position = :scaled
            @background_pattern, @background_image, @desc, @title, @metadata = nil
            @x, @y = 0.0, 0.0
            @nested = false
            yield(self) if block_given?
        end

        # Construct a canvas or reuse an existing canvas.
        # Execute drawing commands. Return the canvas.
        def draw
            raise StandardError, "draw not permitted in nested RVG objects" if @nested
            @canvas ||= new_canvas    # allow drawing over existing canvas
            gc = Utility::GraphicContext.new
            add_outermost_primitives(gc)
            pp(self) if ENV['debug_rvg']
            print_gc(gc) if ENV['debug_prim']
            gc.draw(@canvas)
            return @canvas
        end

        # Accept #use arguments. Use (x,y) to generate an additional translate.
        # Override @width and @height if new values are supplied.
        def ref(x, y, rw, rh)   #:nodoc:
            translate(x, y) if (x != 0 || y != 0)
            @width = rw if rw
            @height = rh if rh
        end

        # Used by Magick::Embellishable.rvg to set non-0 x- and y-coordinates
        def corner(x, y)        #:nodoc:
            @nested = true
            @x, @y = Float(x), Float(y)
            translate(@x, @y) if (@x != 0.0 || @y != 0.0)
        end

        # Primitives for the outermost RVG object
        def add_outermost_primitives(gc)    #:nodoc:
            add_transform_primitives(gc)
            gc.push
            add_viewbox_primitives(@width, @height, gc)
            add_style_primitives(gc)
            @content.each { |element| element.add_primitives(gc) }
            gc.pop
            self
        end

        # Primitives for nested RVG objects
        def add_primitives(gc)  #:nodoc:
            if @width.nil? || @height.nil?
                raise ArgumentError, "RVG width or height undefined"
            elsif @width == 0 || @height == 0
                return self
            end
            gc.push
            add_outermost_primitives(gc)
            gc.pop
        end

    end # end class RVG
end # end module Magick

