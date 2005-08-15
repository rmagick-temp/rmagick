#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'

class Image2_UT < Test::Unit::TestCase
    
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
        assert_raise(TypeError) { img1.composite!(img2, Magick::NorthWestGravity, Magick::OverCompositeOp) }
    end
    
    def test_composite_affine
        affine = Magick::AffineMatrix.new(1, 0, 1, 0, 0, 0)
        img1 = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        img2 = Magick::Image.read(IMAGES_DIR+'/Button_1.gif').first
        assert_nothing_raised do
            res = img1.composite_affine(img2, affine)
            assert_instance_of(Magick::Image, res)
        end
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
        end
        assert_nothing_raised { @img.contrast(true) }
        assert_raise(ArgumentError) { @img.contrast(true, 2) }
    end
    
    def test_convolve
        kernel = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]
        order = 3
        assert_nothing_raised do
            res = @img.convolve(order, kernel)
            assert_instance_of(Magick::Image, res)
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
        end
        
        assert_nothing_raised { @img.convolve_channel(order, kernel, Magick::RedChannel, Magick:: BlueChannel) }
        assert_raise(ArgumentError) { @img.convolve_channel(order, kernel, Magick::RedChannel, 2) }
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
            assert_equal(Magick::PseudoClass, res.class_type)
        end
    end
    
    def test_despeckle
        assert_nothing_raised do
            res = @img.despeckle
            assert_instance_of(Magick::Image, res)
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
                assert_equal("test profile", value)
            end
        end
    end
    
    def test_edge
        assert_nothing_raised do
            res = @img.edge
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.edge(2.0) }
        assert_raise(ArgumentError) { @img.edge(2.0, 2) }
        assert_raise(TypeError) { @img.edge('x') }
    end
    
    def test_emboss
        assert_nothing_raised do
            res = @img.emboss
            assert_instance_of(Magick::Image, res)
        end
        assert_nothing_raised { @img.emboss(1.0) }
        assert_nothing_raised { @img.emboss(1.0, 0.5) }
        assert_raise(ArgumentError) { @img.emboss(1.0, 0.5, 2) }
        assert_raise(TypeError) { @img.emboss(1.0, 'x') }
        assert_raise(TypeError) { @img.emboss('x', 1.0) }
    end
    
    
end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
Test::Unit::UI::Console::TestRunner.run(Image2_UT)
end
