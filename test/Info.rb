
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'


class Info_UT < Test::Unit::TestCase

    def setup
        @info = Magick::Image::Info.new
    end

    def test_options
        assert_nothing_raised { @info['tiff', 'bits-per-sample'] = 2 }
        assert_equal("2", @info['tiff', 'bits-per-sample'])

        assert_nothing_raised { @info.define('tiff', 'bits-per-sample', 4) }
        assert_equal("4", @info['tiff', 'bits-per-sample'])

        assert_nothing_raised { @info.undefine('tiff', 'bits-per-sample') }
        assert_nil(@info['tiff', 'bits-per-sample'])
    end
end
