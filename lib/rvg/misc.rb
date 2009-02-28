# $Id: misc.rb,v 1.15 2009/02/28 23:52:27 rmagick Exp $
# Copyright (C) 2009 Timothy P. Hunter
module Magick
    class RVG

        # This is a standard deep_copy method that is used in most classes.
        # Thanks to Robert Klemme.
        module Duplicatable

            def deep_copy(h = {})
                # Prevent recursion. If we reach the
                # object we started with, stop copying.
                copy = h[__id__]
                unless copy
                    h[__id__] = copy = self.class.allocate
                    ivars = instance_variables
                    ivars.each do |ivar|
                        ivalue = instance_variable_get(ivar)
                        cvalue = case
                            when NilClass === ivalue, Symbol === ivalue, Float === ivalue,
                                 Fixnum === ivalue, FalseClass === ivalue, TrueClass === ivalue
                                ivalue
                            when ivalue.respond_to?(:deep_copy)
                                ivalue.deep_copy(h)
                            when ivalue.respond_to?(:dup)
                                ivalue.dup
                            else
                                ivalue
                            end
                        copy.instance_variable_set(ivar, cvalue)
                    end
                    copy.freeze if frozen?
                end
                return copy
            end

        end     # module Duplicatable


        # Convert an array of method arguments to Float objects. If any
        # cannot be converted, raise ArgumentError and issue a message.
        def self.fmsg(*args)
            "at least one argument cannot be converted to Float (got #{args.collect {|a| a.class}.join(', ')})"
        end

        def self.convert_to_float(*args)
            allow_nil = false
            if args.last == :allow_nil
                allow_nil = true
                args.pop
            end
            begin
                fargs = args.collect { |a| (allow_nil && a.nil?) ? a : Float(a) }
            rescue ArgumentError, TypeError
                raise ArgumentError, self.fmsg(*args)
            end
            return fargs
        end

        def self.convert_one_to_float(arg)
            begin
                farg = Float(arg)
            rescue ArgumentError, TypeError
                raise ArgumentError, "argument cannot be converted to Float (got #{arg.class})"
            end
            return farg
        end

    end # class RVG
end # module Magick





module Magick
    class RVG
        class Utility

            class TextStrategy

                def initialize(context)
                    @ctx = context
                    @ctx.shadow.affine = @ctx.text_attrs.affine
                end

                def enquote(text)
                    if text.length > 2 && /\A(?:\"[^\"]+\"|\'[^\']+\'|\{[^\}]+\})\z/.match(text)
                        return text
                    elsif !text['\'']
                        text = '\''+text+'\''
                        return text
                    elsif !text['"']
                        text = '"'+text+'"'
                        return text
                    elsif !(text['{'] || text['}'])
                        text = '{'+text+'}'
                        return text
                    end

                    # escape existing braces, surround with braces
                    text.gsub!(/[}]/) { |b| '\\' + b }
                    return '{' +  text + '}'
                end

                def glyph_metrics(glyph_orientation, glyph)
                    gm = @ctx.shadow.get_type_metrics("a" + glyph + "a")
                    gm2 = @ctx.shadow.get_type_metrics("aa")
                    h = (gm.ascent - gm.descent + 0.5 ).to_i
                    w = gm.width - gm2.width
                    if glyph_orientation == 0 || glyph_orientation == 180
                        [w, h]
                    else
                        [h, w]
                    end
                end

                def text_rel_coords(text)
                    y_rel_coords = []
                    x_rel_coords = []
                    first_word = true
                    words = text.split(::Magick::RVG::WORD_SEP)
                    words.each do |word|
                        unless first_word
                            wx, wy = get_word_spacing()
                            x_rel_coords << wx
                            y_rel_coords << wy
                        end
                        first_word = false
                        word.split('').each do |glyph|
                            wx, wy = get_letter_spacing(glyph)
                            x_rel_coords << wx
                            y_rel_coords << wy
                        end
                    end
                    [x_rel_coords, y_rel_coords]
                end

                def shift_baseline(glyph_orientation, glyph)
                    glyph_dimensions = @ctx.shadow.get_type_metrics(glyph)
                    if glyph_orientation == 0 || glyph_orientation == 180
                        x = glyph_dimensions.width
                    else
                        x = glyph_dimensions.ascent - glyph_dimensions.descent
                    end
                    case @ctx.text_attrs.baseline_shift
                        when :baseline
                            x = 0
                        when :sub
                            ;
                        when :super
                            x = -x
                        when /[-+]?(\d+)%/
                            m = $1 == '-' ? -1.0 : 1.0
                            x = (m * x * $1.to_f / 100.0)
                        else
                            x = -@ctx.text_attrs.baseline_shift
                    end
                    return x
                end

                def render_glyph(glyph_orientation, x, y, glyph)
                    if glyph_orientation == 0
                        @ctx.gc.text(x, y, enquote(glyph))
                    else
                        @ctx.gc.push
                        @ctx.gc.translate(x, y)
                        @ctx.gc.rotate(glyph_orientation)
                        @ctx.gc.translate(-x, -y)
                        @ctx.gc.text(x, y, enquote(glyph))
                        @ctx.gc.pop
                    end
                end

            end     # class TextStrategy

            class LRTextStrategy < TextStrategy

                def get_word_spacing()
                    @word_space ||= glyph_metrics(@ctx.text_attrs.glyph_orientation_horizontal, ' ')[0]
                    [@word_space + @ctx.text_attrs.word_spacing, 0]
                end

                def get_letter_spacing(glyph)
                    gx, gy = glyph_metrics(@ctx.text_attrs.glyph_orientation_horizontal, glyph)
                    [gx+@ctx.text_attrs.letter_spacing, gy]
                end

                def render(x, y, text)
                    x_rel_coords, y_rel_coords = text_rel_coords(text)
                    dx = x_rel_coords.inject(0) {|sum, a| sum + a}
                    dy = y_rel_coords.max

                    # We're handling the anchoring.
                    @ctx.gc.push()
                    @ctx.gc.text_anchor(Magick::StartAnchor)
                    if @ctx.text_attrs.text_anchor == :end
                        x -= dx
                    elsif @ctx.text_attrs.text_anchor == :middle
                        x -= dx / 2
                    end

                    # Align the first glyph
                    case @ctx.text_attrs.glyph_orientation_horizontal
                        when 0
                            ;
                        when 90
                            y -= dy
                        when 180
                            x += x_rel_coords.shift
                            x_rel_coords << 0
                            y -= dy
                        when 270
                            x += x_rel_coords[0]
                    end

                    y += shift_baseline(@ctx.text_attrs.glyph_orientation_horizontal, text[0,1])

                    first_word = true
                    text.split(::Magick::RVG::WORD_SEP).each do |word|
                        unless first_word
                            x += x_rel_coords.shift
                        end
                        first_word = false
                        word.split('').each do |glyph|
                            render_glyph(@ctx.text_attrs.glyph_orientation_horizontal, x, y, glyph)
                            x += x_rel_coords.shift
                        end
                    end

                    @ctx.gc.pop()
                    [dx, 0]
                end

            end     # class LRTextStrategy

            class RLTextStrategy < TextStrategy

                def render(x, y, text)
                    raise NotImplementedError
                end

            end     # class RLTextStrategy


            class TBTextStrategy < TextStrategy

                def get_word_spacing()
                    @word_space ||= glyph_metrics(@ctx.text_attrs.glyph_orientation_vertical, ' ')[1]
                    [0, @word_space + @ctx.text_attrs.word_spacing]
                end

                def get_letter_spacing(glyph)
                    gx, gy = glyph_metrics(@ctx.text_attrs.glyph_orientation_vertical, glyph)
                    [gx, gy+@ctx.text_attrs.letter_spacing]
                end

                def render(x, y, text)
                    x_rel_coords, y_rel_coords = text_rel_coords(text)
                    dx = x_rel_coords.max
                    dy = y_rel_coords.inject(0) {|sum, a| sum + a}

                    # We're handling the anchoring.
                    @ctx.gc.push()
                    @ctx.gc.text_anchor(Magick::StartAnchor)
                    if @ctx.text_attrs.text_anchor == :end
                        y -= dy
                    elsif @ctx.text_attrs.text_anchor == :middle
                        y -= dy / 2
                    end

                    # Align the first glyph such that its center
                    # is aligned on x and its top is aligned on y.

                    case @ctx.text_attrs.glyph_orientation_vertical
                        when 0
                            x -= x_rel_coords.max / 2
                            y += y_rel_coords[0]
                        when 90
                            x -= x_rel_coords.max / 2
                        when 180
                            x += x_rel_coords.max / 2
                        when 270
                            x += x_rel_coords.max / 2
                            y += y_rel_coords.shift
                            y_rel_coords << 0   # since we used an element we need to add a dummy
                    end

                    x -= shift_baseline(@ctx.text_attrs.glyph_orientation_vertical, text[0,1])

                    first_word = true
                    text.split(::Magick::RVG::WORD_SEP).each do |word|
                        unless first_word
                            y += y_rel_coords.shift
                            x_rel_coords.shift
                        end
                        first_word = false
                        word.split('').each do |glyph|
                            case @ctx.text_attrs.glyph_orientation_vertical.to_i
                                when 0, 90, 270
                                    x_shift = (dx - x_rel_coords.shift) / 2
                                when 180
                                    x_shift = -(dx - x_rel_coords.shift) / 2
                            end

                            render_glyph(@ctx.text_attrs.glyph_orientation_vertical, x+x_shift, y, glyph)
                            y += y_rel_coords.shift
                        end
                    end

                    @ctx.gc.pop()
                    [0, dy]
                end

            end     # class TBTextStrategy

            # Handle "easy" text
            class DefaultTextStrategy < TextStrategy

                def render(x, y, text)
                    @ctx.gc.text(x, y, enquote(text))
                    tm = @ctx.shadow.get_type_metrics(text)
                    dx = case @ctx.text_attrs.text_anchor
                            when :start
                                 tm.width
                            when :middle
                                 tm.width / 2
                            when :end
                                 0
                          end
                    [dx, 0]
                end

            end     # class NormalTextStrategy

        end # class Utility
    end # class RVG
end # module Magick




module Magick
    class RVG
        class Utility

            class TextAttributes

              public

                WRITING_MODE = %w{lr-tb lr rl-tb rl tb-rl tb}

                def initialize()
                    @affine = Array.new
                    @affine << Magick::AffineMatrix.new(1, 0, 0, 1, 0, 0)
                    @baseline_shift = Array.new
                    @baseline_shift << :baseline
                    @glyph_orientation_horizontal = Array.new
                    @glyph_orientation_horizontal << 0
                    @glyph_orientation_vertical = Array.new
                    @glyph_orientation_vertical << 90
                    @letter_spacing = Array.new
                    @letter_spacing << 0
                    @text_anchor = Array.new
                    @text_anchor << :start
                    @word_spacing = Array.new
                    @word_spacing << 0
                    @writing_mode = Array.new
                    @writing_mode << 'lr-tb'
                end

                def push()
                    @affine.push(@affine.last.dup)
                    @baseline_shift.push(@baseline_shift.last)
                    @text_anchor.push(@text_anchor.last)
                    @writing_mode.push(@writing_mode.last.dup)
                    @glyph_orientation_vertical.push(@glyph_orientation_vertical.last)
                    @glyph_orientation_horizontal.push(@glyph_orientation_horizontal.last)
                    @letter_spacing.push(@letter_spacing.last)
                    @word_spacing.push(@word_spacing.last)
                end

                def pop()
                    @affine.pop
                    @baseline_shift.pop
                    @text_anchor.pop
                    @writing_mode.pop
                    @glyph_orientation_vertical.pop
                    @glyph_orientation_horizontal.pop
                    @letter_spacing.pop
                    @word_spacing.pop
                end

                def set_affine(sx, rx, ry, sy, tx, ty)
                    @affine[-1].sx = sx
                    @affine[-1].rx = rx
                    @affine[-1].ry = ry
                    @affine[-1].sy = sy
                    @affine[-1].tx = tx
                    @affine[-1].ty = ty
                end

                def affine()
                    @affine[-1]
                end

                def baseline_shift()
                    @baseline_shift[-1]
                end

                def baseline_shift=(value)
                    @baseline_shift[-1] = value
                end

                def text_anchor()
                    @text_anchor[-1]
                end

                def text_anchor=(anchor)
                    @text_anchor[-1] = anchor
                end

                def glyph_orientation_vertical()
                    @glyph_orientation_vertical[-1]
                end

                def glyph_orientation_vertical=(angle)
                    @glyph_orientation_vertical[-1] = angle
                end

                def glyph_orientation_horizontal()
                    @glyph_orientation_horizontal[-1]
                end

                def glyph_orientation_horizontal=(angle)
                    @glyph_orientation_horizontal[-1] = angle
                end

                def letter_spacing()
                    @letter_spacing[-1]
                end

                def letter_spacing=(value)
                    @letter_spacing[-1] = value
                end

                def non_default?
                    @baseline_shift[-1] != :baseline || @letter_spacing[-1] != 0 ||
                    @word_spacing[-1] != 0 || @writing_mode[-1][/\Alr/].nil? ||
                    @glyph_orientation_horizontal[-1] != 0
                end

                def word_spacing()
                    @word_spacing[-1]
                end

                def word_spacing=(value)
                    @word_spacing[-1] = value
                end

                def writing_mode()
                    @writing_mode[-1]
                end

                def writing_mode=(mode)
                    @writing_mode[-1] = WRITING_MODE.include?(mode) ? mode : 'lr-tb'
                end

            end     # class TextAttributes

            class GraphicContext

                FONT_STRETCH =    {:normal          => Magick::NormalStretch,
                                   :ultra_condensed => Magick::UltraCondensedStretch,
                                   :extra_condensed => Magick::ExtraCondensedStretch,
                                   :condensed       => Magick::CondensedStretch,
                                   :semi_condensed  => Magick::SemiCondensedStretch,
                                   :semi_expanded   => Magick::SemiExpandedStretch,
                                   :expanded        => Magick::ExpandedStretch,
                                   :extra_expanded  => Magick::ExtraExpandedStretch,
                                   :ultra_expanded  => Magick::UltraExpandedStretch}

                FONT_STYLE =      {:normal  => Magick::NormalStyle,
                                   :italic  => Magick::ItalicStyle,
                                   :oblique => Magick::ObliqueStyle}

                FONT_WEIGHT =     {'normal'  => Magick::NormalWeight,
                                   'bold'    => Magick::BoldWeight,
                                   'bolder'  => Magick::BolderWeight,
                                   'lighter' => Magick::LighterWeight}

                TEXT_ANCHOR =     {:start  => Magick::StartAnchor,
                                   :middle => Magick::MiddleAnchor,
                                   :end    => Magick::EndAnchor}

                ANCHOR_TO_ALIGN = {:start  => Magick::LeftAlign,
                                   :middle => Magick::CenterAlign,
                                   :end    => Magick::RightAlign}

                TEXT_DECORATION = {:none         => Magick::NoDecoration,
                                   :underline    => Magick::UnderlineDecoration,
                                   :overline     => Magick::OverlineDecoration,
                                   :line_through => Magick::LineThroughDecoration}

                TEXT_STRATEGIES  = {'lr-tb'=>LRTextStrategy, 'lr'=>LRTextStrategy,
                                    'rt-tb'=>RLTextStrategy, 'rl'=>RLTextStrategy,
                                    'tb-rl'=>TBTextStrategy, 'tb'=>TBTextStrategy}

                def GraphicContext.degrees_to_radians(deg)
                    Math::PI * (deg % 360.0) / 180.0
                end

              private

                def init_matrix()
                    @rx = @ry = 0
                    @sx = @sy = 1
                    @tx = @ty = 0
                end

                def concat_matrix()
                    curr = @text_attrs.affine
                    sx = curr.sx * @sx + curr.ry * @rx
                    rx = curr.rx * @sx + curr.sy * @rx
                    ry = curr.sx * @ry + curr.ry * @sy
                    sy = curr.rx * @ry + curr.sy * @sy
                    tx = curr.sx * @tx + curr.ry * @ty + curr.tx
                    ty = curr.rx * @tx + curr.sy * @ty + curr.ty
                    @text_attrs.set_affine(sx, rx, ry, sy, tx, ty)
                    init_matrix()
                end

              public

                attr_reader :gc, :text_attrs

                def initialize()
                    @gc = Magick::Draw.new
                    @shadow = Array.new
                    @shadow << Magick::Draw.new
                    @text_attrs = TextAttributes.new
                    init_matrix()
                end

                def method_missing(methID, *args, &block)
                    @gc.__send__(methID, *args, &block)
                end

                def affine(sx, rx, ry, sy, tx, ty)
                    sx, rx, ry, sy, tx, ty = Magick::RVG.convert_to_float(sx, rx, ry, sy, tx, ty)
                    @gc.affine(sx, rx, ry, sy, tx, ty)
                    @text_attrs.set_affine(sx, rx, ry, sy, tx, ty)
                    nil
                end

                def baseline_shift(value)
                    @text_attrs.baseline_shift = case value
                        when 'baseline', 'sub', 'super'
                            value.intern
                        when /[-+]?\d+%/, Numeric
                            value
                        else
                            :baseline
                        end
                    nil
                end

                def font(name)
                    @gc.font(name)
                    @shadow[-1].font = name
                    nil
                end

                def font_family(name)
                    @gc.font_family(name)
                    @shadow[-1].font_family = name
                    nil
                end

                def font_size(points)
                    @gc.font_size(points)
                    @shadow[-1].pointsize = points
                    nil
                end

                def font_stretch(stretch)
                    stretch = FONT_STRETCH.fetch(stretch.intern, Magick::NormalStretch)
                    @gc.font_stretch(stretch)
                    @shadow[-1].font_stretch = stretch
                    nil
                end

                def font_style(style)
                    style = FONT_STYLE.fetch(style.intern, Magick::NormalStyle)
                    @gc.font_style(style)
                    @shadow[-1].font_style = style
                    nil
                end

                def font_weight(weight)
                    # If the arg is not in the hash use it directly. Handles numeric values.
                    weight = FONT_WEIGHT.fetch(weight) {|key| key}
                    @gc.font_weight(weight)
                    @shadow[-1].font_weight = weight
                    nil
                end

                def glyph_orientation_horizontal(deg)
                    deg = Magick::RVG.convert_one_to_float(deg)
                    @text_attrs.glyph_orientation_horizontal = (deg % 360) / 90 * 90
                    nil
                end

                def glyph_orientation_vertical(deg)
                    deg = Magick::RVG.convert_one_to_float(deg)
                    @text_attrs.glyph_orientation_vertical = (deg % 360) / 90 * 90
                    nil
                end

                def inspect()
                    @gc.inspect
                end

                def letter_spacing(value)
                    @text_attrs.letter_spacing = Magick::RVG.convert_one_to_float(value)
                    nil
                end

                def push()
                    @gc.push
                    @shadow.push(@shadow.last.dup)
                    @text_attrs.push
                    nil
                end

                def pop()
                    @gc.pop
                    @shadow.pop
                    @text_attrs.pop
                    nil
                end

                def rotate(degrees)
                    degrees = Magick::RVG.convert_one_to_float(degrees)
                    @gc.rotate(degrees)
                    @sx =  Math.cos(GraphicContext.degrees_to_radians(degrees))
                    @rx =  Math.sin(GraphicContext.degrees_to_radians(degrees))
                    @ry = -Math.sin(GraphicContext.degrees_to_radians(degrees))
                    @sy =  Math.cos(GraphicContext.degrees_to_radians(degrees))
                    concat_matrix()
                    nil
                end

                def scale(sx, sy)
                    sx, sy = Magick::RVG.convert_to_float(sx, sy)
                    @gc.scale(sx, sy)
                    @sx, @sy = sx, sy
                    concat_matrix()
                    nil
                end

                def shadow()
                    @shadow.last
                end

                def skewX(degrees)
                    degrees = Magick::RVG.convert_one_to_float(degrees)
                    @gc.skewX(degrees)
                    @ry = Math.tan(GraphicContext.degrees_to_radians(degrees))
                    concat_matrix()
                    nil
                end

                def skewY(degrees)
                    degrees = Magick::RVG.convert_one_to_float(degrees)
                    @gc.skewY(degrees)
                    @rx = Math.tan(GraphicContext.degrees_to_radians(degrees))
                    concat_matrix()
                    nil
                end

                def stroke_width(width)
                    width = Magick::RVG.convert_one_to_float(width)
                    @gc.stroke_width(width)
                    @shadow[-1].stroke_width = width
                    nil
                end

                def text(x, y, text)
                    return if text.length == 0
                    if @text_attrs.non_default?
                        text_renderer = TEXT_STRATEGIES[@text_attrs.writing_mode].new(self)
                    else
                        text_renderer = DefaultTextStrategy.new(self)
                    end

                    return text_renderer.render(x, y, text)
                end

                def text_anchor(anchor)
                    anchor = anchor.intern
                    anchor_enum = TEXT_ANCHOR.fetch(anchor, Magick::StartAnchor)
                    @gc.text_anchor(anchor_enum)
                    align = ANCHOR_TO_ALIGN.fetch(anchor, Magick::LeftAlign)
                    @shadow[-1].align = align
                    @text_attrs.text_anchor = anchor
                    nil
                end

                def text_decoration(decoration)
                    decoration = TEXT_DECORATION.fetch(decoration.intern, Magick::NoDecoration)
                    @gc.decorate(decoration)
                    @shadow[-1].decorate = decoration
                    nil
                end

                def translate(tx, ty)
                    tx, ty = Magick::RVG.convert_to_float(tx, ty)
                    @gc.translate(tx, ty)
                    @tx, @ty = tx, ty
                    concat_matrix()
                    nil
                end

                def word_spacing(value)
                    @text_attrs.word_spacing = Magick::RVG.convert_one_to_float(value)
                    nil
                end

                def writing_mode(mode)
                    @text_attrs.writing_mode = mode
                    nil
                end

            end     # class GraphicContext

        end # class Utility
    end # class RVG
end # module Magick

