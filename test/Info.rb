
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner' if RUBY_VERSION != '1.9.1'


class Info_UT < Test::Unit::TestCase

    def setup
        @info = Magick::Image::Info.new
    end

    def test_options

        # 1-argument form
        assert_nothing_raised { @info['fill'] }
        assert_nil(@info['fill'])

        assert_nothing_raised { @info['fill'] = 'red' }
        assert_equal("red", @info['fill'])

        assert_nothing_raised { @info['fill'] = nil }
        assert_nil(@info['fill'])

        # 2-argument form
        assert_nothing_raised { @info['tiff', 'bits-per-sample'] = 2 }
        assert_equal("2", @info['tiff', 'bits-per-sample'])

        # define and undefine
        assert_nothing_raised { @info.define('tiff', 'bits-per-sample', 4) }
        assert_equal("4", @info['tiff', 'bits-per-sample'])

        assert_nothing_raised { @info.undefine('tiff', 'bits-per-sample') }
        assert_nil(@info['tiff', 'bits-per-sample'])
    end

    def test_antialias
      assert @info.antialias
      assert_nothing_raised { @info.antialias = false }
      assert !@info.antialias
    end

    def test_attenuate
        assert_nothing_raised { @info.attenuate = 10 }
        assert_equal(10, @info.attenuate)
        assert_nothing_raised { @info.attenuate = 5.25 }
        assert_equal(5.25, @info.attenuate)
        assert_nothing_raised { @info.attenuate = nil }
        assert_equal(nil, @info.attenuate)
    end

    def test_authenticate
      assert_nothing_raised { @info.authenticate = "string" }
      assert_equal("string", @info.authenticate)
    end

    def test_background_color
      assert_nothing_raised { @info.background_color = "red" }
      red = Magick::Pixel.new(Magick::MaxRGB)
      assert_nothing_raised { @info.background_color = red }
      assert_equal("red", @info.background_color)
      img = Magick::Image.new(20,20) { self.background_color = "red" }
      assert_equal("red", img.background_color)
    end

    def test_border_color
      assert_nothing_raised { @info.border_color = "red" }
      red = Magick::Pixel.new(Magick::MaxRGB)
      assert_nothing_raised { @info.border_color = red }
      assert_equal("red", @info.border_color)
      img = Magick::Image.new(20,20) { self.border_color = "red" }
      assert_equal("red", img.border_color)
    end

    def caption
      assert_nothing_raised { @info.caption = "string" }
      assert_equal("string", @info.caption)
      img = Magick::Image.new(20,20) { self.caption = "string" }
      assert_equal("string", img.caption)
    end

    def test_channel
        assert_nothing_raised { @info.channel(Magick::RedChannel) }
        assert_nothing_raised { @info.channel(Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @info.channel(1) }
        assert_raise(TypeError) { @info.channel(Magick::RedChannel, 1) }
    end

    def test_colorspace
      Magick::ColorspaceType.values.each do |cs|
        assert_nothing_raised {@info.colorspace = cs}
        assert_equal(cs, @info.colorspace)
      end
    end

    def test_comment
      assert_nothing_raised { @info.comment = "comment" }
      assert_equal("comment", @info.comment)
    end

    def test_compression
      Magick::CompressionType.values.each do |v|
        assert_nothing_raised { @info.compression = v }
        assert_equal(v, @info.compression)
      end
    end

    def test_define
      assert_nothing_raised { @info.define("tiff", "bits-per-sample", 2) }
      assert_nothing_raised { @info.undefine("tiff", "bits-per-sample") }
    end

    def test_density
      assert_nothing_raised { @info.density = "72x72" }
      assert_equal("72x72", @info.density)
      assert_nothing_raised { @info.density = Magick::Geometry.new(72, 72) }
      assert_equal("72x72", @info.density)
    end

    def test_delay
      assert_nothing_raised { @info.delay = 60 }
      assert_equal(60, @info.delay)
    end

    def test_depth
      assert_nothing_raised { @info.depth = 8 }
      assert_equal(8, @info.depth)
      assert_nothing_raised { @info.depth = 16 }
      assert_equal(16, @info.depth)
      assert_raise(ArgumentError) { @info.depth = 32 }
    end

    def test_dispose
      Magick::DisposeType.values.each do |v|
        assert_nothing_raised { @info.dispose = v }
        assert_equal(v, @info.dispose)
      end
    end

    def test_dither
      assert_nothing_raised { @info.dither = true }
      assert_equal(true, @info.dither)
      assert_nothing_raised { @info.dither = false }
      assert_equal(false, @info.dither)
    end

    def test_extract
        assert_nothing_raised { @info.extract = "100x100" }
        assert_equal("100x100", @info.extract)
        assert_nothing_raised { @info.extract = Magick::Geometry.new(100, 100) }
        assert_equal("100x100", @info.extract)
    end

    def test_filename
      assert_nothing_raised { @info.filename = "string" }
      assert_equal("string", @info.filename)
    end

    def test_fill
        assert_nothing_raised { @info.fill }
        assert_nil(@info.fill)

        assert_nothing_raised { @info.fill = 'white' }
        assert_equal("white", @info.fill)

        assert_nothing_raised { @info.fill = nil }
        assert_nil(@info.fill)

        assert_raise(ArgumentError) { @info.fill = 'xxx' }
    end

    def test_font
      assert_nothing_raised { @info.font = "Arial" }
      assert_equal("Arial", @info.font)
    end

    def test_format
      assert_nothing_raised { @info.format = "GIF" }
      assert_equal("GIF", @info.format)
    end

    def test_fuzz
      assert_nothing_raised { @info.fuzz = 50 }
      assert_equal(50, @info.fuzz)
      assert_nothing_raised { @info.fuzz = "50%" }
      assert_equal(Magick::MaxRGB * 0.5, @info.fuzz)
    end

    def test_gravity
      Magick::GravityType.values.each do |v|
        assert_nothing_raised { @info.gravity = v }
        assert_equal(v, @info.gravity)
      end
    end

    def test_image_type
      Magick::ImageType.values.each do |v|
        assert_nothing_raised { @info.image_type = v }
        assert_equal(v, @info.image_type)
      end
    end

    def test_interlace
      Magick::InterlaceType.values.each do |v|
        assert_nothing_raised { @info.interlace = v }
        assert_equal(v, @info.interlace)
      end
    end

    def test_label
      assert_nothing_raised { @info.label = "string" }
      assert_equal("string", @info.label)
    end

    def test_matte_color
      assert_nothing_raised { @info.matte_color = "red" }
      red = Magick::Pixel.new(Magick::MaxRGB)
      assert_nothing_raised { @info.matte_color = red }
      assert_equal("red", @info.matte_color)
      img = Magick::Image.new(20,20) { self.matte_color = "red" }
      assert_equal("red", img.matte_color)
    end

    def test_monitor
      assert_nothing_raised { @info.monitor = lambda {} }
      monitor = Proc.new do |mth, q, s|
        assert_equal("resize!", mth)
        assert_instance_of(Fixnum, q)
        assert_instance_of(Fixnum, s)
        true
      end
      img = Magick::Image.new(2000, 2000) { self.monitor = monitor }
      img.resize!(20, 20)
      img.monitor = nil
    end

    def test_monochrome
      assert_nothing_raised { @info.monochrome = true }
      assert @info.monochrome
    end

    def test_orientation
      Magick::OrientationType.values.each do |v|
        assert_nothing_raised { @info.orientation = v }
        assert_equal(v, @info.orientation)
      end
    end

    def test_origin
      assert_nothing_raised { @info.origin = "+10+10" }
      assert_equal("+10+10", @info.origin)
      assert_nothing_raised { @info.origin = Magick::Geometry.new(nil, nil, 10, 10) }
      assert_equal("+10+10", @info.origin)
    end

    def test_page
      assert_nothing_raised { @info.page = "612x792>" }
      assert_equal("612x792>", @info.page)
    end

    def test_pointsize
      assert_nothing_raised { @info.pointsize = 12 }
      assert_equal(12, @info.pointsize)
    end

    def test_quality
      assert_nothing_raised { @info.quality = 75 }
      assert_equal(75, @info.quality)
    end

    def test_sampling_factor
      assert_nothing_raised { @info.sampling_factor = "2x1" }
      assert_equal("2x1", @info.sampling_factor)
    end

    def test_server_name
      assert_nothing_raised { @info.server_name = "foo" }
      assert_equal("foo", @info.server_name)
    end

    def test_size
      assert_nothing_raised { @info.size = "200x100" }
      assert_equal("200x100", @info.size)
      assert_nothing_raised { @info.size = Magick::Geometry.new(100,200) }
      assert_equal("100x200", @info.size)
    end

    def test_stroke
        assert_nothing_raised { @info.stroke }
        assert_nil(@info.stroke)

        assert_nothing_raised { @info.stroke = 'white' }
        assert_equal("white", @info.stroke)

        assert_nothing_raised { @info.stroke = nil }
        assert_nil(@info.stroke)

        assert_raise(ArgumentError) { @info.stroke = 'xxx' }
    end

    def test_stroke_width
        assert_nothing_raised { @info.stroke_width = 10 }
        assert_equal(10, @info.stroke_width)
        assert_nothing_raised { @info.stroke_width = 5.25 }
        assert_equal(5.25, @info.stroke_width)
        assert_nothing_raised { @info.stroke_width = nil }
        assert_equal(nil, @info.stroke_width)
    end

    def test_texture
      img = Magick::Image.read("granite:") {self.size = "20x20" }
      assert_nothing_raised { @info.texture = img.first }
    end

    def test_tile_offset
      assert_nothing_raised { @info.tile_offset = "200x100" }
      assert_equal("200x100", @info.tile_offset)
      assert_nothing_raised { @info.tile_offset = Magick::Geometry.new(100,200) }
      assert_equal("100x200", @info.tile_offset)
    end

    def test_undercolor
      assert_nothing_raised { @info.undercolor }
      assert_nil(@info.undercolor)

      assert_nothing_raised { @info.undercolor = 'white' }
      assert_equal("white", @info.undercolor)

      assert_nothing_raised { @info.undercolor = nil }
      assert_nil(@info.undercolor)

      assert_raise(ArgumentError) { @info.undercolor = 'xxx' }
    end

    def test_units
      Magick::ResolutionType.values.each do |v|
        assert_nothing_raised { @info.units = v }
        assert_equal(v, @info.units)
      end
    end

    def test_view
      assert_nothing_raised { @info.view = "string" }
      assert_equal("string", @info.view)
    end

end
