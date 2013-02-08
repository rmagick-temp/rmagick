
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'  if !RUBY_VERSION[/^1\.9|^2/]

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

    def test_kerning
        assert_nothing_raised { @draw.kerning = 1 }
        assert_nothing_raised { @draw.kerning(1) }
        assert_raise(ArgumentError) { @draw.kerning("a") }
        assert_raise(TypeError) { @draw.kerning([]) }
    end

    def test_interline_spacing
        assert_nothing_raised { @draw.interline_spacing = 1 }
        assert_nothing_raised { @draw.interline_spacing(1) }
        assert_raise(ArgumentError) { @draw.interline_spacing("a") }
        assert_raise(TypeError) { @draw.interline_spacing([]) }
    end

    def test_interword_spacing
        assert_nothing_raised { @draw.interword_spacing = 1 }
        assert_nothing_raised { @draw.interword_spacing(1) }
        assert_raise(ArgumentError) { @draw.interword_spacing("a") }
        assert_raise(TypeError) { @draw.interword_spacing([]) }
    end

    def assert_marshal
       rose = Magick::Image.read("rose:").first
       granite = Magick::Image.read("granite:").first
       s = granite.to_blob {self.format="miff"}
       granite = Magick::Image.from_blob(s).first
       blue_stroke = Magick::Image.new(20,20) {self.background_color = "blue"}
       s = blue_stroke.to_blob {self.format="miff"}
       blue_stroke = Magick::Image.from_blob(s).first

       @draw.affine = Magick::AffineMatrix.new(1, 2, 3, 4, 5, 6)
       @draw.decorate = Magick::LineThroughDecoration
       @draw.encoding = "AdobeCustom"
       @draw.gravity = Magick::CenterGravity
       @draw.fill = Magick::Pixel.from_color("red")
       @draw.stroke = Magick::Pixel.from_color("blue")
       @draw.stroke_width = 5
       @draw.fill_pattern = granite
       @draw.stroke_pattern = blue_stroke
       @draw.text_antialias = true
       @draw.font = "Arial-Bold"
       @draw.font_family = "arial"
       @draw.font_style = Magick::ItalicStyle
       @draw.font_stretch = Magick::CondensedStretch
       @draw.font_weight = Magick::BoldWeight
       @draw.pointsize = 12
       @draw.density = "72x72"
       @draw.align = Magick::CenterAlign
       @draw.undercolor = Magick::Pixel.from_color("green")
       @draw.kerning = 10.5
       @draw.interword_spacing = 3.75

       @draw.circle(20, 25, 20, 28)
       dumped = nil
       assert_nothing_raised { dumped = Marshal.dump(@draw) }
       assert_nothing_raised { Marshal.load(dumped) }
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
Test::Unit::UI::Console::TestRunner.run(Draw_UT)  if !RUBY_VERSION[/^1\.9|^2/]
end

