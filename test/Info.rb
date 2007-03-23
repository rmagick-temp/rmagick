
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'


class Info_UT < Test::Unit::TestCase

    def setup
        @info = Magick::Image::Info.new
    end

    def test_options

        # 1-argument form
        assert_nothing_raised { @info['fill'] }
        assert_nil(@info['fill'])

        assert_nothing_raised { @info['fill'] = 'red' }
        assert_equal("red", @info['fill'])

        assert_nothing_raised { @info['fill'] = nil }
        assert_nil(@info['fill'])

        # 2-argument form
        assert_nothing_raised { @info['tiff', 'bits-per-sample'] = 2 }
        assert_equal("2", @info['tiff', 'bits-per-sample'])

        # define and undefine
        assert_nothing_raised { @info.define('tiff', 'bits-per-sample', 4) }
        assert_equal("4", @info['tiff', 'bits-per-sample'])

        assert_nothing_raised { @info.undefine('tiff', 'bits-per-sample') }
        assert_nil(@info['tiff', 'bits-per-sample'])
    end

    def test_fill
        assert_nothing_raised { @info.fill }
        assert_nil(@info.fill)

        assert_nothing_raised { @info.fill = 'white' }
        assert_equal("white", @info.fill)

        assert_nothing_raised { @info.fill = nil }
        assert_nil(@info.fill)

        assert_raise(ArgumentError) { @info.fill = 'xxx' }
    end

    def test_stroke
        assert_nothing_raised { @info.stroke }
        assert_nil(@info.stroke)

        assert_nothing_raised { @info.stroke = 'white' }
        assert_equal("white", @info.stroke)

        assert_nothing_raised { @info.stroke = nil }
        assert_nil(@info.stroke)

        assert_raise(ArgumentError) { @info.stroke = 'xxx' }
    end

    def test_undercolor
        assert_nothing_raised { @info.undercolor }
        assert_nil(@info.undercolor)

        assert_nothing_raised { @info.undercolor = 'white' }
        assert_equal("white", @info.undercolor)

        assert_nothing_raised { @info.undercolor = nil }
        assert_nil(@info.undercolor)

        assert_raise(ArgumentError) { @info.undercolor = 'xxx' }
    end

end
