
#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner' if RUBY_VERSION != '1.9.1'


module Magick
  def self._tmpnam_
    @@_tmpnam_
  end
end

class Magick::AlphaChannelType
  def self.enumerators
    @@enumerators
  end
end

class Magick::AlignType
  def self.enumerators
    @@enumerators
  end
end

class Magick::AnchorType
  def self.enumerators
    @@enumerators
  end
end


class Magick_UT < Test::Unit::TestCase

    def test_colors
      res = nil
      assert_nothing_raised { res = Magick.colors }
      assert_instance_of(Array, res)
      res.each do |c|
        assert_instance_of(Magick::Color, c)
        assert_instance_of(String, c.name)
        assert_instance_of(Magick::ComplianceType, c.compliance)
        assert_instance_of(Magick::Pixel, c.color)
      end
      Magick.colors {|c| assert_instance_of(Magick::Color, c) }
    end

    # Test a few of the @@enumerator arrays in the Enum subclasses.
    # No need to test all of them.
    def test_enumerators
      ary = nil
      assert_nothing_raised do
        ary = Magick::AlphaChannelType.enumerators
      end
      assert_instance_of(Array, ary)
      assert_equal(11, ary.length)

      assert_nothing_raised do
        ary = Magick::AlignType.enumerators
      end
      assert_instance_of(Array, ary)
      assert_equal(4, ary.length)

      assert_nothing_raised do
        ary = Magick::AnchorType.enumerators
      end
      assert_instance_of(Array, ary)
      assert_equal(3, ary.length)
    end

    def test_fonts
      res = nil
      assert_nothing_raised { res = Magick.fonts }
      assert_instance_of(Array, res)
      res.each do |f|
        assert_instance_of(Magick::Font, f)
        assert_instance_of(String, f.name)
        assert_instance_of(String, f.description) unless f.description.nil?
        assert_instance_of(String, f.family)
        assert_instance_of(Magick::StyleType, f.style)
        assert_instance_of(Magick::StretchType, f.stretch)
        assert_instance_of(Fixnum, f.weight)
        assert_instance_of(String, f.encoding) unless f.encoding.nil?
        assert_instance_of(String, f.foundry) unless f.foundry.nil?
        assert_instance_of(String, f.format) unless f.format.nil?
      end
    Magick.fonts {|f| assert_instance_of(Magick::Font, f) }
    end

    def test_formats
      res = nil
      assert_nothing_raised { res = Magick.formats }
      assert_instance_of(Hash, res)
      res.each do |f, v|
        assert_instance_of(String, f)
        assert_instance_of(String, v)
      end
      Magick.formats.each { |f, v| assert_not_nil(f); assert_not_nil(v) }
    end

    def test_geometry
      g, gs, g2, gs2 = nil, nil, nil, nil
      assert_nothing_raised { g = Magick::Geometry.new() }
      assert_nothing_raised { gs = g.to_s }
      assert_equal("", gs)

      g = Magick::Geometry.new(40)
      gs = g.to_s
      assert_equal("40x", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50)
      gs = g.to_s
      assert_equal("40x50", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 10)
      gs = g.to_s
      assert_equal("40x50+10+0", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 10, -15)
      gs = g.to_s
      assert_equal("40x50+10-15", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 0, 0, Magick::AreaGeometry)
      gs = g.to_s
      assert_equal("40x50@", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 0, 0, Magick::AspectGeometry)
      gs = g.to_s
      assert_equal("40x50!", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 0, 0, Magick::LessGeometry)
      gs = g.to_s
      assert_equal("40x50<", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 0, 0, Magick::GreaterGeometry)
      gs = g.to_s
      assert_equal("40x50>", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 50, 0, 0, Magick::MinimumGeometry)
      gs = g.to_s
      assert_equal("40x50^", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 0, 0, 0, Magick::PercentGeometry)
      gs = g.to_s
      assert_equal("40%", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 60, 0, 0, Magick::PercentGeometry)
      gs = g.to_s
      assert_equal("40%x60%", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 60, 10, 0, Magick::PercentGeometry)
      gs = g.to_s
      assert_equal("40%x60%+10+0", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40, 60, 10, 20, Magick::PercentGeometry)
      gs = g.to_s
      assert_equal("40%x60%+10+20", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40.5, 60.75)
      gs = g.to_s
      assert_equal("40.50x60.75", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(40.5, 60.75, 0, 0, Magick::PercentGeometry)
      gs = g.to_s
      assert_equal("40.50%x60.75%", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(0, 0, 10, 20)
      gs = g.to_s
      assert_equal("+10+20", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      g = Magick::Geometry.new(0, 0, 10)
      gs = g.to_s
      assert_equal("+10+0", gs)

      assert_nothing_raised { g2 = Magick::Geometry.from_s(gs) }
      gs2 = g2.to_s
      assert_equal(gs, gs2)

      # assert behavior with empty string argument
      assert_nothing_raised { g = Magick::Geometry.from_s("") }
      assert_equal("", g.to_s)

      assert_raise(ArgumentError) { Magick::Geometry.new(Magick::AreaGeometry) }
      assert_raise(ArgumentError) { Magick::Geometry.new(40, Magick::AreaGeometry) }
      assert_raise(ArgumentError) { Magick::Geometry.new(40, 20, Magick::AreaGeometry) }
      assert_raise(ArgumentError) { Magick::Geometry.new(40, 20, 10, Magick::AreaGeometry) }

    end

    def test_set_log_event_mask
      assert_nothing_raised { Magick.set_log_event_mask("Module,Coder") }
    end

    def test_set_log_format
      assert_nothing_raised { Magick.set_log_format("format %d%e%f") }
    end

    # put old limits back in place after testing
    def test_limit_resources
        cur = new = nil

        assert_nothing_raised {cur = Magick::limit_resource(:memory, 500)}
        assert_equal(782635008, cur)
        assert_nothing_raised {new = Magick::limit_resource("memory")}
        assert_equal(500, new)
        Magick::limit_resource(:memory, cur)

        assert_nothing_raised {cur = Magick::limit_resource(:map, 3500)}
        assert_equal(2087026688, cur)
        assert_nothing_raised {new = Magick::limit_resource("map")}
        assert_equal(3500, new)
        Magick::limit_resource(:map, cur)

        assert_nothing_raised {cur = Magick::limit_resource(:disk, 3*1024*1024*1024)}
        assert_equal(4294967295, cur)
        assert_nothing_raised {new = Magick::limit_resource("disk")}
        assert_equal(3221225472, new)
        Magick::limit_resource(:disk, cur)

        assert_nothing_raised {cur = Magick::limit_resource(:file, 500)}
        assert_equal(768, cur)
        assert_nothing_raised {new = Magick::limit_resource("file")}
        assert_equal(500, new)
        Magick::limit_resource(:file, cur)

        assert_raise(ArgumentError) { Magick::limit_resource(:xxx) }
        assert_raise(ArgumentError) { Magick::limit_resource("xxx") }
        assert_raise(ArgumentError) { Magick::limit_resource("map", 3500, 2) }
        assert_raise(ArgumentError) { Magick::limit_resource() }

    end

    # test the @@_tmpnam_ class variable
    # the count is incremented by Image::Info#texture=,
    # ImageList::Montage#texture=, and Draw.composite

    def test_tmpnam

      tmpfiles = Dir[ENV["HOME"] + "/tmp/magick*"].length

      texture = Magick::Image.read("granite:") {self.size = "20x20" }.first
      info = Magick::Image::Info.new

      # does not exist at first
      # assert_raise(NameError) { x = Magick._tmpnam_ }
      info.texture = texture

      # now it exists
      assert_nothing_raised { x = Magick._tmpnam_ }
      assert_equal(1, Magick._tmpnam_)

      info.texture = texture
      assert_equal(2, Magick._tmpnam_)

      mon = Magick::ImageList::Montage.new
      mon.texture = texture
      assert_equal(3, Magick._tmpnam_)

      mon.texture = texture
      assert_equal(4, Magick._tmpnam_)

      gc = Magick::Draw.new
      gc.composite(0, 0, 20, 20, texture)
      assert_equal(5, Magick._tmpnam_)

      gc.composite(0, 0, 20, 20, texture)
      assert_equal(6, Magick._tmpnam_)

      tmpfiles2 = Dir[ENV["HOME"] + "/tmp/magick*"].length

      # The 2nd montage texture deletes the first.
      # The 2nd info texture deletes the first.
      # Both composite images are still alive.
      # Therefore only 4 tmp files are left.
      #assert_equal(tmpfiles+4, tmpfiles2)
      # 6.4.1-5 - only 1 tmpfile?
      assert_equal(tmpfiles, tmpfiles2)

    end

    def test_trace_proc
      Magick.trace_proc = lambda do |which, description, id, method|
        assert(which == :c)
        assert_instance_of(String, description)
        assert_instance_of(String, id)
        assert_equal(:initialize, method)
      end
      begin
        img = Magick::Image.new(20,20)
      ensure
        Magick.trace_proc = nil
      end
    end
end

if __FILE__ == $0
Test::Unit::UI::Console::TestRunner.run(Magick_UT) if RUBY_VERSION != '1.9.1'
end

