# $Id: RMagick.rb,v 1.15 2004/01/31 18:32:56 rmagick Exp $
#==============================================================================
#                  Copyright (C) 2004 by Timothy P. Hunter
#   Name:       RMagick.rb
#   Author:     Tim Hunter
#   Purpose:    Extend Ruby to interface with ImageMagick.
#   Notes:      RMagick.so defines the classes. The code below adds methods
#               to the classes.
#==============================================================================

require 'RMagick.so'

module Magick
    @@formats = nil

def Magick.formats(&block)
    @@formats ||= Magick.init_formats
    if block_given?
        @@formats.each { |k,v| yield(k,v) }
        self
    else
        @@formats
    end
end


# Geometry class and related enum constants
class GeometryValue < Enum
    # no methods
end

PercentGeometry  = GeometryValue.new(:PercentGeometry, 1)
AspectGeometry   = GeometryValue.new(:AspectGeometry, 2)
LessGeometry     = GeometryValue.new(:LessGeometry, 3)
GreaterGeometry  = GeometryValue.new(:GreaterGeometry, 4)
AreaGeometry     = GeometryValue.new(:AreaGeometry, 5)

class Geometry
    FLAGS = ['', '%', '!', '<', '>', '@']
    RFLAGS = { '%' => PercentGeometry,
               '!' => AspectGeometry,
               '<' => LessGeometry,
               '>' => GreaterGeometry,
               '@' => AreaGeometry }

    attr_accessor :width, :height, :x, :y, :flag

    def initialize(width=nil, height=nil, x=nil, y=nil, flag=nil)

        # Support floating-point width and height arguments so Geometry
        # objects can be used to specify Image#density= arguments.
        if width == nil
            @width = 0
        elsif width.to_f >= 0.0
            @width = width.to_f
        else
            raise ArgumentError, "width must be >= 0: #{width}"
        end
        if height == nil
            @height = 0
        elsif height.to_f >= 0.0
            @height = height.to_f
        else
            raise ArgumentError, "height must be >= 0: #{height}"
        end

        @x    = x.to_i
        @y    = y.to_i
        @flag = flag.to_i
    end

    # Construct an object from a geometry string
    RE = /\A(\d*)(?:x(\d+))?([-+]\d+)?([-+]\d+)?([%!<>@]?)\Z/

    def Geometry.from_s(str)
        raise(ArgumentError, "no geometry string specified") unless str

        m = RE.match(str)
        if m
            width  = m[1].to_i
            height = m[2].to_i
            x      = m[3].to_i
            y      = m[4].to_i
            flag   = RFLAGS[m[5]]
        else
            raise ArgumentError, "invalid geometry format"
        end
        Geometry.new(width, height, x, y, flag)
    end

    # Convert object to a geometry string
    def to_s
        str = ''
        str << sprintf("%g", @width) if @width > 0
        str << sprintf("x%g", @height) if @height > 0
        str << sprintf("%+d%+d", @x, @y) if (@x != 0 || @y != 0)
        str << FLAGS[@flag]
    end
end


class Draw

    # Thse hashes are used to map Magick constant
    # values to the strings used in the primitives.
    ALIGN_TYPE_NAMES = {
        LeftAlign.to_i => 'left',
        RightAlign.to_i => 'right',
        CenterAlign.to_i => 'center'
        }
    ANCHOR_TYPE_NAMES = {
        StartAnchor.to_i => 'start',
        MiddleAnchor.to_i => 'middle',
        EndAnchor.to_i => 'end'
        }
    DECORATION_TYPE_NAMES = {
        NoDecoration.to_i => 'none',
        UnderlineDecoration.to_i => 'underline',
        OverlineDecoration.to_i => 'overline',
        LineThroughDecoration.to_i => 'line-through'
        }
    FONT_WEIGHT_NAMES = {
        AnyWeight.to_i => 'all',
        NormalWeight.to_i => 'normal',
        BoldWeight.to_i => 'bold',
        BolderWeight.to_i => 'bolder',
        LighterWeight.to_i => 'lighter',
        }
    GRAVITY_NAMES = {
        NorthWestGravity.to_i => 'northwest',
        NorthGravity.to_i => 'north',
        NorthEastGravity.to_i => 'northeast',
        WestGravity.to_i => 'west',
        CenterGravity.to_i => 'center',
        EastGravity.to_i => 'east',
        SouthWestGravity.to_i => 'southwest',
        SouthGravity.to_i => 'south',
        SouthEastGravity.to_i => 'southeast'
        }
    PAINT_METHOD_NAMES = {
        PointMethod.to_i => 'point',
        ReplaceMethod.to_i => 'replace',
        FloodfillMethod.to_i => 'floodfill',
        FillToBorderMethod.to_i => 'filltoborder',
        ResetMethod.to_i => 'reset'
        }
    STRETCH_TYPE_NAMES = {
        NormalStretch.to_i => 'normal',
        UltraCondensedStretch.to_i => 'ultra-condensed',
        ExtraCondensedStretch.to_i => 'extra-condensed',
        CondensedStretch.to_i => 'condensed',
        SemiCondensedStretch.to_i => 'semi-condensed',
        SemiExpandedStretch.to_i => 'semi-expanded',
        ExpandedStretch.to_i => 'expanded',
        ExtraExpandedStretch.to_i => 'extra-expanded',
        UltraExpandedStretch.to_i => 'ultra-expanded',
        AnyStretch.to_i => 'all'
        }
    STYLE_TYPE_NAMES = {
        NormalStyle.to_i => 'normal',
        ItalicStyle.to_i => 'italic',
        ObliqueStyle.to_i => 'oblique',
        AnyStyle.to_i => 'all'
        }

    # Apply coordinate transformations to support scaling (s), rotation (r),
    # and translation (t). Angles are specified in radians.
    def affine(sx, rx, ry, sy, tx, ty)
        primitive "affine " + sprintf("%g,%g,%g,%g,%g,%g", sx, rx, ry, sy, tx, ty)
    end

    # Draw an arc.
    def arc(startX, startY, endX, endY, startDegrees, endDegrees)
        primitive "arc " + sprintf("%g,%g %g,%g %g,%g",
                    startX, startY, endX, endY, startDegrees, endDegrees)
    end

    # Draw a bezier curve.
    def bezier(*points)
        if points.length == 0
            raise ArgumentError, "no points specified"
        elsif points.length % 2 != 0
            raise ArgumentError, "odd number of arguments specified"
        end
        primitive "bezier " + points.join(',')
    end

    # Draw a circle
    def circle(originX, originY, perimX, perimY)
        primitive "circle " + sprintf("%g,%g %g,%g", originX, originY, perimX, perimY)
    end

    # Invoke a clip-path defined by def_clip_path.
    def clip_path(name)
        primitive "clip-path #{name}"
    end

    # Define the clipping rule.
    def clip_rule(rule)
        if ( not ["evenodd", "nonzero"].include?(rule.downcase) )
            raise ArgumentError, "Unknown clipping rule #{rule}"
        end
        primitive "clip-rule #{rule}"
    end

    # Define the clip units
    def clip_units(unit)
        if ( not ["userspace", "userspaceonuse", "objectboundingbox"].include?(unit.downcase) )
            raise ArgumentError, "Unknown clip unit #{unit}"
        end
        primitive "clip-units #{unit}"
    end

    # Set color in image according to specified colorization rule. Rule is one of
    # point, replace, floodfill, filltoborder,reset
    def color(x, y, method)
        if ( not PAINT_METHOD_NAMES.has_key?(method.to_i) )
            raise ArgumentError, "Unknown PaintMethod: #{method}"
        end
        primitive "color #{x},#{y},#{PAINT_METHOD_NAMES[method.to_i]}"
    end

    # Specify EITHER the text decoration (none, underline, overline,
    # line-through) OR the text solid background color (any color name or spec)
    def decorate(decoration)
        if ( DECORATION_TYPE_NAMES.has_key?(decoration.to_i) )
            primitive "decorate #{DECORATION_TYPE_NAMES[decoration.to_i]}"
        else
            primitive "decorate #{decoration}"
        end
    end

    # Define a clip-path. A clip-path is a sequence of primitives
    # bracketed by the "push clip-path <name>" and "pop clip-path"
    # primitives. Upon advice from the IM guys, we also bracket
    # the clip-path primitives with "push(pop) defs" and "push
    # (pop) graphic-context".
    def define_clip_path(name)
        begin
            push('defs')
            push('clip-path ', name)
            push('graphic-context')
            yield
        ensure
            pop('graphic-context')
            pop('clip-path')
            pop('defs')
        end
    end

    # Draw an ellipse
    def ellipse(originX, originY, width, height, arcStart, arcEnd)
        primitive "ellipse " + sprintf("%g,%g %g,%g %g,%g",
                        originX, originY, width, height, arcStart, arcEnd)
    end

    # Let anything through, but the only defined argument
    # is "UTF-8". All others are apparently ignored.
    def encoding(encoding)
        primitive "encoding #{encoding}"
    end

    # Specify object fill, a color name or pattern name
    def fill(colorspec)
        primitive "fill #{colorspec}"
    end
    alias fill_color fill
    alias fill_pattern fill

    # Specify fill opacity (use "xx%" to indicate percentage)
    def fill_opacity(opacity)
        primitive "fill-opacity #{opacity}"
    end

    def fill_rule(rule)
        if ( not ["evenodd", "nonzero"].include?(rule.downcase) )
            raise ArgumentError, "Unknown fill rule #{rule}"
        end
        primitive "fill-rule #{rule}"
    end

    # Specify text drawing font
    def font(name)
        primitive "font #{name}"
    end

    def font_family(name)
        primitive "font-family \'#{name}\'"
    end

    def font_stretch(stretch)
        if ( not STRETCH_TYPE_NAMES.has_key?(stretch.to_i) )
            raise ArgumentError, "Unknown stretch type"
        end
        primitive "font-stretch #{STRETCH_TYPE_NAMES[stretch.to_i]}"
    end

    def font_style(style)
        if ( not STYLE_TYPE_NAMES.has_key?(style.to_i) )
            raise ArgumentError, "Unknown style type"
        end
        primitive "font-style #{STYLE_TYPE_NAMES[style.to_i]}"
    end

    # The font weight argument can be either a font weight
    # constant or [100,200,...,900]
    def font_weight(weight)
        if ( FONT_WEIGHT_NAMES.has_key?(weight.to_i) )
            primitive "font-weight #{FONT_WEIGHT_NAMES[weight.to_i]}"
        else
            primitive "font-weight #{weight}"
        end
    end

    # Specify the text positioning gravity, one of:
    # NorthWest, North, NorthEast, West, Center, East, SouthWest, South, SouthEast
    def gravity(grav)
        if ( not GRAVITY_NAMES.has_key?(grav.to_i) )
            raise ArgumentError, "Unknown text positioning gravity"
        end
        primitive "gravity #{GRAVITY_NAMES[grav.to_i]}"
    end

    # Draw a line
    def line(startX, startY, endX, endY)
        primitive "line " + sprintf("%g,%g %g,%g", startX, startY, endX, endY)
    end

    # Set matte (make transparent) in image according to the specified
    # colorization rule
    def matte(x, y, rule)
        if ( not PAINT_METHOD_NAMES.has_key?(method.to_i) )
            raise ArgumentError, "Unknown paint method"
        end
        primitive "matte #{x},#{y} #{PAINT_METHOD_NAMES[method.to_i]}"
    end

    # Specify drawing fill and stroke opacities. If the value is a string
    # ending with a %, the number will be multiplied by 0.01.
    def opacity(opacity)
        if (Numeric === opacity)
            if (opacity < 0 || opacity > 1.0)
                raise ArgumentError, "opacity must be >= 0 and <= 1.0"
            end
        end
        primitive "opacity #{opacity}"
    end

    # Draw using SVG-compatible path drawing commands. Note that the
    # primitive requires that the commands be surrounded by quotes or
    # apostrophes. Here we simply use apostrophes.
    def path(cmds)
        primitive "path '" + cmds + "'"
    end

    # Define a pattern. In the block, call primitive methods to
    # draw the pattern. Reference the pattern by using its name
    # as the argument to the 'fill' or 'stroke' methods
    def pattern(name, x, y, width, height)
        begin
            push('defs')
            push("pattern #{name} #{x} #{y} #{width} #{height}")
            push('graphic-context')
            yield
        ensure
            pop('graphic-context')
            pop('pattern')
            pop('defs')
        end
    end

    # Set point to fill color.
    def point(x, y)
        primitive "point #{x},#{y}"
    end

    # Specify the font size in points. Yes, the primitive is "font-size" but
    # in other places this value is called the "pointsize". Give it both names.
    def pointsize(points)
        primitive "font-size #{points}"
    end
    alias font_size pointsize

    # Draw a polygon
    def polygon(*points)
        if points.length == 0
            raise ArgumentError, "no points specified"
        elsif points.length % 2 != 0
            raise ArgumentError, "odd number of points specified"
        end
        primitive "polygon " + points.join(',')
    end

    # Draw a polyline
    def polyline(*points)
        if points.length == 0
            raise ArgumentError, "no points specified"
        elsif points.length % 2 != 0
            raise ArgumentError, "odd number of points specified"
        end
        primitive "polyline " + points.join(',')
    end

    # Return to the previously-saved set of whatever
    # pop('graphic-context') (the default if no arguments)
    # pop('defs')
    # pop('gradient')
    # pop('pattern')

    def pop(*what)
        if what.length == 0
            primitive "pop graphic-context"
        else
            # to_s allows a Symbol to be used instead of a String
            primitive "pop " + what.to_s
        end
    end

    # Push the current set of drawing options. Also you can use
    # push('graphic-context') (the default if no arguments)
    # push('defs')
    # push('gradient')
    # push('pattern')
    def push(*what)
        if what.length == 0
            primitive "push graphic-context"
        else
            # to_s allows a Symbol to be used instead of a String
            primitive "push " + what.to_s
        end
    end

    # Draw a rectangle
    def rectangle(upper_left_x, upper_left_y, lower_right_x, lower_right_y)
        primitive "rectangle " + sprintf("%g,%g %g,%g",
                upper_left_x, upper_left_y, lower_right_x, lower_right_y)
    end

    # Specify coordinate space rotation. "angle" is measured in degrees
    def rotate(angle)
        primitive "rotate #{angle}"
    end

    # Draw a rectangle with rounded corners
    def roundrectangle(center_x, center_y, width, height, corner_width, corner_height)
        primitive "roundrectangle " + sprintf("%g,%g,%g,%g,%g,%g",
            center_x, center_y, width, height, corner_width, corner_height)
    end

    # Specify scaling to be applied to coordinate space on subsequent drawing commands.
    def scale(x, y)
        primitive "scale #{x},#{y}"
    end

    def skewx(angle)
        primitive "skewX #{angle}"
    end

    def skewy(angle)
        primitive "skewY #{angle}"
    end

    # Specify the object stroke, a color name or pattern name.
    def stroke(colorspec)
        primitive "stroke #{colorspec}"
    end
    alias stroke_color stroke
    alias stroke_pattern stroke

    # Specify if stroke should be antialiased or not
    def stroke_antialias(bool)
        bool = bool ? '1' : '0'
        primitive "stroke-antialias #{bool}"
    end

    # Specify a stroke dash pattern
    def stroke_dasharray(*list)
        if list.length == 0
            primitive "stroke-dasharray none"
        else
            list.each { |x|
                if x <= 0 then
                    raise ArgumentError, "dash array elements must be > 0 (#{x} given)"
                end
            }
            primitive "stroke-dasharray #{list.join(',')}"
        end
    end

    # Specify the initial offset in the dash pattern
    def stroke_dashoffset(value=0)
        primitive "stroke-dashoffset #{value}"
    end

    def stroke_linecap(value)
        if ( not ["butt", "round", "square"].include?(value.downcase) )
            raise ArgumentError, "Unknown linecap type: #{value}"
        end
        primitive "stroke-linecap #{value}"
    end

    def stroke_linejoin(value)
        if ( not ["round", "miter", "bevel"].include?(value.downcase) )
            raise ArgumentError, "Unknown linejoin type: #{value}"
        end
        primitive "stroke-linejoin #{value}"
    end

    def stroke_miterlimit(value)
        if (value < 1)
            raise ArgumentError, "miterlimit must be >= 1"
        end
        primitive "stroke-miterlimit #{value}"
    end

    # Specify opacity of stroke drawing color
    #  (use "xx%" to indicate percentage)
    def stroke_opacity(value)
        primitive "stroke-opacity #{value}"
    end

    # Specify stroke (outline) width in pixels.
    def stroke_width(pixels)
        primitive "stroke-width #{pixels}"
    end

    # Draw text at position x,y. Generally it's best to surround the text with
    # quotes. For example,
    #      gc.text(100, 100, "'embedded blanks'")
    def text(x, y, text)
        if text.to_s.empty?
            raise ArgumentError, "missing text argument"
        end
        primitive "text #{x},#{y} #{text}"
    end

    # Specify text alignment relative to a given point
    def text_align(alignment)
        if ( not ALIGN_TYPE_NAMES.has_key?(alignment.to_i) )
            raise ArgumentError, "Unknown alignment constant: #{alignment}"
        end
        primitive "text-align #{ALIGN_TYPE_NAMES[alignment.to_i]}"
    end

    # SVG-compatible version of text_align
    def text_anchor(anchor)
        if ( not ANCHOR_TYPE_NAMES.has_key?(anchor.to_i) )
            raise ArgumentError, "Unknown anchor constant: #{anchor}"
        end
        primitive "text-anchor #{ANCHOR_TYPE_NAMES[anchor.to_i]}"
    end

    # Specify if rendered text is to be antialiased.
    def text_antialias(boolean)
        boolean = boolean ? '1' : '0'
        primitive "text-antialias #{boolean}"
    end

    # Specify color underneath text
    def text_undercolor(color)
        primitive "text-undercolor #{color}"
    end

    # Specify center of coordinate space to use for subsequent drawing
    # commands.
    def translate(x, y)
        primitive "translate #{x},#{y}"
    end
end # class Magick::Draw

# Ruby-level Magick::Image methods
class Image
    include Comparable

    # Provide an alternate version of Draw#annotate, for folks who
    # want to find it in this class.
    def annotate(draw, width, height, x, y, text, &block)
      draw.annotate(self, width, height, x, y, text, &block)
      self
    end

    # Set the color at x,y
    def color_point(x, y, fill)
        f = copy
        f.pixel_color(x, y, fill)
        return f
    end

    # Set all pixels that have the same color as the pixel at x,y and
    # are neighbors to the fill color
    def color_floodfill(x, y, fill)
        target = pixel_color(x, y)
        color_flood_fill(target, fill, x, y, Magick::FloodfillMethod)
    end

    # Set all pixels that are neighbors of x,y and are not the border color
    # to the fill color
    def color_fill_to_border(x, y, fill)
        color_flood_fill(border_color, fill, x, y, Magick::FillToBorderMethod)
    end

    # Set all pixels to the fill color. Very similar to Image#erase!
    # Accepts either String or Pixel arguments
    def color_reset!(fill)
        save = background_color
        begin
            self.background_color = fill
            erase!
        ensure
            self.background_color = save
        end
        self
    end

    # Used by ImageList methods - see ImageList#cur_image
    def cur_image
        self
    end

    # These four methods are equivalent to the Draw#matte
    # method with the "Point", "Replace", "Floodfill", "FilltoBorder", and
    # "Replace" arguments, respectively.

    # Make the pixel at (x,y) transparent.
    def matte_point(x, y)
        f = copy
        f.opacity = OpaqueOpacity unless f.matte
        pixel = f.pixel_color(x,y)
        pixel.opacity = TransparentOpacity
        f.pixel_color(x, y, pixel)
        return f
    end

    # Make transparent all pixels that are the same color as the
    # pixel at (x, y).
    def matte_replace(x, y)
        f = copy
        f.opacity = OpaqueOpacity unless f.matte
        target = f.pixel_color(x, y)
        f.transparent(target)
    end

    # Make transparent any pixel that matches the color of the pixel
    # at (x,y) and is a neighbor.
    def matte_floodfill(x, y)
        f = copy
        f.opacity = OpaqueOpacity unless f.matte
        target = f.pixel_color(x, y)
        f.matte_flood_fill(target, TransparentOpacity,
                           x, y, FloodfillMethod)
    end

    # Make transparent any neighbor pixel that is not the border color.
    def matte_fill_to_border(x, y)
        f = copy
        f.opacity = Magick::OpaqueOpacity unless f.matte
        f.matte_flood_fill(border_color, TransparentOpacity,
                           x, y, FillToBorderMethod)
    end

    # Make all pixels transparent.
    def matte_reset!
        self.opacity = Magick::TransparentOpacity
    end

    # Replace matching neighboring pixels with texture pixels
    def texture_floodfill(x, y, texture)
        target = pixel_color(x, y)
        texture_flood_fill(target, texture, x, y, FloodfillMethod)
    end

    # Replace neighboring pixels to border color with texture pixels
    def texture_fill_to_border(x, y, texture)
        texture_flood_fill(border_color, texture, x, y, FillToBorderMethod)
    end

    # Retrieve EXIF data by entry or all. If one or more entry names specified,
    # return the values associated with the entries. If no entries specified,
    # return all entries and values. The return value is an array of [name,value]
    # arrays.
    def get_exif_by_entry(*entry)
        ary = Array.new
        if entry.length == 0
            exif_data = self['EXIF:*']
            if exif_data
                exif_data.split("\n").each { |exif| ary.push(exif.split('=')) }
            end
        else
            entry.each do |name|
                rval = self["EXIF:#{name}"]
                ary.push([name, rval])
            end
        end
        return ary
    end

    # Retrieve EXIF data by tag number or all tag/value pairs. The return value is a hash.
    def get_exif_by_number(*tag)
        hash = Hash.new
        if tag.length == 0
            exif_data = self['EXIF:!']
            if exif_data
                exif_data.split("\n").each do |exif|
                    tag, value = exif.split('=')
                    tag = tag[1,4].hex
                    hash[tag] = value
                end
            end
        else
            tag.each do |num|
                rval = self["EXIF:#{'#%04X' % num}"]
                hash[num] = rval == 'unknown' ? nil : rval
            end
        end
        return hash
    end

end # class Magick::Image

class ImageList < Array

    include Comparable

    undef_method :assoc
    undef_method :flatten!      # These methods are undefined
    undef_method :flatten       # because they're not useful
    undef_method :join          # for an ImageList object
    undef_method :pack
    undef_method :rassoc
    undef_method :transpose if Array.instance_methods(false).include? 'transpose'
    undef_method :zip if Array.instance_methods(false).include? 'zip'

    attr_reader :scene

protected

    def is_a_image(obj)
        unless obj.kind_of? Magick::Image
            raise ArgumentError, "Magick::Image required (#{obj.class} given)"
        end
    end

    # Ensure array is always an array of Magick::Image objects
    def is_a_image_array(ary)
        ary.each { |obj| is_a_image obj }
    end

    # Find old current image, update @scene
    # cfid is the id of the old current image.
    def set_cf(cfid)
        if length == 0
            @scene = nil
            return
        # Don't bother looking for current image
        elsif @scene == nil || @scene >= length
            @scene = length - 1
            return
        elsif cfid != nil
            each_with_index do |f,i|
                if f.__id__ == cfid
                    @scene = i
                    return
                end
            end
        end
        @scene = length - 1
    end

public

    # Allow scene to be set to nil
    def scene=(n)
        if (length == 0 && n != nil) || (length > 0 && n == nil)
            raise IndexError, "scene number out of bounds"
        elsif (length == 0) || (Integer(n) < 0) || (Integer(n) > length - 1)
            raise IndexError, "scene number out of bounds"
        end
        @scene = Integer(n)
    end

    def [](*args)
        if (args.length > 1) || args[0].kind_of?(Range)
            self.class.new.replace super
        else
            super
        end
    end

    def []=(*args)
        if args.length == 3             # f[start,length] = [f1,f2...]
            is_a_image_array args[2]
            super
            if args[1] > 0
                @scene = args[0] + args[1] - 1
            else                        # inserts elements if length == 0
                @scene = args[0] + args[2].length - 1
            end
        elsif args[0].kind_of? Range    # f[first..last] = [f1,f2...]
            is_a_image_array args[1]
            super
            @scene = args[0].end
        else                            # f[index] = f1
            is_a_image args[1]
            super                       # index can be negative
            @scene = args[0] < 0 ? length + args[0] : args[0]
        end
        args.last                       # return value is always assigned value
    end

    def &(other)
        is_a_image_array other
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def *(n)
        unless n.kind_of? Integer
            raise ArgumentError, "Integer required (#{n.class} given)"
        end
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def +(other)
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def -(other)
        is_a_image_array other
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def <<(obj)
        is_a_image obj
        a = super
        @scene = length-1
        return a
    end

    def |(other)
        is_a_image_array other
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def clear
        @scene = nil
        super
    end

    def collect(&block)
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def collect!(&block)
        super
        is_a_image_array self
        self
    end

    def compact
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def compact!
        cfid = self[@scene].__id__ rescue nil
        a = super          # returns nil if no changes were made
        set_cf cfid
        return a
    end

    def concat(other)
        is_a_image_array other
        a = super
        @scene = length-1
        return a
    end

    def delete(obj, &block)
        is_a_image obj
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    def delete_at(ndx)
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    def delete_if(&block)
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    def fill(obj, *args)
        is_a_image obj
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    if Array.instance_methods(false).include? 'fetch' then
        def fetch(*args,&block)
            super
        end
    end

    if Array.instance_methods(false).include? 'insert' then
        def insert(*args)
            cfid = self[@scene].__id__ rescue nil
            a = self.class.new.replace super
            a.set_cf cfid
            return a
        end
    end

    # __map__ is a synonym for Array#map. We've used an alias
    # so it doesn't conflict with our own map method.
    if Array.instance_methods(false).include? '__map__' then
        def __map__(&block)
            cfid = self[@scene].__id__ rescue nil
            a = self.class.new.replace super
            a.set_cf cfid
            return a
        end
    end

    def map!(&block)
        super
        is_a_image_array self
        self
    end

    def pop
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    def push(*objs)
        objs.each { |o| is_a_image o }
        super
        @scene = length - 1
        self
    end

    if Array.instance_methods(false).include? 'reject' then
        def reject(&block)
            cfid = self[@scene].__id__ rescue nil
            a = super
            set_cf cfid
            return a
        end
    end

    def reject!(&block)
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    def replace(other)
        is_a_image_array other
        # Since replace gets called so frequently when @scene == nil
        # test for it instead of letting rescue catch it.
        cfid = nil
        if @scene then
            cfid = self[@scene].__id__ rescue nil
        end
        super
        set_cf cfid
        self
    end

    def reverse
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def reverse!
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    if Array.instance_methods(false).include? 'select' then
        def select(*args,&block)
            cfid = self[@scene].__id__ rescue nil
            a = super
            a.set_cf cfid
            return a
        end
    end

    def shift
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    def slice(*args)
        self[*args]
    end

    def slice!(*args)
        cfid = self[@scene].__id__ rescue nil
        if args.length > 1 || args[0].kind_of?(Range)
            a = self.class.new.replace super
        else
            a = super
        end
        set_cf cfid
        return a
    end

    def uniq
        cfid = self[@scene].__id__ rescue nil
        a = self.class.new.replace super
        a.set_cf cfid
        return a
    end

    def uniq!(*args)
        cfid = self[@scene].__id__ rescue nil
        a = super
        set_cf cfid
        return a
    end

    # @scene -> new object
    def unshift(obj)
        is_a_image obj
        a = super
        @scene = 0
        return a
    end

    # Compare ImageLists
    # Compare each image in turn until the result of a comparison
    # is not 0. If all comparisons return 0, then
    #   return if A.scene != B.scene
    #   return A.length <=> B.length
    def <=>(other)
        unless other.kind_of? self.class
           raise TypeError, "#{self.class} required (#{other.class} given)"
        end
        size = [self.length, other.length].min
        size.times do |x|
            r = self[x] <=> other[x]
            return r unless r == 0
        end
        r = self.scene <=> other.scene
        return r unless r == 0
        return self.length <=> other.length
    end

    # Make a deep copy (see also dup)
    def copy
        copy_img = ImageList.new
        each { |f| copy_img << f.copy }
        copy_img.scene = @scene
        copy_img
    end

    # Make a shallow copy (aka clone, see also copy)
    def dup
        copy_img = ImageList.new
        each { |f| copy_img << f }
        copy_img.scene = @scene
        copy_img
    end
    alias clone dup

    # Return the current image
    def cur_image
        if ! @scene
            raise IndexError, "no images in this list"
        end
        self[@scene]
    end

    # Set same delay for all images
    def delay=(d)
        if Integer(d) < 0
            raise ArgumentError, "delay must be greater than 0"
        end
        each { |f| f.delay = Integer(d) }
    end

    def from_blob(*blobs, &block)
        if (blobs.length == 0)
            raise ArgumentError, "no blobs given"
        end
        blobs.each { |b|
            Magick::Image.from_blob(b, &block).each { |n| self << n  }
            }
        @scene = length - 1
        self
    end

    # Initialize new instances
    def initialize(*filenames)
        @scene = nil
        filenames.each { |f|
            Magick::Image.read(f).each { |n| self << n }
            }
        if length > 0
            @scene = length - 1     # last image in array
        end
        self
    end

    # Call inspect for all the images
    def inspect
        ins = '['
        each {|image| ins << image.inspect << "\n"}
        ins.chomp("\n") + "]\nscene=#{@scene}"
    end

    # Set the number of iterations of an animated GIF
    def iterations=(n)
        if n < 0 || n > 65535
            raise ArgumentError, "iterations must be between 0 and 65535"
        end
        each {|f| f.iterations=n}
        self
    end

    # The ImageList class supports the Magick::Image class methods by simply sending
    # the method to the current image. If the method isn't explicitly supported,
    # send it to the current image in the array. If there are no images, send
    # it up the line. Catch a NameError and emit a useful message.
    def method_missing(methID, *args, &block)
        begin
            if @scene
                self[@scene].send(methID, *args, &block)
            else
                super
            end
        rescue NameError
          raise NameError, "undefined method `#{methID.object_id2name}' for #{self.class}"
        rescue Exception
            $@.delete_if { |s| /:in `send'$/.match(s) || /:in `method_missing'$/.match(s) }
            raise
        end
    end

    # Ensure respond_to? answers correctly when we are delegating to Image
    alias_method :__respond_to__?, :respond_to?
    def respond_to?(methID, priv=false)
        return true if __respond_to__?(methID, priv)
        if @scene
            self[@scene].respond_to?(methID, priv)
        else
            super
        end
    end

    # Create a new image and add it to the end
    def new_image(cols, rows, *fill, &info_blk)
        self << Magick::Image.new(cols, rows, *fill, &info_blk)
    end

    # Ping files and concatenate the new images
    def ping(*files, &block)
        if (files.length == 0)
            raise ArgumentError, "no files given"
        end
        files.each { |f|
            Magick::Image.ping(f, &block).each { |n| self << n }
            }
        @scene = length - 1
        self
    end

    # Read files and concatenate the new images
    def read(*files, &block)
        if (files.length == 0)
            raise ArgumentError, "no files given"
        end
        files.each { |f|
            Magick::Image.read(f, &block).each { |n| self << n }
            }
        @scene = length - 1
        self
    end
end # Magick::ImageList

# Example fill class. Fills the image with the specified background
# color, then crosshatches with the specified crosshatch color.
# @dist is the number of pixels between hatch lines.
# See Magick::Draw examples.
class HatchFill
   def initialize(bgcolor, hatchcolor="white", dist=10)
      @bgcolor = bgcolor
      @hatchpixel = Pixel.from_color(hatchcolor)
      @dist = dist
   end

   def fill(img)                # required
      img.background_color = @bgcolor
      img.erase!                # sets image to background color
      pixels = Array.new([img.rows, img.columns].max, @hatchpixel)
      @dist.step((img.columns-1)/@dist*@dist, @dist) { |x|
         img.store_pixels(x,0,1,img.rows,pixels)
      }
      @dist.step((img.rows-1)/@dist*@dist, @dist) { |y|
         img.store_pixels(0,y,img.columns,1,pixels)
      }
   end
end

end # Magick
