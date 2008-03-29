
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'

class Magick::Draw
  def self._dummy_img_
    @@_dummy_img_
  end
end

class Draw_UT < Test::Unit::TestCase

    def setup
        @draw = Magick::Draw.new
    end

    # Ensure @@_dummy_img_ class var is working properly
    def test_dummy_img
      # initially this variable is not defined.
      assert_raise(NameError) do
        Magick::Draw._dummy_img_
      end

      # cause it to become defined. save the object id.
      @draw.get_type_metrics("ABCDEF")
      dummy = nil
      assert_nothing_raised do
        dummy = Magick::Draw._dummy_img_
      end

      assert_instance_of(Magick::Image, dummy)

      # ensure that it is always the same object
      @draw.get_type_metrics("ABCDEF")
      dummy2 = nil
      assert_nothing_raised do
        dummy2 = Magick::Draw._dummy_img_
      end
      assert_same(dummy, dummy2)
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

