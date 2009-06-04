#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'  if RUBY_VERSION != '1.9.1'
require 'fileutils'

ColorspaceTypes = [
  Magick::RGBColorspace,
  Magick::GRAYColorspace,
  Magick::TransparentColorspace,
  Magick::OHTAColorspace,
  Magick::LABColorspace,
  Magick::XYZColorspace,
  Magick::YCbCrColorspace,
  Magick::YCCColorspace,
  Magick::YIQColorspace,
  Magick::YPbPrColorspace,
  Magick::YUVColorspace,
  Magick::CMYKColorspace,
  Magick::SRGBColorspace,
  Magick::HSLColorspace,
  Magick::HWBColorspace,
  Magick::HSBColorspace,
  Magick::LABColorspace,
  Magick::Rec601LumaColorspace,
  Magick::Rec601YCbCrColorspace,
  Magick::Rec709LumaColorspace,
  Magick::Rec709YCbCrColorspace,
  Magick::LogColorspace
]



class Image3_UT < Test::Unit::TestCase
    FreezeError = RUBY_VERSION == '1.9.1' ? RuntimeError : TypeError

    def setup
        @img = Magick::Image.new(20, 20)
    end

    def test_profile!
        assert_nothing_raised do
            res = @img.profile!('*', nil)
            assert_same(@img, res)
        end
        assert_nothing_raised { @img.profile!('icc', 'xxx') }
        assert_nothing_raised { @img.profile!('iptc', 'xxx') }
        assert_nothing_raised { @img.profile!('icc', nil) }
        assert_nothing_raised { @img.profile!('iptc', nil) }

        @img.freeze
        assert_raise(FreezeError) { @img.profile!('icc', 'xxx') }
        assert_raise(FreezeError) { @img.profile!('*', nil) }
    end

    def test_quantize
        assert_nothing_raised do
            res = @img.quantize
            assert_instance_of(Magick::Image, res)
        end

        ColorspaceTypes.each do |cs|
            assert_nothing_raised { @img.quantize(256, cs) }
        end
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, false) }
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, true) }
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, Magick::NoDitherMethod) }
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, Magick::RiemersmaDitherMethod) }
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, Magick::FloydSteinbergDitherMethod) }
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, true, 2) }
        assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, true, 2, true) }
        assert_raise(TypeError) { @img.quantize('x') }
        assert_raise(TypeError) { @img.quantize(16, 2) }
        assert_raise(TypeError) { @img.quantize(16, Magick::RGBColorspace, false, 'x') }
    end

    def test_quantum_operator
        quantum_ops = [
            Magick::AddQuantumOperator,
            Magick::AndQuantumOperator,
            Magick::DivideQuantumOperator,
            Magick::LShiftQuantumOperator,
            Magick::MultiplyQuantumOperator,
            Magick::OrQuantumOperator,
            Magick::RShiftQuantumOperator,
            Magick::SubtractQuantumOperator,
            Magick::XorQuantumOperator ]

        assert_nothing_raised do
            res = @img.quantum_operator(Magick::AddQuantumOperator, 2)
            assert_instance_of(Magick::Image, res)
        end
        quantum_ops.each do |op|
            assert_nothing_raised { @img.quantum_operator(op, 2) }
        end
        assert_nothing_raised { @img.quantum_operator(Magick::AddQuantumOperator, 2, Magick::RedChannel) }
        assert_raise(TypeError) { @img.quantum_operator(2, 2) }
        assert_raise(TypeError) { @img.quantum_operator(Magick::AddQuantumOperator, 'x') }
        assert_raise(TypeError) { @img.quantum_operator(Magick::AddQuantumOperator, 2, 2) }
        assert_raise(ArgumentError) { @img.quantum_operator(Magick::AddQuantumOperator, 2, Magick::RedChannel, 2) }
    end

    def test_radial_blur
        assert_nothing_raised do
            res = @img.radial_blur(30)
            assert_instance_of(Magick::Image, res)
        end
    end

    def test_radial_blur_channel
        res = nil
        assert_nothing_raised { res = @img.radial_blur_channel(30) }
        assert_not_nil(res)
        assert_instance_of(Magick::Image, res)
        assert_nothing_raised { res = @img.radial_blur_channel(30, Magick::RedChannel) }
        assert_nothing_raised { res = @img.radial_blur_channel(30, Magick::RedChannel, Magick::BlueChannel) }

        assert_raise(ArgumentError) { @img.radial_blur_channel }
        assert_raise(TypeError) { @img.radial_blur_channel(30, 2) }
    end

    def test_raise
        assert_nothing_raised do
            res = @img.raise
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.raise(4) }
        assert_nothing_raised { @img.raise(4,4) }
        assert_nothing_raised { @img.raise(4,4, false) }
        assert_raise(TypeError) { @img.raise('x') }
        assert_raise(TypeError) { @img.raise(2, 'x') }
        assert_raise(ArgumentError) { @img.raise(4, 4, false, 2) }
    end

    def test_random_threshold_channel
        assert_nothing_raised do
            res = @img.random_threshold_channel('20%')
            assert_instance_of(Magick::Image, res)
        end
        threshold = Magick::Geometry.new(20)
        assert_nothing_raised { @img.random_threshold_channel(threshold) }
        assert_nothing_raised { @img.random_threshold_channel(threshold, Magick::RedChannel) }
        assert_nothing_raised { @img.random_threshold_channel(threshold, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(ArgumentError) { @img.random_threshold_channel }
        assert_raise(TypeError) { @img.random_threshold_channel('20%', 2) }
    end

    def test_reduce_noise
        assert_nothing_raised do
            res = @img.reduce_noise(0)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.reduce_noise(4) }
    end

    def test_remap
       remap_image = Magick::Image.new(20,20) {self.background_color = "green"}
       assert_nothing_raised { @img.remap(remap_image) }
       assert_nothing_raised { @img.remap(remap_image, Magick::NoDitherMethod) }
       assert_nothing_raised { @img.remap(remap_image, Magick::RiemersmaDitherMethod) }
       assert_nothing_raised { @img.remap(remap_image, Magick::FloydSteinbergDitherMethod) }

       assert_raise(ArgumentError) {@img.remap() }
       assert_raise(ArgumentError) {@img.remap(remap_image, Magick::NoDitherMethod, 1) }
       assert_raise(TypeError) {@img.remap(remap_image, 1) }
    end

    def test_resample
        assert_nothing_raised { @img.resample }
        assert_nothing_raised { @img.resample(100) }
        assert_nothing_raised { @img.resample(100, 100) }

        girl = Magick::Image.read(IMAGES_DIR+'/Flower_Hat.jpg').first
        assert_equal(240.0, girl.x_resolution)
        assert_equal(240.0, girl.y_resolution)
        res = girl.resample(120, 120)
        assert_equal(100, res.columns)
        assert_equal(125, res.rows)
        assert_equal(120.0, res.x_resolution)
        assert_equal(120.0, res.y_resolution)

        assert_raise(NoMethodError) { @img.resample('x') }
        assert_raise(NoMethodError) { @img.resample(100, 'x') }
    end

    def test_resize
        assert_nothing_raised do
            res = @img.resize(2)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.resize(50,50) }
        filters = [
          Magick::PointFilter,
          Magick::BoxFilter,
          Magick::TriangleFilter,
          Magick::HermiteFilter,
          Magick::HanningFilter,
          Magick::HammingFilter,
          Magick::BlackmanFilter,
          Magick::GaussianFilter,
          Magick::QuadraticFilter,
          Magick::CubicFilter,
          Magick::CatromFilter,
          Magick::MitchellFilter,
          Magick::LanczosFilter,
          Magick::BesselFilter,
          Magick::SincFilter ]

        filters.each do |filter|
            assert_nothing_raised { @img.resize(50, 50, filter) }
        end
        assert_nothing_raised { @img.resize(50, 50, Magick::PointFilter, 2.0) }
        assert_raise(TypeError) { @img.resize('x') }
        assert_raise(TypeError) { @img.resize(50, 'x') }
        assert_raise(TypeError) { @img.resize(50, 50, 2) }
        assert_raise(TypeError) { @img.resize(50, 50, Magick::CubicFilter, 'x') }
        assert_raise(ArgumentError) { @img.resize(50, 50, Magick::SincFilter, 2.0, 'x') }
        assert_raise(ArgumentError) { @img.resize }
    end

    def test_resize!
        assert_nothing_raised do
            res = @img.resize!(2)
            assert_same(@img, res)
        end
        @img.freeze
        assert_raise(FreezeError) { @img.resize!(0.50) }
    end

    def test_resize_to_fill_0
        changed = @img.resize_to_fill(@img.columns,@img.rows)
        assert_equal(@img.columns, changed.columns)
        assert_equal(@img.rows, changed.rows)
        assert_not_same(changed, @img)
    end

    def test_resize_to_fill_1
        @img = Magick::Image.new(200, 250)
        @img.resize_to_fill!(100,100)
        assert_equal(100, @img.columns)
        assert_equal(100, @img.rows)
    end

    def test_resize_to_fill_2
        @img = Magick::Image.new(200, 250)
        changed = @img.resize_to_fill(300,100)
        assert_equal(300, changed.columns)
        assert_equal(100, changed.rows)
    end

    def test_resize_to_fill_3
        @img = Magick::Image.new(200, 250)
        changed = @img.resize_to_fill(100,300)
        assert_equal(100, changed.columns)
        assert_equal(300, changed.rows)
    end

    def test_resize_to_fill_4
        @img = Magick::Image.new(200, 250)
        changed = @img.resize_to_fill(300,350)
        assert_equal(300, changed.columns)
        assert_equal(350, changed.rows)
    end

    def test_resize_to_fill_5
        changed = @img.resize_to_fill(20,400)
        assert_equal(20, changed.columns)
        assert_equal(400, changed.rows)
    end
    def test_resize_to_fill_6
        changed = @img.resize_to_fill(3000,400)
        assert_equal(3000, changed.columns)
        assert_equal(400, changed.rows)
    end

    # Make sure the old name is still around
    def test_resize_to_fill_7
      assert_block {@img.respond_to? :crop_resized}
      assert_block {@img.respond_to? :crop_resized!}
    end

    # 2nd argument defaults to the same value as the 1st argument
    def test_resize_to_fill_8
        changed = @img.resize_to_fill(100)
        assert_equal(100, changed.columns)
        assert_equal(100, changed.rows)
    end

    def test_resize_to_fit
        img = Magick::Image.new(200, 250)
        res = nil
        assert_nothing_raised { res = img.resize_to_fit(50, 50) }
        assert_not_nil(res)
        assert_instance_of(Magick::Image, res)
        assert_not_same(img, res)
        assert_equal(40, res.columns)
        assert_equal(50, res.rows)
    end

    def test_resize_to_fit2
      img = Magick::Image.new(200, 300)
      changed = img.resize_to_fit(100)
      assert_instance_of(Magick::Image, changed)
      assert_not_same(img, changed)
      assert_equal(67, changed.columns)
      assert_equal(100, changed.rows)
    end

    def test_resize_to_fit3
      img = Magick::Image.new(200, 300)
      keep = img
      img.resize_to_fit!(100)
      assert_instance_of(Magick::Image, img)
      assert_same(img, keep)
      assert_equal(67, img.columns)
      assert_equal(100, img.rows)
    end

    def test_roll
        assert_nothing_raised do
            res = @img.roll(5, 5)
            assert_instance_of(Magick::Image, res)
        end
    end

    def test_rotate
        assert_nothing_raised do
            res = @img.rotate(45)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.rotate(-45) }

        img = Magick::Image.new(100, 50)
        assert_nothing_raised do
            res = img.rotate(90, '>')
            assert_instance_of(Magick::Image, res)
            assert_equal(50, res.columns)
            assert_equal(100, res.rows);
        end
        assert_nothing_raised do
            res = img.rotate(90, '<')
            assert_nil(res)
        end
        assert_raise(ArgumentError) { img.rotate(90, 't') }
        assert_raise(TypeError) { img.rotate(90, []) }
    end

    def test_rotate!
        assert_nothing_raised do
            res = @img.rotate!(45)
            assert_same(@img, res)
        end
        @img.freeze
        assert_raise(FreezeError) { @img.rotate!(45) }
    end

    def test_sample
        assert_nothing_raised do
            res = @img.sample(10, 10)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.sample(2) }
        assert_raise(ArgumentError) { @img.sample }
        assert_raise(ArgumentError) { @img.sample(25, 25, 25) }
        assert_raise(TypeError) { @img.sample('x') }
        assert_raise(TypeError) { @img.sample(10, 'x') }
    end

    def test_sample!
        assert_nothing_raised do
            res = @img.sample!(2)
            assert_same(@img, res)
        end
        @img.freeze
        assert_raise(FreezeError) { @img.sample!(0.50) }
    end

    def test_scale
        assert_nothing_raised do
            res = @img.scale(10, 10)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.scale(2) }
        assert_raise(ArgumentError) { @img.scale }
        assert_raise(ArgumentError) { @img.scale(25, 25, 25) }
        assert_raise(TypeError) { @img.scale('x') }
        assert_raise(TypeError) { @img.scale(10, 'x') }
    end

    def test_scale!
        assert_nothing_raised do
            res = @img.scale!(2)
            assert_same(@img, res)
        end
        @img.freeze
        assert_raise(FreezeError) { @img.scale!(0.50) }
    end

    def test_segment
        assert_nothing_raised do
            res = @img.segment
            assert_instance_of(Magick::Image, res)
        end

        # Don't test colorspaces that require PsuedoColor images
        (ColorspaceTypes - [Magick::OHTAColorspace,
                            Magick::LABColorspace,
                            Magick::XYZColorspace,
                            Magick::YCbCrColorspace,
                            Magick::YCCColorspace,
                            Magick::YIQColorspace,
                            Magick::YPbPrColorspace,
                            Magick::YUVColorspace,
                            Magick::Rec601YCbCrColorspace,
                            Magick::Rec709YCbCrColorspace,
                            Magick::LogColorspace]).each do |cs|
            assert_nothing_raised { @img.segment(cs) }
        end

        assert_nothing_raised { @img.segment(Magick::RGBColorspace, 2.0) }
        assert_nothing_raised { @img.segment(Magick::RGBColorspace, 2.0, 2.0) }
        assert_nothing_raised { @img.segment(Magick::RGBColorspace,  2.0, 2.0, false) }

        assert_raise(ArgumentError) { @img.segment(Magick::RGBColorspace, 2.0, 2.0, false, 2) }
        assert_raise(TypeError) { @img.segment(2) }
        assert_raise(TypeError) { @img.segment(Magick::RGBColorspace, 'x') }
        assert_raise(TypeError) { @img.segment(Magick::RGBColorspace, 2.0, 'x') }
    end

    def test_selective_blur_channel
        res = nil
        assert_nothing_raised { res = @img.selective_blur_channel(0, 1, '10%') }
        assert_instance_of(Magick::Image, res)
        assert_not_same(@img, res)
        assert_equal([@img.columns, @img.rows], [res.columns, res.rows])

        assert_nothing_raised { @img.selective_blur_channel(0, 1, 0.1) }
        assert_nothing_raised { @img.selective_blur_channel(0, 1, '10%', Magick::RedChannel) }
        assert_nothing_raised { @img.selective_blur_channel(0, 1, '10%', Magick::RedChannel, Magick::BlueChannel) }
        assert_nothing_raised { @img.selective_blur_channel(0, 1, '10%', Magick::RedChannel, Magick::BlueChannel, Magick::GreenChannel) }
        # not enough arguments
        assert_raise(ArgumentError) { @img.selective_blur_channel(0, 1) }
    end

    def test_sepiatone
        assert_nothing_raised do
            res = @img.sepiatone
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.sepiatone(Magick::QuantumRange*0.80) }
        assert_raise(ArgumentError) { @img.sepiatone(Magick::QuantumRange, 2) }
        assert_raise(TypeError) { @img.sepiatone('x') }
    end

    def test_set_channel_depth
        channels = [
              Magick::RedChannel,
              Magick::GrayChannel,
              Magick::CyanChannel,
              Magick::GreenChannel,
              Magick::MagentaChannel,
              Magick::BlueChannel,
              Magick::YellowChannel,
        #     Magick::AlphaChannel,
              Magick::OpacityChannel,
              Magick::MatteChannel,
              Magick::BlackChannel,
              Magick::IndexChannel,
              Magick::AllChannels]

        channels.each do |ch|
            assert_nothing_raised {@img.set_channel_depth(ch, 8) }
        end
    end

    def test_shade
        assert_nothing_raised do
            res = @img.shade
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.shade(true) }
        assert_nothing_raised { @img.shade(true, 30) }
        assert_nothing_raised { @img.shade(true, 30, 30) }
        assert_raise(ArgumentError) { @img.shade(true, 30, 30, 2) }
        assert_raise(TypeError) { @img.shade(true, 'x') }
        assert_raise(TypeError) { @img.shade(true, 30, 'x') }
    end

    def test_shadow
        assert_nothing_raised do
            res = @img.shadow
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.shadow(5) }
        assert_nothing_raised { @img.shadow(5, 5) }
        assert_nothing_raised { @img.shadow(5, 5, 3.0) }
        assert_nothing_raised { @img.shadow(5, 5, 3.0, 0.50) }
        assert_nothing_raised { @img.shadow(5, 5, 3.0, '50%') }
        assert_raise(ArgumentError) { @img.shadow(5, 5, 3.0, 0.50, 2) }
        assert_raise(TypeError) { @img.shadow('x') }
        assert_raise(TypeError) { @img.shadow(5, 'x') }
        assert_raise(TypeError) { @img.shadow(5, 5, 'x') }
        assert_raise(ArgumentError) { @img.shadow(5, 5, 3.0, 'x') }
    end

    def test_sharpen
        assert_nothing_raised do
            res = @img.sharpen
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.sharpen(2.0) }
        assert_nothing_raised { @img.sharpen(2.0, 1.0) }
        assert_raise(ArgumentError) { @img.sharpen(2.0, 1.0, 2) }
        assert_raise(TypeError) { @img.sharpen('x') }
        assert_raise(TypeError) { @img.sharpen(2.0, 'x') }
    end

    def test_sharpen_channel
        assert_nothing_raised do
            res = @img.sharpen_channel
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.sharpen_channel(2.0) }
        assert_nothing_raised { @img.sharpen_channel(2.0, 1.0) }
        assert_nothing_raised { @img.sharpen_channel(2.0, 1.0, Magick::RedChannel) }
        assert_nothing_raised { @img.sharpen_channel(2.0, 1.0, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.sharpen_channel(2.0, 1.0, Magick::RedChannel, 2) }
        assert_raise(TypeError) { @img.sharpen_channel('x') }
        assert_raise(TypeError) { @img.sharpen_channel(2.0, 'x') }
    end

    def test_shave
        assert_nothing_raised do
            res = @img.shave(5,5)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised do
            res = @img.shave!(5,5)
            assert_same(@img, res)
        end
        @img.freeze
        assert_raise(FreezeError) { @img.shave!(2,2) }
    end

    def test_shear
        assert_nothing_raised do
            res = @img.shear(30, 30)
            assert_instance_of(Magick::Image, res)
        end
    end

    def test_sigmoidal_contrast_channel
        assert_nothing_raised do
            res = @img.sigmoidal_contrast_channel
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.sigmoidal_contrast_channel(3.0) }
        assert_nothing_raised { @img.sigmoidal_contrast_channel(3.0, 50.0) }
        assert_nothing_raised { @img.sigmoidal_contrast_channel(3.0, 50.0, true) }
        assert_nothing_raised { @img.sigmoidal_contrast_channel(3.0, 50.0, true, Magick::RedChannel) }
        assert_nothing_raised { @img.sigmoidal_contrast_channel(3.0, 50.0, true, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.sigmoidal_contrast_channel(3.0, 50.0, true, Magick::RedChannel, 2) }
        assert_raise(TypeError) { @img.sigmoidal_contrast_channel('x') }
        assert_raise(TypeError) { @img.sigmoidal_contrast_channel(3.0, 'x') }
    end

    def test_signature
        assert_nothing_raised do
            res = @img.signature
            assert_instance_of(String, res)
        end
    end

    def test_sketch
        assert_nothing_raised { @img.sketch }
        assert_nothing_raised { @img.sketch(0) }
        assert_nothing_raised { @img.sketch(0, 1) }
        assert_nothing_raised { @img.sketch(0, 1, 0) }
        assert_raise(ArgumentError) { @img.sketch(0, 1, 0, 1) }
        assert_raise(TypeError) { @img.sketch('x') }
        assert_raise(TypeError) { @img.sketch(0, 'x') }
        assert_raise(TypeError) { @img.sketch(0, 1, 'x') }
    end

    def test_solarize
        assert_nothing_raised do
            res = @img.solarize
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.solarize(100) }
        assert_raise(ArgumentError) { @img.solarize(100, 2) }
        assert_raise(TypeError) { @img.solarize('x') }
    end

    def test_sparse_color
        img = Magick::Image.new(100, 100)
        args = [30, 10, 'red', 10, 80, 'blue', 70, 60, 'lime', 80, 20, 'yellow']
        # assert good calls work
        Magick::SparseColorMethod.values do |v|
            next if v == Magick::UndefinedColorInterpolate
            assert_nothing_raised { img.sparse_color(v, *args) }
        end
        args << Magick::RedChannel
        assert_nothing_raised { img.sparse_color(Magick::VoronoiColorInterpolate, *args) }
        args << Magick::GreenChannel
        assert_nothing_raised { img.sparse_color(Magick::VoronoiColorInterpolate, *args) }
        args << Magick::BlueChannel
        assert_nothing_raised { img.sparse_color(Magick::VoronoiColorInterpolate, *args) }

        # bad calls
        args = [30, 10, 'red', 10, 80, 'blue', 70, 60, 'lime', 80, 20, 'yellow']
        # invalid method
        assert_raise(TypeError) { img.sparse_color(1, *args) }
        # missing arguments
        assert_raise(ArgumentError) { img.sparse_color(Magick::VoronoiColorInterpolate) }
        args << 10   # too many arguments
        assert_raise(ArgumentError) { img.sparse_color(Magick::VoronoiColorInterpolate, *args) }
        args.shift
        args.shift  # too few
        assert_raise(ArgumentError) { img.sparse_color(Magick::VoronoiColorInterpolate, *args) }
    end

    def test_splice
        assert_nothing_raised do
            res = @img.splice(0, 0, 2, 2)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.splice(0, 0, 2, 2, 'red') }
        red = Magick::Pixel.new(Magick::QuantumRange)
        assert_nothing_raised { @img.splice(0, 0, 2, 2, red) }
        assert_raise(ArgumentError) { @img.splice(0,0, 2, 2, red, 'x') }
        assert_raise(TypeError) { @img.splice([], 0, 2, 2, red) }
        assert_raise(TypeError) { @img.splice(0, 'x', 2, 2, red) }
        assert_raise(TypeError) { @img.splice(0, 0, 'x', 2, red) }
        assert_raise(TypeError) { @img.splice(0, 0, 2, [], red) }
        assert_raise(TypeError) { @img.splice(0, 0, 2, 2, /m/) }
    end

    def test_spread
        assert_nothing_raised do
            res = @img.spread
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.spread(3.0) }
        assert_raise(ArgumentError) { @img.spread(3.0, 2) }
        assert_raise(TypeError) { @img.spread('x') }
    end

    def test_stegano
        img = Magick::Image.new(100, 100) { self.background_color = 'black' }
        watermark = Magick::Image.new(10, 10) { self.background_color = 'white' }
        assert_nothing_raised do
            res = @img.stegano(watermark, 0)
            assert_instance_of(Magick::Image, res)
        end

        watermark.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.stegano(watermark, 0) }
    end

    def test_stereo
        assert_nothing_raised do
            res = @img.stereo(@img)
            assert_instance_of(Magick::Image, res)
        end

        img = Magick::Image.new(20,20)
        img.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.stereo(img) }
    end

    def test_store_pixels
        pixels = @img.get_pixels(0, 0, @img.columns, 1)
        assert_nothing_raised do
            res = @img.store_pixels(0, 0, @img.columns, 1, pixels)
            assert_same(@img, res)
        end

        pixels[0] = 'x'
        assert_raise(TypeError) { @img.store_pixels(0, 0, @img.columns, 1, pixels) }
        assert_raise(RangeError) { @img.store_pixels(-1, 0, @img.columns, 1, pixels) }
        assert_raise(RangeError) { @img.store_pixels(0, -1, @img.columns, 1, pixels) }
        assert_raise(RangeError) { @img.store_pixels(0, 0, 1+@img.columns, 1, pixels) }
        assert_raise(RangeError) { @img.store_pixels(-1, 0, 1, 1+@img.rows, pixels) }
        assert_raise(IndexError) { @img.store_pixels(0, 0, @img.columns, 1, ['x']) }
    end

    def test_strip!
        assert_nothing_raised do
            res = @img.strip!
            assert_same(@img, res)
        end
    end

    def test_swirl
        assert_nothing_raised do
            res = @img.swirl(30)
            assert_instance_of(Magick::Image, res)
        end
    end

    def test_sync_profiles
        assert_nothing_raised { assert(@img.sync_profiles) }
    end

    def test_texture_fill_to_border
        texture = Magick::Image.read('granite:').first
        assert_nothing_raised do
            res = @img.texture_fill_to_border(@img.columns/2, @img.rows/2, texture)
            assert_instance_of(Magick::Image, res)
        end
        assert_raise(NoMethodError) { @img.texture_fill_to_border(@img.columns/2, @img.rows/2, 'x') }
    end

    def test_texture_floodfill
        texture = Magick::Image.read('granite:').first
        assert_nothing_raised do
            res = @img.texture_floodfill(@img.columns/2, @img.rows/2, texture)
            assert_instance_of(Magick::Image, res)
        end
        assert_raise(NoMethodError) { @img.texture_floodfill(@img.columns/2, @img.rows/2, 'x') }
        texture.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.texture_floodfill(@img.columns/2, @img.rows/2, texture) }
    end

    def test_threshold
        assert_nothing_raised do
            res = @img.threshold(100)
            assert_instance_of(Magick::Image, res)
        end
    end

    def test_thumbnail
        assert_nothing_raised do
            res = @img.thumbnail(10, 10)
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.thumbnail(2) }
        assert_raise(ArgumentError) { @img.thumbnail }
        assert_raise(ArgumentError) { @img.thumbnail(25, 25, 25) }
        assert_raise(TypeError) { @img.thumbnail('x') }
        assert_raise(TypeError) { @img.thumbnail(10, 'x') }
    end

    def test_thumbnail!
        assert_nothing_raised do
            res = @img.thumbnail!(2)
            assert_same(@img, res)
        end
        @img.freeze
        assert_raise(FreezeError) { @img.thumbnail!(0.50) }
    end

    def test_to_blob
        res = nil
        assert_nothing_raised { res = @img.to_blob { self.format = 'miff' } }
        assert_instance_of(String, res)
        restored = Magick::Image.from_blob(res)
        assert_equal(@img, restored[0])
    end

    def test_to_color
        red = Magick::Pixel.new(Magick::QuantumRange)
        assert_nothing_raised do
            res = @img.to_color(red)
            assert_equal('red', res)
        end
    end

    def test_transparent
        assert_nothing_raised do
            res = @img.transparent('white')
            assert_instance_of(Magick::Image, res)
        end
        pixel = Magick::Pixel.new
        assert_nothing_raised { @img.transparent(pixel) }
        assert_nothing_raised { @img.transparent('white', Magick::TransparentOpacity) }
        assert_raise(ArgumentError) { @img.transparent('white', Magick::TransparentOpacity, 2) }
        assert_nothing_raised { @img.transparent('white', Magick::QuantumRange/2) }
        assert_raise(TypeError) { @img.transparent(2) }
    end

    def test_transpose
        assert_nothing_raised do
            res = @img.transpose
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised do
            res = @img.transpose!
            assert_instance_of(Magick::Image, res)
            assert_same(@img, res)
        end
    end

    def test_transverse
        assert_nothing_raised do
            res = @img.transverse
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised do
            res = @img.transverse!
            assert_instance_of(Magick::Image, res)
            assert_same(@img, res)
        end
    end

    def test_trim
        # Can't use the default image because it's a solid color
        hat = Magick::Image.read(IMAGES_DIR+'/Flower_Hat.jpg').first
        assert_nothing_raised do
            res = hat.trim
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised do
            res = hat.trim!
            assert_same(hat, res)
        end
    end

    def test_unique_colors
        assert_nothing_raised do
            res = @img.unique_colors
            assert_instance_of(Magick::Image, res)
            assert_equal(1, res.columns)
            assert_equal(1, res.rows)
        end
    end

    def test_unsharp_mask
        assert_nothing_raised do
            res = @img.unsharp_mask
            assert_instance_of(Magick::Image, res)
        end

        assert_nothing_raised { @img.unsharp_mask(2.0) }
        assert_nothing_raised { @img.unsharp_mask(2.0, 1.0) }
        assert_nothing_raised { @img.unsharp_mask(2.0, 1.0, 0.50) }
        assert_nothing_raised { @img.unsharp_mask(2.0, 1.0, 0.50, 0.10) }
        assert_raise(ArgumentError) { @img.unsharp_mask(2.0, 1.0, 0.50, 0.10, 2) }
        assert_raise(TypeError) { @img.unsharp_mask('x') }
        assert_raise(TypeError) { @img.unsharp_mask(2.0, 'x') }
        assert_raise(TypeError) { @img.unsharp_mask(2.0, 1.0, 'x') }
        assert_raise(TypeError) { @img.unsharp_mask(2.0, 1.0, 0.50, 'x') }
    end

    def test_unsharp_mask_channel
        assert_nothing_raised do
            res = @img.unsharp_mask_channel
            assert_instance_of(Magick::Image, res)
        end

        assert_nothing_raised { @img.unsharp_mask_channel(2.0) }
        assert_nothing_raised { @img.unsharp_mask_channel(2.0, 1.0) }
        assert_nothing_raised { @img.unsharp_mask_channel(2.0, 1.0, 0.50) }
        assert_nothing_raised { @img.unsharp_mask_channel(2.0, 1.0, 0.50, 0.10) }
        assert_nothing_raised { @img.unsharp_mask_channel(2.0, 1.0, 0.50, 0.10, Magick::RedChannel) }
        assert_nothing_raised { @img.unsharp_mask_channel(2.0, 1.0, 0.50, 0.10, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.unsharp_mask_channel(2.0, 1.0, 0.50, 0.10, Magick::RedChannel, 2) }
        assert_raise(TypeError) { @img.unsharp_mask_channel(2.0, 1.0, 0.50, 0.10, 2) }
        assert_raise(TypeError) { @img.unsharp_mask_channel('x') }
        assert_raise(TypeError) { @img.unsharp_mask_channel(2.0, 'x') }
        assert_raise(TypeError) { @img.unsharp_mask_channel(2.0, 1.0, 'x') }
        assert_raise(TypeError) { @img.unsharp_mask_channel(2.0, 1.0, 0.50, 'x') }
    end

    def test_view
        assert_nothing_raised do
            res = @img.view(0, 0, 5, 5)
            assert_instance_of(Magick::Image::View, res)
        end
        assert_nothing_raised do
            @img.view(0, 0, 5, 5) { |v| assert_instance_of(Magick::Image::View, v) }
        end
        assert_raise(RangeError) { @img.view(-1, 0, 5, 5) }
        assert_raise(RangeError) { @img.view(0, -1, 5, 5) }
        assert_raise(RangeError) { @img.view(1, 0, @img.columns, 5) }
        assert_raise(RangeError) { @img.view(0, 1, 5, @img.rows) }
        assert_raise(ArgumentError) { @img.view(0, 0, 0, 1) }
        assert_raise(ArgumentError) { @img.view(0, 0, 1, 0) }
    end

    def test_vignette
        assert_nothing_raised do
            res = @img.vignette
            assert_instance_of(Magick::Image, res)
            assert_not_same(res, @img)
        end
        assert_nothing_raised { @img.vignette(0) }
        assert_nothing_raised { @img.vignette(0, 0) }
        assert_nothing_raised { @img.vignette(0, 0, 0) }
        assert_nothing_raised { @img.vignette(0, 0, 0, 1) }
        # too many arguments
        assert_raise(ArgumentError) { @img.vignette(0, 0, 0, 1, 1) }
    end

    def test_watermark
        mark = Magick::Image.new(5,5)
        mark_list = Magick::ImageList.new
        mark_list << mark.copy
        assert_nothing_raised { @img.watermark(mark) }
        assert_nothing_raised { @img.watermark(mark_list) }
        assert_nothing_raised { @img.watermark(mark, 0.50) }
        assert_nothing_raised { @img.watermark(mark, '50%') }
        assert_nothing_raised { @img.watermark(mark, 0.50, 0.50) }
        assert_nothing_raised { @img.watermark(mark, 0.50, '50%') }
        assert_nothing_raised { @img.watermark(mark, 0.50, 0.50, 10) }
        assert_nothing_raised { @img.watermark(mark, 0.50, 0.50, 10, 10) }
        assert_nothing_raised { @img.watermark(mark, 0.50, 0.50, Magick::NorthEastGravity) }
        assert_nothing_raised { @img.watermark(mark, 0.50, 0.50, Magick::NorthEastGravity, 10) }
        assert_nothing_raised { @img.watermark(mark, 0.50, 0.50, Magick::NorthEastGravity, 10, 10) }

        assert_raise(ArgumentError) { @img.watermark(mark, 'x') }
        assert_raise(ArgumentError) { @img.watermark(mark, 0.50, 'x') }
        assert_raise(TypeError) { @img.watermark(mark, 0.50, 0.50, 'x') }
        assert_raise(TypeError) { @img.watermark(mark, 0.50, 0.50, Magick::NorthEastGravity, 'x') }
        assert_raise(TypeError) { @img.watermark(mark, 0.50, 0.50, Magick::NorthEastGravity, 10, 'x') }

        mark.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.watermark(mark) }
    end

    def test_wave
        assert_nothing_raised do
            res = @img.wave
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.wave(25) }
        assert_nothing_raised { @img.wave(25, 200) }
        assert_raise(ArgumentError) { @img.wave(25, 200, 2) }
        assert_raise(TypeError) { @img.wave('x') }
        assert_raise(TypeError) { @img.wave(25, 'x') }
    end

    def test_white_threshold
        assert_raise(ArgumentError) { @img.white_threshold }
        assert_nothing_raised { @img.white_threshold(50) }
        assert_nothing_raised { @img.white_threshold(50, 50) }
        assert_nothing_raised { @img.white_threshold(50, 50, 50) }
        assert_nothing_raised { @img.white_threshold(50, 50, 50, 50) }
        assert_raise(ArgumentError) { @img.white_threshold(50, 50, 50, 50, 50) }
        res = @img.white_threshold(50)
        assert_instance_of(Magick::Image,  res)
    end

    # test write with #format= attribute
    def test_write

        @img.write('temp.gif')
        img = Magick::Image.read('temp.gif')
        assert_equal("GIF", img.first.format)
        FileUtils.rm('temp.gif')

        @img.write("jpg:temp.foo")
        img = Magick::Image.read('temp.foo')
        assert_equal("JPEG", img.first.format)
        FileUtils.rm('temp.foo')

        @img.write("temp.0") { self.format = "JPEG" }
        img = Magick::Image.read('temp.0')
        assert_equal("JPEG", img.first.format)

        # JPEG has two names.
        @img.write("jpeg:temp.0") { self.format = "JPEG" }
        img = Magick::Image.read('temp.0')
        assert_equal("JPEG", img.first.format)

        @img.write("jpg:temp.0") { self.format = "JPG" }
        img = Magick::Image.read('temp.0')
        assert_equal("JPEG", img.first.format)

        @img.write("jpg:temp.0") { self.format = "JPEG" }
        img = Magick::Image.read('temp.0')
        assert_equal("JPEG", img.first.format)

        @img.write("jpeg:temp.0") { self.format = "JPG" }
        img = Magick::Image.read('temp.0')
        assert_equal("JPEG", img.first.format)

        assert_raise(RuntimeError) do
          @img.write("gif:temp.0") { self.format = "JPEG" }
        end

        f = File.new("test.0", "w")
        @img.write(f) { self.format = "JPEG" }
        f.close
        img = Magick::Image.read('test.0')
        assert_equal("JPEG", img.first.format)
        FileUtils.rm('test.0')
    end


end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
Test::Unit::UI::Console::TestRunner.run(Image3_UT) if RUBY_VERSION != '1.9.1'
end