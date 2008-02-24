#--
# $Id: stretchable.rb,v 1.6 2008/02/24 21:52:32 rmagick Exp $
# Copyright (C) 2008 Timothy P. Hunter
#++

module Magick
    class RVG

        module PreserveAspectRatio
            #--
            #   Included in Stretchable module and Image class
            #++
            # Specifies how the image within a viewport should be scaled.
            # [+align+] a combination of 'xMin', 'xMid', or 'xMax', followed by
            #           'YMin', 'YMid', or 'YMax'
            # [+meet_or_slice+] one of 'meet' or 'slice'
            def preserve_aspect_ratio(align, meet_or_slice='meet')
                @align = align.to_s
                if @align != 'none'
                    m = /\A(xMin|xMid|xMax)(YMin|YMid|YMax)\z/.match(@align)
                    raise(ArgumentError, "unknown alignment specifier: #{@align}") unless m
                end

                if meet_or_slice
                    meet_or_slice = meet_or_slice.to_s.downcase
                    if meet_or_slice == 'meet' || meet_or_slice == 'slice'
                        @meet_or_slice = meet_or_slice
                    else
                        raise(ArgumentError, "specifier must be `meet' or `slice' (got #{meet_or_slice})")
                    end
                end
                yield(self) if block_given?
                self
            end
        end     # module PreserveAspectRatio


        # The methods in this module describe the user-coordinate space.
        # RVG and Pattern objects are stretchable.
        module Stretchable

          private
            # Scale to fit
            def set_viewbox_none(width, height)
                sx, sy = 1.0, 1.0

                if @vbx_width
                    sx = width / @vbx_width
                end
                if @vbx_height
                    sy = height / @vbx_height
                end

                return [sx, sy]
            end

            # Use align attribute to compute x- and y-offset from viewport's upper-left corner.
            def align_to_viewport(width, height, sx, sy)
                tx = case @align
                         when /\AxMin/
                             0
                         when NilClass, /\AxMid/
                             (width - @vbx_width*sx) / 2.0
                         when /\AxMax/
                             width - @vbx_width*sx
                end

                ty = case @align
                         when /YMin\z/
                             0
                         when NilClass, /YMid\z/
                             (height - @vbx_height*sy) / 2.0
                         when /YMax\z/
                             height - @vbx_height*sy
                end
                return [tx, ty]
            end

            # Scale to smaller viewbox dimension
            def set_viewbox_meet(width, height)
                sx = sy = [width / @vbx_width, height / @vbx_height].min
                return [sx, sy]
            end

            # Scale to larger viewbox dimension
            def set_viewbox_slice(width, height)
                sx = sy = [width / @vbx_width, height / @vbx_height].max
                return [sx, sy]
            end

            # Establish the viewbox as necessary
            def add_viewbox_primitives(width, height, gc)
                @vbx_width  ||= width
                @vbx_height ||= height
                @vbx_x ||= 0.0
                @vbx_y ||= 0.0

                if @align == 'none'
                    sx, sy = set_viewbox_none(width, height)
                    tx, ty = 0, 0
                elsif @meet_or_slice == 'meet'
                    sx, sy = set_viewbox_meet(width, height)
                    tx, ty = align_to_viewport(width, height, sx, sy)
                else
                    sx, sy = set_viewbox_slice(width, height)
                    tx, ty = align_to_viewport(width, height, sx, sy)
                end

                # Establish clipping path around the current viewport
                name = __id__.to_s
                gc.define_clip_path(name) do
                    gc.path("M0,0 l#{width},0 l0,#{height} l-#{width},0 l0,-#{height}z")
                end

                gc.clip_path(name)
                # Add a non-scaled translation if meet or slice
                gc.translate(tx, ty) if (tx.abs > 1.0e-10 || ty.abs > 1.0e-10)
                # Scale viewbox as necessary
                gc.scale(sx, sy) if (sx != 1.0 || sy != 1.0)
                # Add a scaled translation if non-0 origin
                gc.translate(-@vbx_x, -@vbx_y) if (@vbx_x.abs != 0.0 || @vbx_y.abs != 0)
            end

            def initialize(*args, &block)
                super()
                @vbx_x, @vbx_y, @vbx_width, @vbx_height = nil
                @meet_or_slice = 'meet'
                @align = nil
            end

          public
            include PreserveAspectRatio

            # Describe a user coordinate system to be imposed on the viewbox.
            # The arguments must be numbers and the +width+ and +height+
            # arguments must be positive.
            def viewbox(x, y, width, height)
                begin
                    @vbx_x = Float(x)
                    @vbx_y = Float(y)
                    @vbx_width = Float(width)
                    @vbx_height = Float(height)
                rescue ArgumentError
                    raise ArgumentError, "arguments must be convertable to float (got #{x.class}, #{y.class}, #{width.class}, #{height.class})"
                end
                raise(ArgumentError, "viewbox width must be > 0 (#{width} given)") unless width >= 0
                raise(ArgumentError, "viewbox height must be > 0 (#{height} given)") unless height >= 0

                # return the user-coordinate space attributes if defined
                class << self
                  if not defined? @redefined then
                    @redefined = true
                    define_method(:x) { @vbx_x }
                    define_method(:y) { @vbx_y }
                    define_method(:width) { @vbx_width}
                    define_method(:height) { @vbx_height }
                  end
                end

                yield(self) if block_given?
                self
            end

        end     # module Stretchable

    end # class RVG
end # module Magick

