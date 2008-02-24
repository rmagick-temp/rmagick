#--
# $Id: stylable.rb,v 1.5 2008/02/24 18:26:37 rmagick Exp $
# Copyright (C) 2008 Timothy P. Hunter
#++

module Magick
    class RVG

        #:stopdoc:
        STYLES = [:clip_path, :clip_rule, :fill, :fill_opacity, :fill_rule, :font,
            :font_family, :font_size, :font_stretch, :font_style, :font_weight,
            :opacity, :stroke, :stroke_dasharray, :stroke_dashoffset, :stroke_linecap,
            :stroke_linejoin, :stroke_miterlimit, :stroke_opacity, :stroke_width,
            :text_anchor, :text_decoration,
            :glyph_orientation_vertical, :glyph_orientation_horizontal,
            :letter_spacing, :word_spacing, :baseline_shift, :writing_mode]

        Styles = Struct.new(*STYLES)

        # Styles is a Struct class with a couple of extra methods
        class Styles

            def set(styles)
                begin
                    styles.each_pair do |style, value|
                        begin
                            self[style] = value
                        rescue NoMethodError
                            raise ArgumentError, "unknown style `#{style}'"
                        end
                    end
                rescue NoMethodError
                    raise ArgumentError, "style arguments must be in the form `style => value'"
                end
                self
            end

            # Iterate over the style names. Yield for each style that has a value.
            def each_value
                each_pair do |style, value|
                    yield(style, value) if value
                end
            end

            # The "usual" deep_copy method doesn't copy a Struct correctly.
            def deep_copy(h=nil)
                copy = Styles.new
                each_pair { |style, value| copy[style] = value }
                return copy
            end

        end     # class Styles

        #:startdoc:

        # This module is mixed into classes that can have styles.
        module Stylable

          private

            # For each style that has a value, add a style primitive to the gc.
            # Use splat to splat out Array arguments such as stroke_dasharray.
            def add_style_primitives(gc)
                @styles.each_value do |style, value|
                    if value.respond_to? :add_primitives
                        value.add_primitives(gc, style)
                    else
                        gc.__send__(style, *value)
                    end
                end
            end

            def initialize
                super
                @styles = Styles.new
            end

          public

            # This method can be used with any RVG, Group, Use, Text, or
            # shape object. The argument is a hash. The style names are
            # the hash keys. The style names and values are:
            # [:clip_path] clipping path defined by clip_path
            # [:clip_rule] 'evenodd' or 'nozero'
            # [:fill] color name
            # [:fill_opacity] the fill opacity, 0.0<=N<=1.0
            # [:fill_rule] 'evenodd' or 'nozero'
            # [:font] font name or font file name
            # [:font_family] font family name, ex. 'serif'
            # [:font_size] font size in points
            # [:font_stretch] 'normal','ultra_condensed','extra_condensed',
            #                 'condensed','semi_condensed','semi_expanded',
            #                 'expanded','extra_expanded','ultra_expanded'
            # [:font_style] 'normal','italic','oblique'
            # [:font_weight] 'normal','bold','bolder','lighter', or
            #                a multiple of 100 between 100 and 900.
            # [:opacity] both fill and stroke opacity, 0.0<=N<=1.0
            # [:stroke] color name
            # [:stroke_dasharray] dash pattern (Array)
            # [:stroke_dashoffset] initial distance into dash pattern
            # [:stroke_linecap] 'butt', 'round', 'square'
            # [:stroke_linejoin] 'miter', 'round', 'bevel'
            # [:stroke_miterlimit] miter length constraint
            # [:stroke_opacity] the stroke opacity, 0.0<=N<=1.0
            # [:stroke_width] stroke width
            # [:text_anchor] 'start','middle','end'
            # [:text_decoration] 'none','underline','overline','line_through'
            def styles(styles)
                @styles.set(styles)
                yield(self) if block_given?
                self
            end

        end     # module Stylable

    end # class RVG
end # module Magick

