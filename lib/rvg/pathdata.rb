#--
# $Id: pathdata.rb,v 1.5 2009/02/28 23:52:28 rmagick Exp $
# Copyright (C) 2009 Timothy P. Hunter
#++

module Magick
    class RVG

        # The PathData class provides an object-oriented way to produce an SVG
        # path. Each of the methods corresponds to a path command. Construct a
        # path by calling one or more methods. The path object can be passed
        # as an argument to the RVG::ShapeConstructors#path method.
        class PathData

          private
            def add_points(req, *coords)
                if coords
                    if coords.length % req != 0
                        raise ArgumentError, "wrong number of coordinates specified. A multiple of #{req} required, #{req+coords.length} given."
                    end
                    coords.each {|c| @path << ("%g" % c)}
                end
            end

          public

            # Construct an empty path
            def initialize
                @path = ''
            end

            # Convert the path to its string equivalent.
            def to_s
                @path
            end

            def deep_copy(h=nil)        #:nodoc:
                @path.dup
            end

            # Add a <tt>moveto</tt> command. If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def moveto(abs, x, y, *coords)
                @path << sprintf("%s%g,%g ", (abs ? 'M' : 'm'), x, y)
                # "subsequent pairs are treated as implicit lineto commands"
                add_points(2, *coords)
            end

            # Add a <tt>closepath</tt> command. The <tt>abs</tt> argument
            # is ignored.
            def closepath(abs=true)
                @path << 'Z'    # ignore `abs'
            end

            # Add a <tt>lineto</tt> command. Any number of x,y coordinate
            # pairs may be specified. If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def lineto(abs, x, y, *coords)
                @path << sprintf("%s%g,%g ", (abs ? 'L' : 'l'), x, y)
                # "a number of coordinate pairs may be specified to draw a polyline"
                add_points(2, *coords)
            end

            # Add a <tt>horizontal lineto</tt> command. If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def hlineto(abs, x)
                @path << sprintf("%s%g ", (abs ? 'H' : 'h'), x)
            end

            # Add a <tt>vertical lineto</tt> command. If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def vlineto(abs, y)
                @path << sprintf("%s%g ", (abs ? 'V' : 'v'), y)
            end

            # Add a <tt>curveto</tt> (<em>cubic Bezier</em>) command.
            # If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def curveto(abs, x1, y1, x2, y2, x, y, *coords)
                @path << sprintf("%s%g,%g %g,%g %g,%g ", (abs ? 'C' : 'c'), x1, y1, x2, y2, x, y)
                # "multiple sets of coordinates may be specified to draw a polybezier"
                add_points(6, *coords)
            end

            # Add a <tt>smooth curveto</tt> (<em>cubic Bezier</em>) command.
            # If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def smooth_curveto(abs, x2, y2, x, y, *coords)
                @path << sprintf("%s%g,%g %g,%g ", (abs ? 'S' : 's'), x2, y2, x, y)
                # "multiple sets of coordinates may be specified to draw a polybezier"
                add_points(4, *coords)
            end

            # Add a <tt>quadratic Bezier curveto</tt> command.
            # If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def quadratic_curveto(abs, x1, y1, x, y, *coords)
                @path << sprintf("%s%g,%g %g,%g ", (abs ? 'Q' : 'q'), x1, y1, x, y)
                add_points(4, *coords)
            end

            # Add a <tt>smooth quadratic Bezier curveto</tt> command.
            # If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.
            def smooth_quadratic_curveto(abs, x, y, *coords)
                @path << sprintf("%s%g,%g ", (abs ? 'T' : 't'), x, y)
                add_points(2, *coords)
            end

            # Add an <tt>arc</tt> command.
            # If <tt>abs</tt> is
            # <tt>true</tt> the coordinates are absolute, otherwise
            # the coordinates are relative.

            def arc(abs, rx, ry, x_axis_rotation, large_arc_flag, sweep_flag, x, y)
                @path << sprintf("%s%g,%g %g %d %d %g,%g ", (abs ? 'A' : 'a'), rx, ry, x_axis_rotation, large_arc_flag, sweep_flag, x, y)
            end

        end # class PathData

    end # class RVG
end # module Magick

