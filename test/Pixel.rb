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

end
