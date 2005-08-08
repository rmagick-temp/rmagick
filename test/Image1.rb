#! /usr/local/bin/ruby -w

require 'RMagick'
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
    

end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
Test::Unit::UI::Console::TestRunner.run(Image1_UT)
end
