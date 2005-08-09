#! /usr/local/bin/ruby -w

require 'RMagick'
require 'base64'
require 'test/unit'
require 'test/unit/ui/console/testrunner'

class Image1_UT < Test::Unit::TestCase
    
    def setup
        @img = Magick::Image.new(20, 20)
    end
    
    # Test [], []=, and #properties
    def test_properties
        assert_nothing_raised do
            @img['a'] = 'string1'
            @img['b'] = 'string2'
            @img['c'] = 'string3'
        end
        assert_equal('string1', @img['a'])
        assert_equal('string2', @img['b'])
        assert_equal('string3', @img['c'])
        assert_nil(@img['d'])
        assert_nothing_raised do
            props = @img.properties
            assert_equal(3, props.length)
            assert_equal('string1', props['a'])
            assert_equal('string2', props['b'])
            assert_equal('string3', props['c'])
        end
        
        known = {'a'=>'string1', 'b'=>'string2', 'c'=>'string3'}
        @img.properties do |name, value|
            assert(known.has_key?(name))
            assert_equal(known[name], value)
        end
    end
    
    # test constitute and dispatch
    def test_constitute
        @img = Magick::Image.read(IMAGES_DIR+'/Button_0.gif')[0]
        assert_nothing_raised do
            pixels = @img.dispatch(0, 0, @img.columns, @img.rows, 'RGBA')
            res = Magick::Image.constitute(@img.columns, @img.rows, 'RGBA', pixels)
            # The constituted image is in MIFF format so we
            # can't compare it directly to the original image.
            assert_equal(@img.columns, res.columns)
            assert_equal(@img.rows, res.rows)
            assert_block { pixels.all? { |v| 0 <= v && v <= Magick::MaxRGB } }
        end
    
        pixels = @img.dispatch(0, 0, @img.columns, @img.rows, 'RGBA', true)
        assert_block { pixels.all? { |v| 0.0 <= v && v <= 1.0 } }
        
        # dispatch wants exactly 5 or exactly 6 arguments
        assert_raise(ArgumentError) { @img.dispatch }
        assert_raise(ArgumentError) { @img.dispatch(0) }
        assert_raise(ArgumentError) { @img.dispatch(0, 0) }
        assert_raise(ArgumentError) { @img.dispatch(0, 0, @img.columns) }
        assert_raise(ArgumentError) { @img.dispatch(0, 0, @img.columns, @img.rows) }
        assert_raise(ArgumentError) { @img.dispatch(0, 0, 20, 20, 'RGBA', false, false) }
    end
    
    # test from_blob and to_blob
    def test_from_blob
        img = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        blob = nil
        res = nil
        assert_nothing_raised { blob = img.to_blob }
        assert_instance_of(String, blob)
        assert_nothing_raised { res = Magick::Image.from_blob(blob) }
        assert_instance_of(Array, res)
        assert_instance_of(Magick::Image, res[0])
        assert_equal(img, res[0])
    end
    
    def test_ping
        res = Magick::Image.ping(IMAGES_DIR+'/Button_0.gif')
        assert_instance_of(Array, res)
        assert_instance_of(Magick::Image, res[0])
        assert_equal('GIF', res[0].format)
        assert_equal(127, res[0].columns)
        assert_equal(120, res[0].rows)
        assert_match(/Button_0.gif/, res[0].filename)
    end
    
    def test_read_inline
        img = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        blob = img.to_blob
        encoded = Base64.encode64(blob)
        res = Magick::Image.read_inline(encoded)
        assert_instance_of(Array, res)
        assert_instance_of(Magick::Image, res[0])
        assert_equal(img, res[0])
    end
    
    def test_spaceship
        img0 = Magick::Image.read(IMAGES_DIR+'/Button_0.gif').first
        img1 = Magick::Image.read(IMAGES_DIR+'/Button_1.gif').first
        sig0 = img0.signature
        sig1 = img1.signature
        # since <=> is based on the signature, the images should
        # have the same relationship to each other as their
        # signatures have to each other.
        assert_equal(sig0 <=> sig1, img0 <=> img1)
        assert_equal(sig1 <=> sig0, img1 <=> img0)
        assert_equal(img0, img0)
        assert_not_equal(img0, img1)
    end
    
    def test_adaptive_threshold
        assert_nothing_raised { @img.adaptive_threshold }
        assert_nothing_raised { @img.adaptive_threshold(2) }
        assert_nothing_raised { @img.adaptive_threshold(2,4) }
        assert_nothing_raised { @img.adaptive_threshold(2,4,1) }
        assert_raise(ArgumentError) { @img.adaptive_threshold(2,4,1,2) }
    end
    
    def test_add_noise
        assert_nothing_raised { @img.add_noise(Magick::UniformNoise) }
        assert_nothing_raised { @img.add_noise(Magick::GaussianNoise) }
        assert_nothing_raised { @img.add_noise(Magick::MultiplicativeGaussianNoise) }
        assert_nothing_raised { @img.add_noise(Magick::ImpulseNoise) }
        assert_nothing_raised { @img.add_noise(Magick::LaplacianNoise) }
        assert_nothing_raised { @img.add_noise(Magick::PoissonNoise) }
        assert_raise(TypeError) { @img.add_noise(0) }
    end
    
    def test_affine_matrix
        affine = Magick::AffineMatrix.new(1, Math::PI/6, Math::PI/6, 1, 0, 0)
        assert_nothing_raised { @img.affine_transform(affine) }
        assert_raise(TypeError) { @img.affine_transform(0) }
    end
    
end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
Test::Unit::UI::Console::TestRunner.run(Image1_UT)
end
