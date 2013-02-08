#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner' if !RUBY_VERSION[/^1\.9|^2/]


class Preview_UT < Test::Unit::TestCase

    def test_preview
        preview_types = [
          Magick::RotatePreview,
          Magick::ShearPreview,
          Magick::RollPreview,
          Magick::HuePreview,
          Magick::SaturationPreview,
          Magick::BrightnessPreview,
          Magick::GammaPreview,
          Magick::SpiffPreview,
          Magick::DullPreview,
          Magick::GrayscalePreview,
          Magick::QuantizePreview,
          Magick::DespecklePreview,
          Magick::ReduceNoisePreview,
          Magick::AddNoisePreview,
          Magick::SharpenPreview,
          Magick::BlurPreview,
          Magick::ThresholdPreview,
          Magick::EdgeDetectPreview,
          Magick::SpreadPreview,
          Magick::SolarizePreview,
          Magick::ShadePreview,
          Magick::RaisePreview,
          Magick::SegmentPreview,
          Magick::SwirlPreview,
          Magick::ImplodePreview,
          Magick::WavePreview,
          Magick::OilPaintPreview,
          Magick::CharcoalDrawingPreview,
          Magick::JPEGPreview ]

        hat = Magick::Image.read(IMAGES_DIR+'/Flower_Hat.jpg').first
        assert_nothing_raised do
            prev = hat.preview(Magick::RotatePreview)
            assert_instance_of(Magick::Image, prev)
        end
        puts "\n"
        preview_types.each do |type|
            puts "testing #{type.to_s}..."
            assert_nothing_raised { hat.preview(type) }
        end
        assert_raise(TypeError) { hat.preview(2) }
    end

end


if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
Test::Unit::UI::Console::TestRunner.run(Preview_UT) if !RUBY_VERSION[/^1\.9|^2/]
end

