
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'

class Magick_UT < Test::Unit::TestCase

    def test_limit_resources
        cur = new = nil

        assert_nothing_raised {cur = Magick::limit_resource(:memory, 500)}
        assert_equal(792336384, cur)
        assert_nothing_raised {new = Magick::limit_resource("memory")}
        assert_equal(500, new)

        assert_nothing_raised {cur = Magick::limit_resource(:map, 3500)}
        assert_equal(2112897024, cur)
        assert_nothing_raised {new = Magick::limit_resource("map")}
        assert_equal(3500, new)

        assert_nothing_raised {cur = Magick::limit_resource(:disk, 3*1024*1024*1024)}
        assert_equal(4294967295, cur)
        assert_nothing_raised {new = Magick::limit_resource("disk")}
        assert_equal(3221225472, new)

        assert_nothing_raised {cur = Magick::limit_resource(:file, 500)}
        assert_equal(768, cur)
        assert_nothing_raised {new = Magick::limit_resource("file")}
        assert_equal(500, new)

        assert_raise(ArgumentError) { Magick::limit_resource(:xxx) }
        assert_raise(ArgumentError) { Magick::limit_resource("xxx") }
        assert_raise(ArgumentError) { Magick::limit_resource("map", 3500, 2) }
        assert_raise(ArgumentError) { Magick::limit_resource() }

    end
end

if __FILE__ == $0
Test::Unit::UI::Console::TestRunner.run(Magick_UT)
end

