
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'

class Draw_UT < Test::Unit::TestCase

    def setup
        @draw = Magick::Draw.new
    end

    def test_patterns
        img = Magick::Image.new(20,20)
        assert_nothing_raised { @draw.fill_pattern = img }
        assert_nothing_raised { @draw.stroke_pattern = img }

        ilist = Magick::ImageList.new
        ilist << img
        assert_nothing_raised { @draw.fill_pattern = ilist }
        assert_nothing_raised { @draw.stroke_pattern = ilist }

        assert_raise(NoMethodError) { @draw.fill_pattern = 1 }
        assert_raise(NoMethodError) { @draw.stroke_pattern = 1 }
    end
end

if __FILE__ == $0
Test::Unit::UI::Console::TestRunner.run(Draw_UT)
end

