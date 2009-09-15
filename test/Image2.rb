#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'  if RUBY_VERSION != '1.9.1'

# TODO: improve exif tests - need a benchmark image with EXIF data


class Image2_UT < Test::Unit::TestCase
    FreezeError = RUBY_VERSION == '1.9.1' ? RuntimeError : TypeError

    def setup
        @img = Magick::Image.new(20, 20)
    end

    def test_composite!
        img1 = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        img2 = Magick::Image.read(IMAGES_DIR+'/Button_1.gif').first
        assert_nothing_raised do
            res = img1.composite!(img2, Magick::NorthWestGravity, Magick::OverCompositeOp)
            assert_same(img1, res)
        end
        img1.freeze
        assert_raise(FreezeError) { img1.composite!(img2, Magick::NorthWestGravity, Magick::OverCompositeOp) }
    end

    def test_composite_affine
        affine = Magick::AffineMatrix.new(1, 0, 1, 0, 0, 0)
        img1 = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        img2 = Magick::Image.read(IMAGES_DIR+'/Button_1.gif').first
        assert_nothing_raised do
            res = img1.composite_affine(img2, affine)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_composite_mathematics
       bg = Magick::Image.new(50, 50)
       fg = Magick::Image.new(50, 50) {self.background_color = "black" }
       res = nil
       assert_nothing_raised { res = bg.composite_mathematics(fg, 1, 0, 0, 0, Magick::CenterGravity) }
       assert_instance_of(Magick::Image, res)
       assert_not_same(bg, res)
       assert_not_same(fg, res)
       assert_nothing_raised { res = bg.composite_mathematics(fg, 1, 0, 0, 0, 0.0, 0.0) }
       assert_nothing_raised { res = bg.composite_mathematics(fg, 1, 0, 0, 0, Magick::CenterGravity, 0.0, 0.0) }

       # too few arguments
       assert_raise(ArgumentError) { bg.composite_mathematics(fg, 1, 0, 0, 0) }
       # too many arguments
       assert_raise(ArgumentError) { bg.composite_mathematics(fg, 1, 0, 0, 0, Magick::CenterGravity, 0.0, 0.0, 'x') }
    end

    def test_composite_tiled
      bg = Magick::Image.new(200,200)
      fg = Magick::Image.new(50,100) { self.background_color = "black" }
      res = nil
      assert_nothing_raised do
        res = bg.composite_tiled(fg)
      end
      assert_instance_of(Magick::Image, res)
      assert_not_same(bg, res)
      assert_not_same(fg, res)
      assert_nothing_raised { bg.composite_tiled!(fg) }
      assert_nothing_raised { bg.composite_tiled(fg, Magick::AtopCompositeOp) }
      assert_nothing_raised { bg.composite_tiled(fg, Magick::OverCompositeOp) }
      assert_nothing_raised { bg.composite_tiled(fg, Magick::RedChannel) }
      assert_nothing_raised { bg.composite_tiled(fg, Magick::RedChannel, Magick::GreenChannel) }

      fg.destroy!
      assert_raise(Magick::DestroyedImageError) { bg.composite_tiled(fg) }
    end

    def test_compress_colormap!
        # DirectClass images are converted to PseudoClass
        assert_equal(Magick::DirectClass, @img.class_type)
        assert_nothing_raised { @img.compress_colormap! }
        assert_equal(Magick::PseudoClass, @img.class_type)
        img = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        assert_equal(Magick::PseudoClass, @img.class_type)
        assert_nothing_raised { @img.compress_colormap! }
    end

    def test_contrast
        assert_nothing_raised do
            res = @img.contrast
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.contrast(true) }
        assert_raise(ArgumentError) { @img.contrast(true, 2) }
    end

    def test_contrast_stretch_channel
        assert_nothing_raised do
            res = @img.contrast_stretch_channel(25)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.contrast_stretch_channel(25, 50) }
        assert_nothing_raised { @img.contrast_stretch_channel('10%') }
        assert_nothing_raised { @img.contrast_stretch_channel('10%', '50%') }
        assert_nothing_raised { @img.contrast_stretch_channel(25, 50, Magick::RedChannel) }
        assert_nothing_raised { @img.contrast_stretch_channel(25, 50, Magick::RedChannel, Magick::GreenChannel) }
        assert_raise(TypeError) { @img.contrast_stretch_channel(25, 50, 'x') }
        assert_raise(ArgumentError) { @img.contrast_stretch_channel }
        assert_raise(ArgumentError) { @img.contrast_stretch_channel('x') }
        assert_raise(ArgumentError) { @img.contrast_stretch_channel(25, 'x') }

    end

    def test_convolve
        kernel = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
        order = 3
        assert_nothing_raised do
            res = @img.convolve(order, kernel)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_raise(ArgumentError) { @img.convolve }
        assert_raise(ArgumentError) { @img.convolve(order) }
        assert_raise(IndexError) { @img.convolve(5, kernel) }
        assert_raise(IndexError) { @img.convolve(order, "x") }
        assert_raise(TypeError) { @img.convolve(3, [1.0, 1.0, 1.0, 1.0, 'x', 1.0, 1.0, 1.0, 1.0]) }
        assert_raise(Magick::ImageMagickError) { @img.convolve(2, [1.0, 1.0, 1.0, 1.0]) }
    end

    def test_convolve_channel
        assert_raise(ArgumentError) { @img.convolve_channel }
        assert_raise(ArgumentError) { @img.convolve_channel(3) }
        kernel = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
        order = 3
        assert_nothing_raised do
            res = @img.convolve_channel(order, kernel, Magick::RedChannel)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end

        assert_nothing_raised { @img.convolve_channel(order, kernel, Magick::RedChannel, Magick:: BlueChannel) }
        assert_raise(TypeError) { @img.convolve_channel(order, kernel, Magick::RedChannel, 2) }
    end

    def test_copy
        assert_nothing_raised do
            ditto = @img.copy
            assert_equal(@img, ditto)
        end
        ditto = @img.copy
        assert_equal(@img.tainted?, ditto.tainted?)
        @img.taint
        ditto = @img.copy
        assert_equal(@img.tainted?, ditto.tainted?)
    end

    def test_crop
        assert_raise(ArgumentError) { @img.crop }
        assert_raise(ArgumentError) { @img.crop(0, 0) }
        assert_nothing_raised do
            res = @img.crop(0, 0, @img.columns/2, @img.rows/2)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        gravity = [
                Magick::NorthEastGravity,
                Magick::EastGravity,
                Magick::SouthWestGravity,
                Magick::SouthGravity,
                Magick::SouthEastGravity]

        # 3-argument form
        gravity.each do |grav|
            assert_nothing_raised { @img.crop(grav, @img.columns/2, @img.rows/2) }
        end
        assert_raise(TypeError) { @img.crop(2, @img.columns/2, @img.rows/2) }
        assert_raise(TypeError) { @img.crop(Magick::NorthWestGravity, @img.columns/2, @img.rows/2, 2) }

        # 4-argument form
        assert_raise(TypeError) { @img.crop(0, 0, @img.columns/2, 'x') }
        assert_raise(TypeError) { @img.crop(0, 0, 'x', @img.rows/2) }
        assert_raise(TypeError) { @img.crop(0, 'x', @img.columns/2, @img.rows/2) }
        assert_raise(TypeError) { @img.crop('x', 0, @img.columns/2, @img.rows/2) }
        assert_raise(TypeError) { @img.crop(0, 0, @img.columns/2, @img.rows/2, 2) }

        # 5-argument form
        gravity.each do |grav|
            assert_nothing_raised { @img.crop(grav, 0, 0, @img.columns/2, @img.rows/2) }
        end

        assert_raise(ArgumentError) { @img.crop(Magick::NorthWestGravity, 0, 0, @img.columns/2, @img.rows/2, 2) }
    end

    def test_crop!
        assert_nothing_raised do
            res = @img.crop!(0, 0, @img.columns/2, @img.rows/2)
            assert_same(@img, res)
        end
    end

    def test_cycle_colormap
        assert_nothing_raised do
            res = @img.cycle_colormap(5)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
            assert_equal(Magick::PseudoClass, res.class_type)
        end
    end

    def test_decipher         # tests encipher, too.
      res = res2 = nil
      assert_nothing_raised do
        res = @img.encipher "passphrase"
        res2 = res.decipher "passphrase"
      end
      assert_instance_of(Magick::Image, res)
      assert_not_same(@img, res)
      assert_equal(@img.columns, res.columns)
      assert_equal(@img.rows, res.rows)
      assert_instance_of(Magick::Image, res2)
      assert_not_same(@img, res2)
      assert_equal(@img.columns, res2.columns)
      assert_equal(@img.rows, res2.rows)
      assert_equal(@img, res2)
    end

    def test_define
      assert_nothing_raised { @img.define("deskew:auto-crop", 40) }
      assert_nothing_raised { @img.undefine("deskew:auto-crop") }
    end

    def test_deskew
       assert_nothing_raised do
        res = @img.deskew
        assert_instance_of(Magick::Image, res)
        assert_not_same(@img, res)
      end

      assert_nothing_raised { @img.deskew(0.10) }
      assert_nothing_raised { @img.deskew("95%") }
      assert_raise(ArgumentError) { @img.deskew("x") }
      assert_raise(TypeError) {@img.deskew(0.40, "x") }
      assert_raise(ArgumentError) {@img.deskew(0.40, 20, [1]) }
    end

    def test_despeckle
        assert_nothing_raised do
            res = @img.despeckle
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    # ensure methods detect destroyed images
    def test_destroy
      methods = Magick::Image.instance_methods(false).sort
      if RUBY_VERSION == '1.9.1'
          methods -= [:__display__, :destroy!, :destroyed?, :inspect, :cur_image, :marshal_load]
      else
          methods -= %w{ __display__ destroy! destroyed? inspect cur_image  marshal_load}
      end

      assert_equal(false, @img.destroyed?)
      @img.destroy!
      assert_equal(true, @img.destroyed?)
      assert_raises(Magick::DestroyedImageError) { @img.check_destroyed }

      methods.each do |method|
          arity = @img.method(method).arity
          method = method.to_s

          case
              when method == "[]="
                  assert_raises(Magick::DestroyedImageError) { @img['foo'] = 1 }
              when method == "difference"
                  other = Magick::Image.new(20,20)
                  assert_raises(Magick::DestroyedImageError) { @img.difference(other) }
              when method == "get_iptc_dataset"
                  assert_raises(Magick::DestroyedImageError) { @img.get_iptc_dataset('x') }
              when method == "profile!"
                  assert_raises(Magick::DestroyedImageError) { @img.profile!('x', 'y') }
              when /=\Z/.match(method)
                  assert_raises(Magick::DestroyedImageError) { @img.send(method, 1) }
              when arity == 0
                  assert_raises(Magick::DestroyedImageError) { @img.send(method) }
              when arity < 0
                  args = (1..(-arity)).to_a
                  assert_raises(Magick::DestroyedImageError) { @img.send(method, *args) }
              when arity > 0
                  args = (1..(arity)).to_a
                  assert_raises(Magick::DestroyedImageError) { @img.send(method, *args) }
              else
                  # Don't know how to test!
                  flunk("don't know how to test method #{method}" )
          end
      end
    end

    # ensure destroy! works
    def test_destroy2
        begin   # ensure Magick.trace_proc gets set to nil even if this test asserts
            images = {}
            GC.disable
            Magick.trace_proc = Proc.new do |which, id, addr, method|
              m = id.split(/ /)
              name = File.basename m[0]
              format = m[1]
              size = m[2]
              geometry = m[3]
              image_class = m[4]

              assert(which == :c || which == :d, "unexpected value for which: #{which}")
              assert_equal(:destroy!, method) if which == :d
              if which == :c
                assert(!images.has_key?(addr), "duplicate image addresses")
                images[addr] = name
              else
                assert(images.has_key?(addr), "destroying image that was not created")
                assert_equal(name, images[addr])
              end
            end
            unmapped = Magick::ImageList.new(IMAGES_DIR+"/Hot_Air_Balloons.jpg", IMAGES_DIR+"/Violin.jpg", IMAGES_DIR+"/Polynesia.jpg")
            map = Magick::ImageList.new "netscape:"
            mapped = unmapped.map map, false
            unmapped.each {|i| i.destroy!}
            map.destroy!
            mapped.each {|i| i.destroy!}
        ensure
            GC.enable
            Magick.trace_proc = nil
        end
    end

    def test_difference
        img1 = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        img2 = Magick::Image.read(IMAGES_DIR+'/Button_1.gif').first
        assert_nothing_raised do
            res = img1.difference(img2)
            assert_instance_of(Array, res)
            assert_equal(3, res.length)
            assert_instance_of(Float, res[0])
            assert_instance_of(Float, res[1])
            assert_instance_of(Float, res[2])
        end

        assert_raise(NoMethodError) { img1.difference(2) }
        img2.destroy!
        assert_raise(Magick::DestroyedImageError) { img1.difference(img2) }
    end

    def test_displace
      @img2 = Magick::Image.new(20,20) {self.background_color = "black"}
      assert_nothing_raised { @img.displace(@img2, 25) }
      res = @img.displace(@img2, 25)
      assert_instance_of(Magick::Image, res)
      assert_not_same(@img, res)
      assert_nothing_raised { @img.displace(@img2, 25, 25) }
      assert_nothing_raised { @img.displace(@img2, 25, 25, 10) }
      assert_nothing_raised { @img.displace(@img2, 25, 25, 10, 10) }
      assert_nothing_raised { @img.displace(@img2, 25, 25, Magick::CenterGravity) }
      assert_nothing_raised { @img.displace(@img2, 25, 25, Magick::CenterGravity, 10) }
      assert_nothing_raised { @img.displace(@img2, 25, 25, Magick::CenterGravity, 10, 10) }
      assert_raise(ArgumentError) { @img.displace }
      assert_raise(TypeError) { @img.displace(@img2, 'x') }
      assert_raise(TypeError) { @img.displace(@img2, 25, []) }
      assert_raise(TypeError) { @img.displace(@img2, 25, 25, 'x') }
      assert_raise(TypeError) { @img.displace(@img2, 25, 25, Magick::CenterGravity, 'x') }
      assert_raise(TypeError) { @img.displace(@img2, 25, 25, Magick::CenterGravity, 10, []) }

      @img2.destroy!
      assert_raise(Magick::DestroyedImageError) { @img.displace(@img2, 25, 25) }
    end

    def test_dissolve
        src = Magick::Image.new(@img.columns, @img.rows)
        src_list = Magick::ImageList.new
        src_list << src.copy
        assert_nothing_raised { @img.dissolve(src, 0.50 ) }
        assert_nothing_raised { @img.dissolve(src_list, 0.50) }
        assert_nothing_raised { @img.dissolve(src, '50%') }
        assert_nothing_raised { @img.dissolve(src, 0.50, 0.10) }
        assert_nothing_raised { @img.dissolve(src, 0.50, 0.10, 10) }
        assert_nothing_raised { @img.dissolve(src, 0.50, 0.10, Magick::NorthEastGravity) }
        assert_nothing_raised { @img.dissolve(src, 0.50, 0.10, Magick::NorthEastGravity, 10) }
        assert_nothing_raised { @img.dissolve(src, 0.50, 0.10, Magick::NorthEastGravity, 10, 10) }

        assert_raise(ArgumentError) { @img.dissolve(src, 'x') }
        assert_raise(ArgumentError) { @img.dissolve(src, 0.50, 'x') }
        assert_raise(TypeError) { @img.dissolve(src, 0.50, Magick::NorthEastGravity, 'x') }
        assert_raise(TypeError) { @img.dissolve(src, 0.50, Magick::NorthEastGravity, 10, 'x') }

        src.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.dissolve(src, 0.50) }
    end

    def test_distort
        @img = Magick::Image.new(200, 200)
        assert_nothing_raised { @img.distort(Magick::AffineDistortion, [2,60, 2,60,     32,60, 32,60,    2,30, 17,35]) }
        assert_nothing_raised { @img.distort(Magick::AffineProjectionDistortion, [1,0,0,1,0,0]) }
        assert_nothing_raised { @img.distort(Magick::BilinearDistortion, [7,40, 4,30,   4,124, 4,123,   85,122, 100,123,   85,2, 100,30]) }
        assert_nothing_raised { @img.distort(Magick::PerspectiveDistortion, [7,40, 4,30,   4,124, 4,123,   85,122, 100,123,   85,2, 100,30]) }
        assert_nothing_raised { @img.distort(Magick::ScaleRotateTranslateDistortion, [28,24,  0.4,0.8  -110,  37.5,60]) }
        assert_raise(ArgumentError) { @img.distort }
        assert_raise(ArgumentError) { @img.distort(Magick::AffineDistortion) }
        assert_raise(TypeError) { @img.distort(1, [1]) }
    end

    def test_distortion_channel
        assert_nothing_raised do
            metric = @img.distortion_channel(@img, Magick::MeanAbsoluteErrorMetric)
            assert_instance_of(Float, metric)
            assert_equal(0.0, metric)
        end
        assert_nothing_raised { @img.distortion_channel(@img, Magick::MeanSquaredErrorMetric) }
        assert_nothing_raised { @img.distortion_channel(@img, Magick::PeakAbsoluteErrorMetric) }
        assert_nothing_raised { @img.distortion_channel(@img, Magick::PeakSignalToNoiseRatioMetric) }
        assert_nothing_raised { @img.distortion_channel(@img, Magick::RootMeanSquaredErrorMetric) }
        assert_nothing_raised { @img.distortion_channel(@img, Magick::MeanSquaredErrorMetric, Magick::RedChannel, Magick:: BlueChannel) }
        assert_raise(TypeError) { @img.distortion_channel(@img, 2) }
        assert_raise(TypeError) { @img.distortion_channel(@img, Magick::RootMeanSquaredErrorMetric, 2) }
        assert_raise(ArgumentError) { @img.distortion_channel }
        assert_raise(ArgumentError) { @img.distortion_channel(@img) }

        img = Magick::Image.new(20,20)
        img.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.distortion_channel(img, Magick::MeanSquaredErrorMetric) }
    end

    def test_dup
        assert_nothing_raised do
            ditto = @img.dup
            assert_equal(@img, ditto)
        end
        ditto = @img.dup
        assert_equal(@img.tainted?, ditto.tainted?)
        @img.taint
        ditto = @img.dup
        assert_equal(@img.tainted?, ditto.tainted?)
    end

    def test_each_profile
        @img.iptc_profile = "test profile"
        assert_nothing_raised do
            @img.each_profile do |name, value|
                assert_equal("iptc", name)
                # As of 6.3.1
                assert_equal("8BIM\004\004\000\000\000\000\001\340test profile", value)
            end
        end
    end

    def test_edge
        assert_nothing_raised do
            res = @img.edge
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.edge(2.0) }
        assert_raise(ArgumentError) { @img.edge(2.0, 2) }
        assert_raise(TypeError) { @img.edge('x') }
    end

    def test_emboss
        assert_nothing_raised do
            res = @img.emboss
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.emboss(1.0) }
        assert_nothing_raised { @img.emboss(1.0, 0.5) }
        assert_raise(ArgumentError) { @img.emboss(1.0, 0.5, 2) }
        assert_raise(TypeError) { @img.emboss(1.0, 'x') }
        assert_raise(TypeError) { @img.emboss('x', 1.0) }
    end

    def test_enhance
        assert_nothing_raised do
            res = @img.enhance
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end


    def test_equalize
        assert_nothing_raised do
            res = @img.equalize
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_equalize_channel
        assert_nothing_raised do
            res = @img.equalize_channel
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.equalize_channel }
        assert_nothing_raised { @img.equalize_channel(Magick::RedChannel) }
        assert_nothing_raised { @img.equalize_channel(Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.equalize_channel(Magick::RedChannel, 2) }
    end

    def test_erase!
        assert_nothing_raised do
            res = @img.erase!
            assert_same(@img, res)
        end
    end

    def test_excerpt
        res = nil
        img = Magick::Image.new(200, 200)
        assert_nothing_raised { res = @img.excerpt(20,20,50,100) }
        assert_not_same(img, res)
        assert_equal(50, res.columns)
        assert_equal(100, res.rows)

        assert_nothing_raised { img.excerpt!(20,20,50,100) }
        assert_equal(50, img.columns)
        assert_equal(100, img.rows)
    end

    def test_export_pixels
        assert_nothing_raised do
            res = @img.export_pixels
            assert_instance_of(Array, res)
            assert_equal(@img.columns*@img.rows*"RGB".length, res.length)
            res.each do |p|
                assert_kind_of(Integer, p)
            end
        end
        assert_nothing_raised { res = @img.export_pixels(0) }
        assert_nothing_raised { res = @img.export_pixels(0, 0) }
        assert_nothing_raised { res = @img.export_pixels(0, 0, 10) }
        assert_nothing_raised { res = @img.export_pixels(0, 0, 10, 10) }
        assert_nothing_raised do
            res = @img.export_pixels(0, 0, 10, 10, 'RGBA')
            assert_equal(10*10*"RGBA".length, res.length)
       end
       assert_nothing_raised do
            res = @img.export_pixels(0, 0, 10, 10, 'I')
            assert_equal(10*10*"I".length, res.length)
       end

       # too many arguments
       assert_raise(ArgumentError) { @img.export_pixels(0, 0, 10, 10, 'I', 2) }

    end

    def test_export_pixels_to_str
        assert_nothing_raised do
            res = @img.export_pixels_to_str
            assert_instance_of(String, res)
            assert_equal(@img.columns*@img.rows*"RGB".length, res.length)
        end
        assert_nothing_raised { @img.export_pixels_to_str(0) }
        assert_nothing_raised { @img.export_pixels_to_str(0, 0) }
        assert_nothing_raised { @img.export_pixels_to_str(0, 0, 10) }
        assert_nothing_raised { @img.export_pixels_to_str(0, 0, 10, 10) }
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "RGBA")
            assert_equal(10*10*"RGBA".length, res.length)
        end
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I")
            assert_equal(10*10*"I".length, res.length)
        end

        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::CharPixel)
            assert_equal(10*10*1, res.length)
        end
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::ShortPixel)
            assert_equal(10*10*2, res.length)
        end
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::IntegerPixel)
            assert_equal(10*10*4, res.length)
        end
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::LongPixel)
            assert_equal(10*10*4, res.length)
        end
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::FloatPixel)
            assert_equal(10*10*4, res.length)
        end
        assert_nothing_raised do
            res = @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::DoublePixel)
            assert_equal(10*10*8, res.length)
        end
        assert_nothing_raised { @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::QuantumPixel) }

        # too many arguments
        assert_raise(ArgumentError) { @img.export_pixels_to_str(0, 0, 10, 10, "I", Magick::QuantumPixel, 1) }
        # last arg s/b StorageType
        assert_raise(TypeError) { @img.export_pixels_to_str(0, 0, 10, 10, "I", 2) }
    end

    def test_extent
      assert_nothing_raised { @img.extent(40, 40) }
      res = @img.extent(40, 40)
      assert_instance_of(Magick::Image, res)
      assert_not_same(@img, res)
      assert_equal(40, res.columns)
      assert_equal(40, res.rows)
      assert_nothing_raised { @img.extent(40, 40, 5) }
      assert_nothing_raised { @img.extent(40, 40, 5, 5) }
      assert_raises(ArgumentError) { @img.extent(-40, 40) }
      assert_raises(ArgumentError) { @img.extent(40, -40) }
      assert_raises(TypeError) { @img.extent('x', 40) }
      assert_raises(TypeError) { @img.extent(40, 'x') }
      assert_raises(TypeError) { @img.extent(40, 40, 'x') }
      assert_raises(TypeError) { @img.extent(40, 40, 5, 'x') }
    end

    def test_find_similar_region
        girl = Magick::Image.read(IMAGES_DIR+"/Flower_Hat.jpg").first
        region = girl.crop(10, 10, 50, 50)
        assert_nothing_raised do
            x, y = girl.find_similar_region(region)
            assert_not_nil(x)
            assert_not_nil(y)
            assert_equal(10, x)
            assert_equal(10, y)
        end
        assert_nothing_raised do
            x, y = girl.find_similar_region(region, 0)
            assert_equal(10, x)
            assert_equal(10, y)
        end
        assert_nothing_raised do
            x, y = girl.find_similar_region(region, 0, 0)
            assert_equal(10, x)
            assert_equal(10, y)
        end

        list = Magick::ImageList.new
        list << region
        assert_nothing_raised do
            x, y = girl.find_similar_region(list, 0, 0)
            assert_equal(10, x)
            assert_equal(10, y)
        end


        x = girl.find_similar_region(@img)
        assert_nil(x)

        assert_raise(ArgumentError) { girl.find_similar_region(region, 10, 10, 10) }
        assert_raise(TypeError) { girl.find_similar_region(region, 10, 'x') }
        assert_raise(TypeError) { girl.find_similar_region(region, 'x') }

        region.destroy!
        assert_raise(Magick::DestroyedImageError) { girl.find_similar_region(region) }

    end

    def test_flip
        assert_nothing_raised do
            res = @img.flip
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_flip!
        assert_nothing_raised do
            res = @img.flip!
            assert_same(@img, res)
        end
    end

    def test_flop
        assert_nothing_raised do
            res = @img.flop
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_flop!
        assert_nothing_raised do
            res = @img.flop!
            assert_same(@img, res)
        end
    end

    def test_frame
        assert_nothing_raised do
            res = @img.frame
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.frame(50) }
        assert_nothing_raised { @img.frame(50, 50) }
        assert_nothing_raised { @img.frame(50, 50, 25) }
        assert_nothing_raised { @img.frame(50, 50, 25, 25) }
        assert_nothing_raised { @img.frame(50, 50, 25, 25, 6) }
        assert_nothing_raised { @img.frame(50, 50, 25, 25, 6, 6) }
        assert_nothing_raised { @img.frame(50, 50, 25, 25, 6, 6, 'red') }
        red = Magick::Pixel.new(Magick::QuantumRange)
        assert_nothing_raised { @img.frame(50, 50, 25, 25, 6, 6, red) }
        assert_raise(TypeError) { @img.frame(50, 50, 25, 25, 6, 6, 2) }
    end

    def test_gamma_channel
        assert_nothing_raised do
            res = @img.gamma_channel(0.8)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_raise(ArgumentError) { @img.gamma_channel }
        assert_nothing_raised { @img.gamma_channel(0.8, Magick::RedChannel) }
        assert_nothing_raised { @img.gamma_channel(0.8, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.gamma_channel(0.8, Magick::RedChannel, 2) }
    end

    def test_function_channel
       img = Magick::Image.read("gradient:") {self.size = "20x600"}
       img = img.first
       img.rotate!(90)
       assert_nothing_raised { img.function_channel Magick::PolynomialFunction, 0.33 }
       assert_nothing_raised { img.function_channel Magick::PolynomialFunction, 4, -1.5 }
       assert_nothing_raised { img.function_channel Magick::PolynomialFunction, 4, -4, 1 }
       assert_nothing_raised { img.function_channel Magick::PolynomialFunction, -25, 53, -36, 8.3, 0.2 }

       assert_nothing_raised { img.function_channel Magick::SinusoidFunction, 1 }
       assert_nothing_raised { img.function_channel Magick::SinusoidFunction, 1, 90 }
       assert_nothing_raised { img.function_channel Magick::SinusoidFunction, 5, 90, 0.25, 0.75 }

       assert_nothing_raised { img.function_channel Magick::ArcsinFunction, 1 }
       assert_nothing_raised { img.function_channel Magick::ArcsinFunction, 0.5 }
       assert_nothing_raised { img.function_channel Magick::ArcsinFunction, 0.4, 0.7 }
       assert_nothing_raised { img.function_channel Magick::ArcsinFunction, 0.5, 0.5, 0.5, 0.5 }

       assert_nothing_raised { img.function_channel Magick::ArctanFunction, 1 }
       assert_nothing_raised { img.function_channel Magick::ArctanFunction, 10, 0.7 }
       assert_nothing_raised { img.function_channel Magick::ArctanFunction, 5, 0.7, 1.2 }
       assert_nothing_raised { img.function_channel Magick::ArctanFunction, 15, 0.7, 0.5, 0.75 }

       # with channel args
       assert_nothing_raised { img.function_channel Magick::PolynomialFunction, 0.33, Magick::RedChannel }
       assert_nothing_raised { img.function_channel Magick::SinusoidFunction, 1, Magick::RedChannel, Magick::BlueChannel }

       # invalid args
       assert_raise(ArgumentError) { img.function_channel }
       assert_raise(TypeError) { img.function_channel 1 }
       assert_raise(ArgumentError) { img.function_channel Magick::PolynomialFunction }
       assert_raise(TypeError) { img.function_channel Magick::PolynomialFunction, [] }
       assert_raise(ArgumentError) { img.function_channel Magick::SinusoidFunction, 5, 90, 0.25, 0.75, 0.1 }
       assert_raise(ArgumentError) { img.function_channel Magick::ArcsinFunction, 0.5, 0.5, 0.5, 0.5, 0.1 }
       assert_raise(ArgumentError) { img.function_channel Magick::ArctanFunction, 15, 0.7, 0.5, 0.75, 0.1 }
    end

    def test_gramma_correct
        assert_raise(ArgumentError) { @img.gamma_correct }
        # All 4 arguments can't default to 1.0
        assert_raise(ArgumentError) { @img.gamma_correct(1.0) }
        assert_nothing_raised do
            res = @img.gamma_correct(0.8)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.gamma_correct(0.8, 0.9) }
        assert_nothing_raised { @img.gamma_correct(0.8, 0.9, 1.0) }
        assert_nothing_raised { @img.gamma_correct(0.8, 0.9, 1.0, 1.1) }
        # too many arguments
        assert_raise(ArgumentError) { @img.gamma_correct(0.8, 0.9, 1.0, 1.1, 2) }
    end

    def test_gaussian_blur
        assert_nothing_raised do
            res = @img.gaussian_blur
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.gaussian_blur(0.0) }
        assert_nothing_raised { @img.gaussian_blur(0.0, 3.0) }
        # sigma must be != 0.0
        assert_raise(ArgumentError) { @img.gaussian_blur(1.0, 0.0) }
        assert_raise(ArgumentError) { @img.gaussian_blur(1.0, 3.0, 2) }
    end

    def test_gaussian_blur_channel
        assert_nothing_raised do
            res = @img.gaussian_blur_channel
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.gaussian_blur_channel(0.0) }
        assert_nothing_raised { @img.gaussian_blur_channel(0.0, 3.0) }
        assert_nothing_raised { @img.gaussian_blur_channel(0.0, 3.0, Magick::RedChannel) }
        assert_nothing_raised { @img.gaussian_blur_channel(0.0, 3.0, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.gaussian_blur_channel(0.0, 3.0, Magick::RedChannel, 2) }
    end

    def test_get_exif_by_entry
        assert_nothing_raised do
            res = @img.get_exif_by_entry
            assert_instance_of(Array, res)
        end
    end

    def test_get_exif_by_number
        assert_nothing_raised do
            res = @img.get_exif_by_number
            assert_instance_of(Hash, res)
        end
    end

    def test_get_pixels
        assert_nothing_raised do
            pixels = @img.get_pixels(0, 0, @img.columns, 1)
            assert_instance_of(Array, pixels)
            assert_equal(@img.columns, pixels.length)
            assert_block do
                pixels.all? { |p| p.is_a? Magick::Pixel }
            end
        end
        assert_raise(RangeError) { @img.get_pixels( 0,  0, -1, 1) }
        assert_raise(RangeError) { @img.get_pixels( 0,  0, @img.columns, -1) }
        assert_raise(RangeError) { @img.get_pixels( 0,  0, @img.columns+1, 1) }
        assert_raise(RangeError) { @img.get_pixels( 0,  0, @img.columns, @img.rows+1) }
    end

    def test_gray?
        gray = Magick::Image.new(20, 20) { self.background_color = 'gray50' }
        assert(gray.gray?)
        red = Magick::Image.new(20, 20) { self.background_color = 'red' }
        assert(!red.gray?)
    end

    def test_histogram?
        assert_nothing_raised { @img.histogram? }
        assert(@img.histogram?)
    end

    def test_implode
        assert_nothing_raised do
            res = @img.implode(0.5)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_import_pixels
        pixels = @img.export_pixels(0, 0, @img.columns, 1, "RGB")
        assert_nothing_raised do
            res = @img.import_pixels(0, 0, @img.columns, 1, "RGB", pixels)
            assert_same(@img, res)
        end
        assert_raise(ArgumentError) { @img.import_pixels }
        assert_raise(ArgumentError) { @img.import_pixels(0) }
        assert_raise(ArgumentError) { @img.import_pixels(0, 0) }
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, @img.columns) }
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, @img.columns, 1) }
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, @img.columns, 1, "RGB") }
        assert_raise(TypeError) { @img.import_pixels('x', 0, @img.columns, 1, "RGB", pixels) }
        assert_raise(TypeError) { @img.import_pixels(0, 'x', @img.columns, 1, "RGB", pixels) }
        assert_raise(TypeError) { @img.import_pixels(0, 0, 'x', 1, "RGB", pixels) }
        assert_raise(TypeError) { @img.import_pixels(0, 0, @img.columns, 'x', "RGB", pixels) }
        assert_raise(TypeError) { @img.import_pixels(0, 0, @img.columns, 1, [2], pixels) }
        assert_raise(ArgumentError) { @img.import_pixels(-1, 0, @img.columns, 1, "RGB", pixels) }
        assert_raise(ArgumentError) { @img.import_pixels(0, -1, @img.columns, 1, "RGB", pixels) }
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, -1, 1, "RGB", pixels) }
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, @img.columns, -1, "RGB", pixels) }

        # pixel array is too small
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, @img.columns, 2, "RGB", pixels) }
        # pixel array doesn't contain a multiple of the map length
        pixels.shift
        assert_raise(ArgumentError) { @img.import_pixels(0, 0, @img.columns, 1, "RGB", pixels) }
    end

    def test_level
        assert_nothing_raised do
            res = @img.level
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.level(0.0) }
        assert_nothing_raised { @img.level(0.0, 1.0) }
        assert_nothing_raised { @img.level(0.0, 1.0, Magick::QuantumRange) }
        assert_raise(ArgumentError) { @img.level(0.0, 1.0, Magick::QuantumRange, 2) }
        assert_raise(ArgumentError) { @img.level('x') }
        assert_raise(ArgumentError) { @img.level(0.0, 'x') }
        assert_raise(ArgumentError) { @img.level(0.0, 1.0, 'x') }
    end

    # Ensure that #level properly swaps old-style arg list
    def test_level2
        img1 = @img.level(10, 2, 200)
        img2 = @img.level(10, 200, 2)
        assert_equal(img2, img1)

        # Ensure that level2 uses new arg order
        img1 = @img.level2(10, 200, 2)
        assert_equal(img2, img1)
    end

    def test_level_channel
        assert_raise(ArgumentError) { @img.level_channel }
        assert_nothing_raised do
            res = @img.level_channel(Magick::RedChannel)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end

        assert_nothing_raised { @img.level_channel(Magick::RedChannel, 0.0) }
        assert_nothing_raised { @img.level_channel(Magick::RedChannel, 0.0, 1.0) }
        assert_nothing_raised { @img.level_channel(Magick::RedChannel, 0.0, 1.0, Magick::QuantumRange) }

        assert_raise(ArgumentError) { @img.level_channel(Magick::RedChannel, 0.0, 1.0, Magick::QuantumRange, 2) }
        assert_raise(TypeError) { @img.level_channel(2) }
        assert_raise(TypeError) { @img.level_channel(Magick::RedChannel, 'x') }
        assert_raise(TypeError) { @img.level_channel(Magick::RedChannel, 0.0, 'x') }
        assert_raise(TypeError) { @img.level_channel(Magick::RedChannel, 0.0, 1.0, 'x') }
    end

    def level_colors
      res = nil
      assert_nothing_raised do
        res = @img.level_colors()
      end
      assert_instance_of(Magick::Image, res)
      assert_not_same(@img, res)

      assert_nothing_raised { @img.level_colors("black") }
      assert_nothing_raised { @img.level_colors("black", Pixel.new(0,0,0)) }
      assert_nothing_raised { @img.level_colors(Pixel.new(0,0,0), Pixel.new(Magick::QuantumRange,Magick::QuantumRange,Magick::QuantumRange)) }
      assert_nothing_raised { @img.level_colors("black", "white") }
      assert_nothing_raised { @img.level_colors("black", "white", false) }
      # too many arguments
      assert_raises(ArgumentError) { @img.level_colors("black", "white", false, 1) }
      # not a pixel or a string
      assert_raises(ArgumentError) { @img.level_colors([]) }
      # not a color name
      assert_raises(ArgumentError) { @img.level_colors("xxx") }
    end

    def levelize_channel
      res = nil
      assert_nothing_raised do
        res = @img.levelize_channel(0, Magick::QuantumRange)
      end
      assert_instance_of(Magick::Image, res)
      assert_not_same(@img, res)

      assert_nothing_raised { @img.levelize_channel(0, Magick::QuantumRange, 0.5) }
      assert_nothing_raised { @img.levelize_channel(0, Magick::QuantumRange, 0.5, Magick::RedChannel) }
      assert_nothing_raised { @img.levelize_channel(0, Magick::QuantumRange, 0.5, Magick::RedChannel, Magick::BlueChannel) }
      # too many arguments
      assert_raise(ArgumentError) { @img.levelize_channel(0, Magick::QuantumRange, 0.5, 1, Magick::RedChannel) }
      # not enough arguments
      assert_raise(ArgumentError) { @img.levelize_channel() }
    end

=begin
    def test_liquid_rescale
      begin
        @img.liquid_rescale(15,15)
      rescue NotImplementedError
        puts "liquid_rescale not implemented."
        return
      end

      res = nil
      assert_nothing_raised do
        res = @img.liquid_rescale(15, 15)
      end
      assert_equal(15, res.columns)
      assert_equal(15, res.rows)
      assert_nothing_raised { @img.liquid_rescale(15, 15, 0, 0) }
      assert_raise(ArgumentError) { @img.liquid_rescale(15) }
      assert_raise(ArgumentError) { @img.liquid_rescale(15, 15, 0, 0, 0) }
      assert_raise(TypeError) { @img.liquid_rescale([], 15) }
      assert_raise(TypeError) { @img.liquid_rescale(15, []) }
      assert_raise(TypeError) { @img.liquid_rescale(15, 15, []) }
      assert_raise(TypeError) { @img.liquid_rescale(15, 15, 0, []) }
    end
=end

    def test_magnify
        assert_nothing_raised do
            res = @img.magnify
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end

        res = @img.magnify!
        assert_same(@img, res)
    end

    def test_map
        map = Magick::Image.read("netscape:").first
        assert_nothing_raised do
            res = @img.map(map)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.map(map, true) }
        assert_raise(NoMethodError) { @img.map(2) }
        assert_raise(ArgumentError) { @img.map(map, true, 2) }
        assert_raise(ArgumentError) { @img.map }
        map.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.map(map, true) }
    end

    def test_marshal
        img =  Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        d = nil
        img2 = nil
        assert_nothing_raised { d = Marshal.dump(img) }
        assert_nothing_raised { img2 = Marshal.load(d) }
        assert_equal(img, img2)
    end

    def test_mask
        cimg = Magick::Image.new(10,10)
        assert_nothing_raised { @img.mask(cimg) }
        res = nil
        assert_nothing_raised { res = @img.mask }
        assert_not_nil(res)
        assert_not_same(cimg, res)
        assert_equal(20, res.columns)
        assert_equal(20, res.rows)

        # mask expects an Image and calls `cur_image'
        assert_raise(NoMethodError) { @img.mask = 2 }

        img = @img.copy.freeze
        assert_raise(FreezeError) { img.mask cimg }

        @img.destroy!
        assert_raise(Magick::DestroyedImageError) { @img.mask cimg }
    end

    def test_matte_fill_to_border
        assert_nothing_raised do
            res = @img.matte_fill_to_border(@img.columns/2, @img.rows/2)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.matte_fill_to_border(@img.columns, @img.rows) }
        assert_raise(ArgumentError) { @img.matte_fill_to_border(@img.columns+1, @img.rows) }
        assert_raise(ArgumentError) { @img.matte_fill_to_border(@img.columns, @img.rows+1) }
    end

    def test_matte_floodfill
        assert_nothing_raised do
            res = @img.matte_floodfill(@img.columns/2, @img.rows/2)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.matte_floodfill(@img.columns, @img.rows) }
        assert_raise(ArgumentError) { @img.matte_floodfill(@img.columns+1, @img.rows) }
        assert_raise(ArgumentError) { @img.matte_floodfill(@img.columns, @img.rows+1) }
    end

    def test_matte_replace
        assert_nothing_raised do
            res = @img.matte_replace(@img.columns/2, @img.rows/2)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_matte_reset!
        assert_nothing_raised do
            res = @img.matte_reset!
            assert_same(@img, res)
        end
    end

    def test_median_filter
        assert_nothing_raised do
            res = @img.median_filter
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.median_filter(0.5) }
        assert_raise(ArgumentError) { @img.median_filter(0.5, 'x') }
        assert_raise(TypeError) { @img.median_filter('x') }
    end

    def test_minify
        assert_nothing_raised do
            res = @img.minify
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end

        res = @img.minify!
        assert_same(@img, res)
    end

    def test_modulate
        assert_nothing_raised do
            res = @img.modulate
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.modulate(0.5) }
        assert_nothing_raised { @img.modulate(0.5, 0.5) }
        assert_nothing_raised { @img.modulate(0.5, 0.5, 0.5) }
        assert_raise(ArgumentError) { @img.modulate(0.5, 0.5, 0.5, 0.5) }
        assert_raise(TypeError) { @img.modulate('x', 0.5, 0.5) }
        assert_raise(TypeError) { @img.modulate(0.5, 'x', 0.5) }
        assert_raise(TypeError) { @img.modulate(0.5, 0.5, 'x') }
    end

    def test_monochrome?
#       assert_block { @img.monochrome? }
        @img.pixel_color(0,0, 'red')
        assert_block { ! @img.monochrome? }
    end

    def test_motion_blur
        assert_nothing_raised do
            res = @img.motion_blur(1.0, 7.0, 180)
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_raise(ArgumentError) { @img.motion_blur(1.0, 0.0, 180) }
        assert_nothing_raised { @img.motion_blur(1.0, -1.0, 180) }
    end

    def test_negate
        assert_nothing_raised do
            res = @img.negate
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.negate(true) }
        assert_raise(ArgumentError) { @img.negate(true, 2) }
    end

    def test_negate_channel
        assert_nothing_raised do
            res = @img.negate_channel
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.negate_channel(true) }
        assert_nothing_raised { @img.negate_channel(true, Magick::RedChannel) }
        assert_nothing_raised { @img.negate_channel(true, Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.negate_channel(true, Magick::RedChannel, 2) }
    end

    def test_normalize
        assert_nothing_raised do
            res = @img.normalize
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
    end

    def test_normalize_channel
        assert_nothing_raised do
            res = @img.normalize_channel
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.normalize_channel(Magick::RedChannel) }
        assert_nothing_raised { @img.normalize_channel(Magick::RedChannel, Magick::BlueChannel) }
        assert_raise(TypeError) { @img.normalize_channel(Magick::RedChannel, 2) }
    end

    def test_oil_paint
        assert_nothing_raised do
            res = @img.oil_paint
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.oil_paint(2.0) }
        assert_raise(ArgumentError) { @img.oil_paint(2.0, 1.0) }
    end

    def test_opaque
        assert_nothing_raised do
            res = @img.opaque('white', 'red')
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        red = Magick::Pixel.new(Magick::QuantumRange)
        blue = Magick::Pixel.new(0, 0, Magick::QuantumRange)
        assert_nothing_raised { @img.opaque(red, blue) }
        assert_raise(TypeError) { @img.opaque(red, 2) }
        assert_raise(TypeError) { @img.opaque(2, blue) }
    end

    def test_opaque_channel
      res = nil
      assert_nothing_raised { res = @img.opaque_channel('white', 'red') }
      assert_not_nil(res)
      assert_instance_of(Magick::Image, res)
      assert_not_same(res, @img)
      assert_nothing_raised { @img.opaque_channel('red', 'blue', true) }
      assert_nothing_raised { @img.opaque_channel('red', 'blue', true, 50) }
      assert_nothing_raised { @img.opaque_channel('red', 'blue', true, 50, Magick::RedChannel) }
      assert_nothing_raised { @img.opaque_channel('red', 'blue', true, 50, Magick::RedChannel, Magick::GreenChannel) }
      assert_nothing_raised do
        @img.opaque_channel('red', 'blue', true, 50, Magick::RedChannel, Magick::GreenChannel, Magick::BlueChannel)
      end

      assert_raise(TypeError) { @img.opaque_channel('red', 'blue', true, 50, 50) }
      assert_raise(TypeError) { @img.opaque_channel('red', 'blue', true, []) }
      assert_raise(ArgumentError) { @img.opaque_channel('red') }
      assert_raise(TypeError) { @img.opaque_channel('red', []) }
    end

    def test_opaque?
        assert_nothing_raised do
            assert_block { @img.opaque? }
        end
        @img.opacity = Magick::TransparentOpacity
        assert_block { ! @img.opaque? }
    end

    def test_ordered_dither
        assert_nothing_raised do
            res = @img.ordered_dither
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.ordered_dither(2) }
        assert_nothing_raised { @img.ordered_dither(3) }
        assert_nothing_raised { @img.ordered_dither(4) }
        assert_raise(ArgumentError) { @img.ordered_dither(5) }
        assert_raise(ArgumentError) { @img.ordered_dither(2, 1) }
    end

    def test_paint_transparent
      res = nil
      assert_nothing_raised { res = @img.paint_transparent("red") }
      assert_not_nil(res)
      assert_instance_of(Magick::Image, res)
      assert_not_same(res, @img)
      assert_nothing_raised { @img.paint_transparent("red", Magick::TransparentOpacity) }
      assert_nothing_raised { @img.paint_transparent("red", Magick::TransparentOpacity, true) }
      assert_nothing_raised { @img.paint_transparent("red", Magick::TransparentOpacity, true, 50) }

      # Too many arguments
      assert_raise(ArgumentError) { @img.paint_transparent("red", Magick::TransparentOpacity, true, 50, 50) }
      # Not enough
      assert_raise(ArgumentError) { @img.paint_transparent() }
      assert_raise(TypeError) { @img.paint_transparent("red", Magick::TransparentOpacity, true, []) }
      assert_raise(TypeError) { @img.paint_transparent("red", "blue") }
      assert_raise(TypeError) { @img.paint_transparent(50) }
    end

    def test_palette?
        img = Magick::Image.read(IMAGES_DIR+'/Flower_Hat.jpg').first
        assert_nothing_raised do
            assert_block { ! img.palette? }
        end
        img = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        assert_block { img.palette? }
    end

    def test_pixel_color
        assert_nothing_raised do
            res = @img.pixel_color(0,0)
            assert_instance_of(Magick::Pixel, res)
        end
        res = @img.pixel_color(0,0)
        assert_equal(@img.background_color, res.to_color)
        res = @img.pixel_color(0, 0, 'red')
        assert_equal('white', res.to_color)
        res = @img.pixel_color(0, 0)
        assert_equal('red', res.to_color)

        blue = Magick::Pixel.new(0, 0, Magick::QuantumRange)
        assert_nothing_raised { @img.pixel_color(0,0, blue) }
        # If args are out-of-bounds return the background color
        img = Magick::Image.new(10, 10) { self.background_color = 'blue' }
        assert_equal('blue', img.pixel_color(50, 50).to_color)
    end

    def test_polaroid
      assert_nothing_raised { @img.polaroid }
      assert_nothing_raised { @img.polaroid(5) }
      assert_instance_of(Magick::Image, @img.polaroid)
      assert_raises(TypeError) { @img.polaroid('x') }
    end

    def test_posterize
        assert_nothing_raised do
            res = @img.posterize
            assert_instance_of(Magick::Image, res)
            assert_not_same(@img, res)
        end
        assert_nothing_raised { @img.posterize(5) }
        assert_nothing_raised { @img.posterize(5, true) }
        assert_raise(ArgumentError) { @img.posterize(5, true, 'x') }
    end

end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
Test::Unit::UI::Console::TestRunner.run(Image2_UT) if RUBY_VERSION != '1.9.1'
end
