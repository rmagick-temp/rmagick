#! /usr/local/bin/ruby -w

require 'fileutils'
require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'  if RUBY_VERSION != '1.9.1'

# TODO
#   test frozen attributes!
#   improve test_directory
#   improve test_montage


class Image_Attributes_UT < Test::Unit::TestCase
    FreezeError = RUBY_VERSION == '1.9.1' ? RuntimeError : TypeError

    def setup
        @img = Magick::Image.new(100, 100)
        gc = Magick::Draw.new

        gc.stroke_width(5)
        gc.circle(50, 50, 80, 80)
        gc.draw(@img)

        @hat = Magick::Image.read(FLOWER_HAT).first
    end

    # Test old alpha attribute. New alpha() behavior is tested in Image1.rb
    def test_alpha
      assert(@img.alpha)
      assert_nothing_raised { @img.alpha = Magick::DeactivateAlphaChannel }
      assert(!@img.alpha)
    end

    def test_background_color
        assert_nothing_raised { @img.background_color }
        assert_equal("white", @img.background_color)
        assert_nothing_raised { @img.background_color = "#dfdfdf" }
        #assert_equal("rgb(223,223,223)", @img.background_color)
        assert_equal("#DFDFDFDFDFDF", @img.background_color)
        assert_nothing_raised { @img.background_color = Magick::Pixel.new(Magick::QuantumRange, Magick::QuantumRange/2.0, Magick::QuantumRange/2.0) }
        #assert_equal("rgb(100%,49.9992%,49.9992%)", @img.background_color)
        assert_equal("#FFFF7FFF7FFF", @img.background_color)
        assert_raise(TypeError) { @img.background_color = 2 }
    end

    def test_base_columns
        assert_nothing_raised { @img.base_columns }
        assert_equal(0, @img.base_columns)
        assert_raise(NoMethodError) { @img.base_columns = 1 }
    end

    def test_base_filename
        assert_nothing_raised { @img.base_filename }
        assert_equal("", @img.base_filename)
        assert_raise(NoMethodError) { @img.base_filename = "xxx" }
    end

    def test_base_rows
        assert_nothing_raised { @img.base_rows }
        assert_equal(0, @img.base_rows)
        assert_raise(NoMethodError) { @img.base_rows = 1 }
    end

    def test_bias
        assert_nothing_raised { @img.bias }
        assert_equal(0.0, @img.bias)
        assert_instance_of(Float, @img.bias)

        assert_nothing_raised { @img.bias = 0.1 }
        assert_in_delta(Magick::QuantumRange * 0.1, @img.bias, 0.1)

        assert_nothing_raised { @img.bias = '10%' }
        assert_in_delta(Magick::QuantumRange * 0.10, @img.bias, 0.1)

        assert_raise(TypeError) { @img.bias = [] }
        assert_raise(ArgumentError) { @img.bias = 'x' }
    end

    def test_black_point_compensation
        assert_nothing_raised { @img.black_point_compensation = true }
        assert(@img.black_point_compensation)
        assert_nothing_raised { @img.black_point_compensation = false }
        assert_equal(false, @img.black_point_compensation)
    end

    def test_blur
        assert_nothing_raised { @img.blur }
        assert_equal(1.0, @img.blur)
        assert_nothing_raised { @img.blur = 2.0 }
        assert_raise(TypeError) { @img.blur = 'x' }
    end

    def test_border_color
        assert_nothing_raised { @img.border_color }
        #assert_equal("rgb(223,223,223)", @img.border_color)
        assert_equal("#DFDFDFDFDFDF", @img.border_color)
        assert_nothing_raised { @img.border_color = "red" }
        assert_equal("red", @img.border_color)
        assert_nothing_raised { @img.border_color = Magick::Pixel.new(Magick::QuantumRange, Magick::QuantumRange/2, Magick::QuantumRange/2) }
        #assert_equal("rgb(100%,49.9992%,49.9992%)", @img.border_color)
        assert_equal("#FFFF7FFF7FFF", @img.border_color)
        assert_raise(TypeError) { @img.border_color = 2 }
    end

    def test_bounding_box
        assert_nothing_raised { @img.bounding_box }
        box = @img.bounding_box
        assert_equal(87, box.width)
        assert_equal(87, box.height)
        assert_equal(7, box.x)
        assert_equal(7, box.y)
        assert_raise(NoMethodError) { @img.bounding_box = 2 }
    end

    def test_chromaticity
        chrom = @img.chromaticity
        assert_nothing_raised { @img.chromaticity }
        assert_instance_of(Magick::Chromaticity, chrom)
        assert_equal(0, chrom.red_primary.x)
        assert_equal(0, chrom.red_primary.y)
        assert_equal(0, chrom.red_primary.z)
        assert_equal(0, chrom.green_primary.x)
        assert_equal(0, chrom.green_primary.y)
        assert_equal(0, chrom.green_primary.z)
        assert_equal(0, chrom.blue_primary.x)
        assert_equal(0, chrom.blue_primary.y)
        assert_equal(0, chrom.blue_primary.z)
        assert_equal(0, chrom.white_point.x)
        assert_equal(0, chrom.white_point.y)
        assert_equal(0, chrom.white_point.z)
        assert_nothing_raised { @img.chromaticity = chrom }
        assert_raise(TypeError) { @img.chromaticity = 2 }
    end

    def test_class_type
        assert_nothing_raised { @img.class_type }
        assert_instance_of(Magick::ClassType, @img.class_type)
        assert_equal(Magick::DirectClass, @img.class_type)
        assert_nothing_raised { @img.class_type = Magick::PseudoClass }
        assert_equal(Magick::PseudoClass, @img.class_type)
        assert_raise(TypeError) { @img.class_type = 2 }
    end

    def test_color_profile
        assert_nothing_raised { @img.color_profile }
        assert_nil(@img.color_profile)
        assert_nothing_raised { @img.color_profile = 'xxx' }
        assert_equal('xxx', @img.color_profile)
        assert_raise(TypeError) { @img.color_profile = 2 }
    end

    def test_colors
        assert_nothing_raised { @img.colors }
        assert_equal(0, @img.colors)
        img = @img.copy
        img.class_type = Magick::PseudoClass
        assert_equal(40, img.colors)
        assert_raise(NoMethodError) { img.colors = 2 }
    end

    def test_colorspace
        assert_nothing_raised { @img.colorspace }
        assert_instance_of(Magick::ColorspaceType, @img.colorspace)
        assert_equal(Magick::RGBColorspace, @img.colorspace)
        img = @img.copy
        assert_nothing_raised { img.colorspace = Magick::GRAYColorspace }
        assert_equal(Magick::GRAYColorspace, img.colorspace)
        assert_raise(TypeError) { @img.colorspace = 2 }
        Magick::ColorspaceType.values.each do |cs|
          assert_nothing_raised { img.colorspace = cs }
        end
    end

    def test_columns
        assert_nothing_raised { @img.columns }
        assert_equal(100, @img.columns)
        assert_raise(NoMethodError) { @img.columns = 2 }
    end

    def test_compose
        assert_nothing_raised { @img.compose }
        assert_instance_of(Magick::CompositeOperator, @img.compose)
        assert_equal(Magick::OverCompositeOp, @img.compose)
        assert_raise(TypeError) { @img.compose = 2 }
        assert_nothing_raised { @img.compose = Magick::UndefinedCompositeOp }
        assert_equal(Magick::UndefinedCompositeOp, @img.compose)

        assert_nothing_raised { @img.compose = Magick::NoCompositeOp }
        assert_nothing_raised { @img.compose = Magick::AddCompositeOp }
        assert_nothing_raised { @img.compose = Magick::AtopCompositeOp }
        assert_nothing_raised { @img.compose = Magick::BlendCompositeOp }
        assert_nothing_raised { @img.compose = Magick::BumpmapCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ClearCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ColorBurnCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ColorDodgeCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ColorizeCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyBlackCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyBlueCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyCyanCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyGreenCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyMagentaCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyOpacityCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyRedCompositeOp }
        assert_nothing_raised { @img.compose = Magick::CopyYellowCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DarkenCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DstAtopCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DstCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DstInCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DstOutCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DstOverCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DifferenceCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DisplaceCompositeOp }
        assert_nothing_raised { @img.compose = Magick::DissolveCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ExclusionCompositeOp }
        assert_nothing_raised { @img.compose = Magick::HardLightCompositeOp }
        assert_nothing_raised { @img.compose = Magick::HueCompositeOp }
        assert_nothing_raised { @img.compose = Magick::InCompositeOp }
        assert_nothing_raised { @img.compose = Magick::LightenCompositeOp }
        assert_nothing_raised { @img.compose = Magick::LuminizeCompositeOp }
        assert_nothing_raised { @img.compose = Magick::MinusCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ModulateCompositeOp }
        assert_nothing_raised { @img.compose = Magick::MultiplyCompositeOp }
        assert_nothing_raised { @img.compose = Magick::OutCompositeOp }
        assert_nothing_raised { @img.compose = Magick::OverCompositeOp }
        assert_nothing_raised { @img.compose = Magick::OverlayCompositeOp }
        assert_nothing_raised { @img.compose = Magick::PlusCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ReplaceCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SaturateCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ScreenCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SoftLightCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SrcAtopCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SrcCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SrcInCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SrcOutCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SrcOverCompositeOp }
        assert_nothing_raised { @img.compose = Magick::SubtractCompositeOp }
        assert_nothing_raised { @img.compose = Magick::ThresholdCompositeOp }
        assert_nothing_raised { @img.compose = Magick::XorCompositeOp }
        assert_raise(TypeError) { @img.compose = 2 }
    end

    def test_compression
        assert_nothing_raised { @img.compression }
        assert_instance_of(Magick::CompressionType, @img.compression)
        assert_equal(Magick::UndefinedCompression, @img.compression)
        assert_nothing_raised { @img.compression = Magick::BZipCompression }
        assert_equal(Magick::BZipCompression, @img.compression)
        assert_nothing_raised { @img.compression = Magick::NoCompression }
        assert_nothing_raised { @img.compression = Magick::BZipCompression }
        assert_nothing_raised { @img.compression = Magick::B44Compression }
        assert_nothing_raised { @img.compression = Magick::B44ACompression }
        assert_nothing_raised { @img.compression = Magick::DXT1Compression }
        assert_nothing_raised { @img.compression = Magick::DXT3Compression }
        assert_nothing_raised { @img.compression = Magick::DXT5Compression }
        assert_nothing_raised { @img.compression = Magick::FaxCompression }
        assert_nothing_raised { @img.compression = Magick::Group4Compression }
        assert_nothing_raised { @img.compression = Magick::JPEGCompression }
        assert_nothing_raised { @img.compression = Magick::JPEG2000Compression }
        assert_nothing_raised { @img.compression = Magick::LosslessJPEGCompression }
        assert_nothing_raised { @img.compression = Magick::LZWCompression }
        assert_nothing_raised { @img.compression = Magick::PizCompression }
        assert_nothing_raised { @img.compression = Magick::Pxr24Compression }
        assert_nothing_raised { @img.compression = Magick::RLECompression }
        assert_nothing_raised { @img.compression = Magick::ZipCompression }
        assert_nothing_raised { @img.compression = Magick::ZipSCompression }
        assert_raise(TypeError) { @img.compression = 2 }
    end

    def test_delay
        assert_nothing_raised { @img.delay }
        assert_equal(0, @img.delay)
        assert_nothing_raised { @img.delay = 10 }
        assert_equal(10, @img.delay)
        assert_raise(TypeError) { @img.delay = 'x' }
    end

    def test_density
        assert_nothing_raised { @img.density }
        assert_equal('72x72', @img.density)
        assert_nothing_raised { @img.density = '90x90' }
        assert_nothing_raised { @img.density = 'x90' }
        assert_nothing_raised { @img.density = '90' }
        assert_raise(TypeError) { @img.density = 2 }
    end

    def test_depth
        assert_equal(Magick::QuantumDepth, @img.depth)
        assert_raise(NoMethodError) { @img.depth = 2 }
    end

    def test_directory
        assert_nothing_raised { @img.directory }
        assert_nil(@img.directory)
        assert_raise(NoMethodError) { @img.directory = nil }
    end

    def test_dispose
        assert_nothing_raised { @img.dispose }
        assert_instance_of(Magick::DisposeType, @img.dispose)
        assert_equal(Magick::UndefinedDispose, @img.dispose)
        assert_nothing_raised { @img.dispose = Magick::NoneDispose }
        assert_equal(Magick::NoneDispose, @img.dispose)
        assert_nothing_raised { @img.dispose = Magick::BackgroundDispose }
        assert_nothing_raised { @img.dispose = Magick::PreviousDispose }
        assert_raise(TypeError) { @img.dispose = 2 }
    end

    def test_endian
        assert_nothing_raised { @img.endian }
        assert_instance_of(Magick::EndianType, @img.endian)
        assert_equal(Magick::UndefinedEndian, @img.endian)
        assert_nothing_raised { @img.endian = Magick::LSBEndian }
        assert_equal(Magick::LSBEndian, @img.endian)
        assert_nothing_raised { @img.endian = Magick::MSBEndian }
        assert_raise(TypeError) { @img.endian = 2 }
    end

    def test_extract_info
        assert_nothing_raised { @img.extract_info }
        assert_instance_of(Magick::Rectangle, @img.extract_info)
        ext = @img.extract_info
        assert_equal(0, ext.x)
        assert_equal(0, ext.y)
        assert_equal(0, ext.width)
        assert_equal(0, ext.height)
        ext = Magick::Rectangle.new(1, 2, 3, 4)
        assert_nothing_raised { @img.extract_info = ext }
        assert_equal(1, ext.width)
        assert_equal(2, ext.height)
        assert_equal(3, ext.x)
        assert_equal(4, ext.y)
        assert_raise(TypeError) { @img.extract_info = 2 }
    end

    def test_filename
        assert_nothing_raised { @img.filename }
        assert_equal("", @img.filename)
        assert_raises(NoMethodError) { @img.filename = 'xxx' }
    end

    def test_filter
        assert_nothing_raised { @img.filter }
        assert_instance_of(Magick::FilterTypes, @img.filter)
        assert_equal(Magick::UndefinedFilter, @img.filter)
        assert_nothing_raised { @img.filter = Magick::PointFilter }
        assert_equal(Magick::PointFilter, @img.filter)
        assert_nothing_raised { @img.filter = Magick::BoxFilter }
        assert_nothing_raised { @img.filter = Magick::TriangleFilter }
        assert_nothing_raised { @img.filter = Magick::HermiteFilter }
        assert_nothing_raised { @img.filter = Magick::HanningFilter }
        assert_nothing_raised { @img.filter = Magick::HammingFilter }
        assert_nothing_raised { @img.filter = Magick::BlackmanFilter }
        assert_nothing_raised { @img.filter = Magick::GaussianFilter }
        assert_nothing_raised { @img.filter = Magick::QuadraticFilter }
        assert_nothing_raised { @img.filter = Magick::CubicFilter }
        assert_nothing_raised { @img.filter = Magick::CatromFilter }
        assert_nothing_raised { @img.filter = Magick::MitchellFilter }
        assert_nothing_raised { @img.filter = Magick::LanczosFilter }
        assert_nothing_raised { @img.filter = Magick::BesselFilter }
        assert_nothing_raised { @img.filter = Magick::SincFilter }
        assert_raise(TypeError) { @img.filter = 2 }
    end

    def test_format
        assert_nothing_raised { @img.format }
        assert_nil(@img.format)
        assert_nothing_raised { @img.format = 'GIF' }
        assert_nothing_raised { @img.format = 'JPG' }
        assert_nothing_raised { @img.format = 'TIFF' }
        assert_nothing_raised { @img.format = 'MIFF' }
        assert_nothing_raised { @img.format = 'MPEG' }
        v = $VERBOSE
        $VERBOSE = nil
        assert_raise(ArgumentError) { @img.format = 'shit' }
        $VERBOSE = v
        assert_raise(TypeError) { @img.format = 2 }
    end

    def test_fuzz
        assert_nothing_raised { @img.fuzz }
        assert_instance_of(Float, @img.fuzz)
        assert_equal(0.0, @img.fuzz)
        assert_nothing_raised { @img.fuzz = 50 }
        assert_equal(50.0, @img.fuzz)
        assert_nothing_raised { @img.fuzz = '50%' }
        assert_in_delta(Magick::QuantumRange * 0.50, @img.fuzz, 0.1)
        assert_raise(TypeError) { @img.fuzz = [] }
        assert_raise(ArgumentError) { @img.fuzz = 'xxx' }
    end

    def test_gamma
        assert_nothing_raised { @img.gamma }
        assert_instance_of(Float, @img.gamma)
        assert_equal(0.0, @img.gamma)
        assert_nothing_raised { @img.gamma = 2.0 }
        assert_equal(2.0, @img.gamma)
        assert_raise(TypeError) { @img.gamma = 'x' }
    end

    def test_geometry
        assert_nothing_raised { @img.geometry }
        assert_nil(@img.geometry)
        assert_nothing_raised { @img.geometry = '90x90' }
        assert_equal('90x90', @img.geometry)
        assert_nothing_raised { @img.geometry = Magick::Geometry.new(100, 80) }
        assert_equal('100x80', @img.geometry)
        assert_raise(TypeError) { @img.geometry = [] }
    end

    def test_image_type
        assert_nothing_raised { @img.image_type }
        assert_instance_of(Magick::ImageType, @img.image_type)
        assert_equal(Magick::GrayscaleMatteType, @img.image_type)
    end

    def test_interlace_type
        assert_nothing_raised { @img.interlace }
        assert_instance_of(Magick::InterlaceType, @img.interlace)
        assert_equal(Magick::NoInterlace, @img.interlace)
        assert_nothing_raised { @img.interlace = Magick::LineInterlace }
        assert_equal(Magick::LineInterlace, @img.interlace)
        assert_nothing_raised { @img.interlace = Magick::PlaneInterlace }
        assert_nothing_raised { @img.interlace = Magick::PartitionInterlace }
        assert_raise(TypeError) { @img.interlace = 2 }
    end

    def test_iptc_profile
        assert_nothing_raised { @img.iptc_profile }
        assert_nil(@img.iptc_profile)
        assert_nothing_raised { @img.iptc_profile = 'xxx' }
        assert_raise(TypeError) { @img.iptc_profile = 2 }
    end

    def test_matte
        assert_nothing_raised { @img.matte }
        assert(@img.matte)
        assert_nothing_raised { @img.matte = false }
        assert(!@img.matte)
    end

    def test_mean_error
        assert_nothing_raised { @hat.mean_error_per_pixel }
        assert_nothing_raised { @hat.normalized_mean_error }
        assert_nothing_raised { @hat.normalized_maximum_error }
        assert_equal(0.0, @hat.mean_error_per_pixel)
        assert_equal(0.0, @hat.normalized_mean_error)
        assert_equal(0.0, @hat.normalized_maximum_error)

        hat = @hat.quantize(16, Magick::RGBColorspace, true, 0, true)

        assert_not_equal(0.0, hat.mean_error_per_pixel)
        assert_not_equal(0.0, hat.normalized_mean_error)
        assert_not_equal(0.0, hat.normalized_maximum_error)
        assert_raise(NoMethodError) { hat.mean_error_per_pixel = 1 }
        assert_raise(NoMethodError) { hat.normalized_mean_error = 1 }
        assert_raise(NoMethodError) { hat.normalized_maximum_error = 1 }
    end

    def test_mime_type
        img = @img.copy
        img.format = 'GIF'
        assert_nothing_raised { img.mime_type }
        assert_equal('image/gif', img.mime_type)
        img.format = 'JPG'
        assert_equal('image/jpeg', img.mime_type)
        assert_raise(NoMethodError) { img.mime_type = 'image/jpeg' }
    end

    def test_monitor
        assert_raise(NoMethodError) { @img.monitor }
        monitor = Proc.new { |name, q, s| puts name }
        assert_nothing_raised { @img.monitor = monitor }
        assert_nothing_raised { @img.monitor = nil }
    end

    def test_montage
        assert_nothing_raised { @img.montage }
        assert_nil(@img.montage)
    end

    def test_number_colors
        assert_nothing_raised { @hat.number_colors }
        assert_equal(27980, @hat.number_colors)
        assert_raise(NoMethodError) { @hat.number_colors = 2 }
    end

    def test_offset
        assert_nothing_raised { @img.offset }
        assert_equal(0, @img.offset)
        assert_nothing_raised { @img.offset = 10 }
        assert_equal(10, @img.offset)
        assert_raise(TypeError) { @img.offset = 'x' }
    end

    def test_opacity
        assert_raise(NoMethodError) { @img.opacity }
        assert_nothing_raised { @img.opacity = 50 }
        assert_raise(TypeError) { @img.opacity = 'x' }
    end

    def test_orientation
        assert_nothing_raised { @img.orientation }
        assert_instance_of(Magick::OrientationType, @img.orientation)
        assert_equal(Magick::UndefinedOrientation, @img.orientation)
        assert_nothing_raised { @img.orientation = Magick::UndefinedOrientation }
        assert_nothing_raised { @img.orientation = Magick::TopLeftOrientation }
        assert_nothing_raised { @img.orientation = Magick::TopRightOrientation }
        assert_nothing_raised { @img.orientation = Magick::BottomRightOrientation }
        assert_nothing_raised { @img.orientation = Magick::LeftTopOrientation }
        assert_nothing_raised { @img.orientation = Magick::RightTopOrientation }
        assert_nothing_raised { @img.orientation = Magick::RightBottomOrientation }
        assert_nothing_raised { @img.orientation = Magick::LeftBottomOrientation }
    end

    def test_page
        assert_nothing_raised { @img.page }
        page = @img.page
        assert_equal(0, page.width)
        assert_equal(0, page.height)
        assert_equal(0, page.x)
        assert_equal(0, page.y)
        page = Magick::Rectangle.new(1,2,3,4)
        assert_nothing_raised { @img.page = page }
        assert_equal(1, page.width)
        assert_equal(2, page.height)
        assert_equal(3, page.x)
        assert_equal(4, page.y)
        assert_raise(TypeError) { @img.page = 2 }
    end

    def test_quality
        assert_nothing_raised { @hat.quality }
        assert_equal(75, @hat.quality)
        assert_raise(NoMethodError) { @img.quality = 80 }
    end

    def test_quantum_depth
        assert_nothing_raised { @img.quantum_depth }
        assert_equal(Magick::QuantumDepth, @img.quantum_depth)
        assert_raise(NoMethodError) { @img.quantum_depth = 8 }
    end

    def test_rendering_intent
        assert_nothing_raised { @img.rendering_intent }
        assert_instance_of(Magick::RenderingIntent, @img.rendering_intent)
        assert_equal(Magick::UndefinedIntent, @img.rendering_intent)
        assert_nothing_raised { @img.rendering_intent = Magick::SaturationIntent }
        assert_nothing_raised { @img.rendering_intent = Magick::PerceptualIntent }
        assert_nothing_raised { @img.rendering_intent = Magick::AbsoluteIntent }
        assert_nothing_raised { @img.rendering_intent = Magick::RelativeIntent }
        assert_raise(TypeError) { @img.rendering_intent = 2 }
    end

    def test_rows
        assert_nothing_raised { @img.rows }
        assert_equal(100, @img.rows)
        assert_raise(NoMethodError) { @img.rows = 2 }
    end

    def test_scene
        ilist = Magick::ImageList.new
        ilist << @img
        img = @img.copy
        ilist << img
        ilist.write('temp.gif')
        FileUtils.rm('temp.gif')

        assert_nothing_raised { img.scene }
        assert_equal(0, @img.scene)
        assert_equal(1, img.scene)
        assert_raise(NoMethodError) { img.scene = 2 }
    end

    def test_start_loop
        assert_nothing_raised { @img.start_loop }
        assert(! @img.start_loop)
        assert_nothing_raised { @img.start_loop = true }
        assert(@img.start_loop)
    end

    def test_ticks_per_second
        assert_nothing_raised { @img.ticks_per_second }
        assert_equal(100, @img.ticks_per_second)
        assert_nothing_raised { @img.ticks_per_second = 1000 }
        assert_equal(1000, @img.ticks_per_second)
        assert_raise(TypeError) { @img.ticks_per_second = 'x' }
    end

    def test_total_colors
        assert_nothing_raised { @hat.total_colors }
        assert_equal(27980, @hat.total_colors)
        assert_raise(NoMethodError) { @img.total_colors = 2 }
    end

    def test_units
        assert_nothing_raised { @img.units }
        assert_instance_of(Magick::ResolutionType, @img.units)
        assert_equal(Magick::UndefinedResolution, @img.units)
        assert_nothing_raised { @img.units = Magick::PixelsPerInchResolution }
        assert_equal(Magick::PixelsPerInchResolution, @img.units)
        assert_nothing_raised { @img.units = Magick::PixelsPerCentimeterResolution }
        assert_raise(TypeError) { @img.units = 2 }
    end

    def test_virtual_pixel_method
        assert_nothing_raised { @img.virtual_pixel_method }
        assert_equal(Magick::UndefinedVirtualPixelMethod, @img.virtual_pixel_method)
        assert_nothing_raised { @img.virtual_pixel_method = Magick::EdgeVirtualPixelMethod }
        assert_equal(Magick::EdgeVirtualPixelMethod, @img.virtual_pixel_method)
        assert_nothing_raised { @img.virtual_pixel_method = Magick::MirrorVirtualPixelMethod }
        assert_nothing_raised { @img.virtual_pixel_method = Magick::TileVirtualPixelMethod }
        assert_nothing_raised { @img.virtual_pixel_method = Magick::TransparentVirtualPixelMethod }
        assert_nothing_raised { @img.virtual_pixel_method = Magick::BackgroundVirtualPixelMethod }
        assert_raise(TypeError) { @img.virtual_pixel_method = 2 }
    end

    def test_x_resolution
        assert_nothing_raised { @img.x_resolution }
        assert_equal(72.0, @img.x_resolution)
        assert_nothing_raised { @img.x_resolution = 90 }
        assert_equal(90.0, @img.x_resolution)
        assert_raise(TypeError) { @img.x_resolution = 'x' }
    end

    def test_y_resolution
        assert_nothing_raised { @img.y_resolution }
        assert_equal(72.0, @img.y_resolution)
        assert_nothing_raised { @img.y_resolution = 90 }
        assert_equal(90.0, @img.y_resolution)
        assert_raise(TypeError) { @img.y_resolution = 'x' }
    end

    def test_frozen
        @img.freeze
        assert_raise(FreezeError) { @img.background_color = 'xxx' }
        assert_raise(FreezeError) { @img.blur = 50 }
        assert_raise(FreezeError) { @img.border_color = 'xxx' }
        rp = Magick::Point.new(1,1)
        gp = Magick::Point.new(1,1)
        bp = Magick::Point.new(1,1)
        wp = Magick::Point.new(1,1)
        assert_raise(FreezeError) { @img.chromaticity = Magick::Chromaticity.new(rp, gp, bp, wp) }
        assert_raise(FreezeError) { @img.class_type = Magick::DirectClass }
        assert_raise(FreezeError) { @img.color_profile = 'xxx' }
        assert_raise(FreezeError) { @img.colorspace = Magick::RGBColorspace }
        assert_raise(FreezeError) { @img.compose = Magick::OverCompositeOp }
        assert_raise(FreezeError) { @img.compression = Magick::RLECompression }
        assert_raise(FreezeError) { @img.delay = 2 }
        assert_raise(FreezeError) { @img.density = '72.0x72.0' }
        assert_raise(FreezeError) { @img.dispose = Magick::NoneDispose }
        assert_raise(FreezeError) { @img.endian = Magick::MSBEndian }
        assert_raise(FreezeError) { @img.extract_info = Magick::Rectangle.new(1,2,3,4) }
        assert_raise(FreezeError) { @img.filter = Magick::PointFilter }
        assert_raise(FreezeError) { @img.format = 'GIF' }
        assert_raise(FreezeError) { @img.fuzz = 50.0 }
        assert_raise(FreezeError) { @img.gamma = 2.0 }
        assert_raise(FreezeError) { @img.geometry = '100x100' }
        assert_raise(FreezeError) { @img.interlace = Magick::NoInterlace }
        assert_raise(FreezeError) { @img.iptc_profile = 'xxx' }
        assert_raise(FreezeError) { @img.mask = @img }
        assert_raise(FreezeError) { @img.matte = true }
        assert_raise(FreezeError) { @img.monitor = Proc.new { |name, q, s| puts name } }
        assert_raise(FreezeError) { @img.offset = 100 }
        assert_raise(FreezeError) { @img.opacity = 100 }
        assert_raise(FreezeError) { @img.page = Magick::Rectangle.new(1,2,3,4) }
        assert_raise(FreezeError) { @img.rendering_intent = Magick::SaturationIntent }
        assert_raise(FreezeError) { @img.start_loop = true }
        assert_raise(FreezeError) { @img.ticks_per_second = 1000 }
        assert_raise(FreezeError) { @img.units = Magick::PixelsPerInchResolution }
        assert_raise(FreezeError) { @img.x_resolution = 72.0 }
        assert_raise(FreezeError) { @img.y_resolution = 72.0 }
    end

end     # class Image_Attributes_UT


if __FILE__ == $0
FLOWER_HAT = '../doc/ex/images/Flower_Hat.jpg'
Test::Unit::UI::Console::TestRunner.run(Image_Attributes_UT) if RUBY_VERSION != '1.9.1'
end
