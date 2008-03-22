#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'


class Pixel_UT < Test::Unit::TestCase

    def setup
        @pixel = Magick::Pixel.from_color('brown')
    end

    def test_hash
        hash = nil
        assert_nothing_raised { hash = @pixel.hash}
        assert_not_nil(hash)
        assert_equal(-761981696, hash)

        p = Magick::Pixel.new
        assert_equal(0, p.hash)

        p = Magick::Pixel.from_color('red')
        assert_equal(-8388608, p.hash)

        # Pixel.hash sacrifices the last bit of the opacity channel
        p = Magick::Pixel.new(0, 0, 0, 72)
        p2 = Magick::Pixel.new(0, 0, 0, 73)
        assert_not_equal(p, p2)
        assert_equal(p.hash, p2.hash)

    end

    def test_eql?
        p = @pixel
        assert(@pixel.eql?(p))
        p = Magick::Pixel.new
        assert(!@pixel.eql?(p))
    end

    def test_fcmp
      red = Magick::Pixel.from_color('red')
      blue = Magick::Pixel.from_color('blue')
      assert_nothing_raised { red.fcmp(red) }
      assert(red.fcmp(red))
      assert(! red.fcmp(blue) )

      assert_nothing_raised { red.fcmp(blue, 10) }
      assert_nothing_raised { red.fcmp(blue, 10, Magick::RGBColorspace) }
      assert_raises(TypeError) { red.fcmp(blue, 'x') }
      assert_raises(TypeError) { red.fcmp(blue, 10, 'x') }
    end

    def test_from_hsla
      assert_nothing_raised { Magick::Pixel.from_hsla(127, 50, 50) }
      assert_nothing_raised { Magick::Pixel.from_hsla(127, 50, 50, 0) }
      assert_raise(TypeError) { Magick::Pixel.from_hsla([], 50, 50, 0) }
      assert_raise(TypeError) { Magick::Pixel.from_hsla(127, [], 50, 0) }
      assert_raise(TypeError) { Magick::Pixel.from_hsla(127, 50, [], 0) }
      assert_raise(TypeError) { Magick::Pixel.from_hsla(127, 50, 50, []) }
      assert_nothing_raised { @pixel.to_hsla }

      180.times do |h|
        25.times do |s|
          25.times do |l|
            5.times do |a|
              args = [2*h, s+25, l+25, a/5]
              px = Magick::Pixel.from_hsla(*args)
              hsla = px.to_hsla()
              #puts "[#{args.join(', ')}] = [#{hsla.join(', ')}]"
              assert_in_delta(args[0], hsla[0], 0.01, "red expected #{args[0]} got #{hsla[0]}")
              assert_in_delta(args[1], hsla[1], 0.01, "green expected #{args[1]} got #{hsla[1]}")
              assert_in_delta(args[2], hsla[2], 0.01, "blue expected #{args[2]} got #{hsla[2]}")
              assert_in_delta(args[3], hsla[3], 0.005, "alpha expected #{args[3]} got #{hsla[3]}")
            end
          end
        end
      end
    end

    def test_to_color
      assert_nothing_raised { @pixel.to_color(Magick::AllCompliance) }
      assert_nothing_raised { @pixel.to_color(Magick::SVGCompliance) }
      assert_nothing_raised { @pixel.to_color(Magick::X11Compliance) }
      assert_nothing_raised { @pixel.to_color(Magick::XPMCompliance) }
      assert_nothing_raised { @pixel.to_color(Magick::AllCompliance, true) }
      assert_nothing_raised { @pixel.to_color(Magick::AllCompliance, false) }
      assert_nothing_raised { @pixel.to_color(Magick::AllCompliance, false, 8) }
      assert_nothing_raised { @pixel.to_color(Magick::AllCompliance, false, 16) }
      assert_raise(ArgumentError) { @pixel.to_color(Magick::AllCompliance, false, 32) }
      assert_raise(TypeError) { @pixel.to_color(1) }
    end

end
