#--
# $Id: transformable.rb,v 1.4 2008/02/24 18:26:37 rmagick Exp $
# Copyright (C) 2008 Timothy P. Hunter
#++

module Magick
    class RVG

        # Transforms is an Array with a deep_copy method.
        # During unit-testing it also has a deep_equal method.
        class Transforms < Array        #:nodoc:

            def deep_copy(h=nil)
                copy = self.class.new
                each { |transform| copy << [transform[0], transform[1].dup] }
                return copy
            end

        end     # class Transform

        # Transformations are operations on the coordinate system.
        # All the transformations defined within a container (an RVG object
        # or a group) are applied before drawing any shapes or text.
        # All transformations are applied in the order they were
        # defined. <em>Note:</em> This means that
        #   g.translate(10,20).scale(2)
        # is not the same as
        #   g.scale(2).translate(10,20)
        module Transformable

          private

            # Apply transforms in the same order they were specified!
            def add_transform_primitives(gc)
                @transforms.each { |transform| gc.__send__(transform[0], *transform[1]) }
            end

            def initialize(*args, &block)
                super()
                @transforms = Transforms.new
            end

          public

            # Applies the transformation matrix [sx, rx, ry, sy, tx, ty]
            def matrix(sx, rx, ry, sy, tx, ty)
                begin
                    @transforms << [:affine, [Float(sx), Float(rx), Float(ry), Float(sy), Float(tx), Float(ty)]]
                rescue ArgumentError
                    raise ArgumentError, "arguments must be convertable to float (got #{sx.class}, #{rx.class}, #{ry.class}, #{sy.class}, #{sx.class}, #{sx.class}, #{tx.class}, #{ty.class})"
                end
                yield(self) if block_given?
                self
            end

            # Add <tt>tx</tt> to all x-coordinates and <tt>ty</tt>
            # to all y-coordinates. If <tt>ty</tt> is omitted it defaults
            # to <tt>tx</tt>.
            def translate(tx, ty=nil)
                ty ||= tx
                begin
                    @transforms << [:translate, [Float(tx), Float(ty)]]
                rescue ArgumentError
                    raise ArgumentError, "arguments must be convertable to float (got #{tx.class}, #{ty.class})"
                end
                yield(self) if block_given?
                self
            end

            # Multiply the x-coordinates by <tt>sx</tt> and the y-coordinates
            # by <tt>sy</tt>. If <tt>sy</tt> is omitted it defaults to <tt>sx</tt>.
            def scale(sx, sy=nil)
                sy ||= sx
                begin
                    @transforms << [:scale, [Float(sx), Float(sy)]]
                rescue ArgumentError
                    raise ArgumentError, "arguments must be convertable to float (got #{sx.class}, #{sy.class})"
                end
                yield(self) if block_given?
                self
            end

            # This method can take either of two argument lists:
            # [rotate(angle)] rotate by <tt>angle</tt> degrees
            # [rotate(angle, cx, cy)] rotate by <tt>angle</tt> degrees about
            #                         the point [<tt>cx</tt>, <tt>cy</tt>].
            def rotate(angle, *args)
                begin
                    case args.length
                        when 0
                            @transforms << [:rotate, [Float(angle)]]
                        when 2
                            cx, cy = Float(args[0]), Float(args[1])
                            @transforms << [:translate, [cx, cy]]
                            @transforms << [:rotate, [angle]]
                            @transforms << [:translate, [-cx, -cy]]
                        else
                            raise ArgumentError, "wrong number of arguments (#{args.length} for 1 or 3)"
                    end
                rescue ArgumentError
                    raise ArgumentError, "arguments must be convertable to float (got #{[angle, *args].collect {|a| a.class}.join(', ')})"
                end
                yield(self) if block_given?
                self
            end

            # Skew the X-axis by <tt>angle</tt> degrees.
            def skewX(angle)
                begin
                    @transforms << [:skewx, [Float(angle)]]
                rescue ArgumentError
                    raise ArgumentError, "argument must be convertable to float (got #{angle.class})"
                end
                yield(self) if block_given?
                self
            end

            # Skew the Y-axis by <tt>angle</tt> degrees.
            def skewY(angle)
                begin
                    @transforms << [:skewy, [Float(angle)]]
                rescue ArgumentError
                    raise ArgumentError, "argument must be convertable to float (got #{angle.class})"
                end
                yield(self) if block_given?
                self
            end

        end     # module Transformable

    end # class RVG
end # module Magick

