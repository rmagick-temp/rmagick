#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner' if RUBY_VERSION != '1.9.1'

class ImageList1_UT < Test::Unit::TestCase

    def setup
        @list = Magick::ImageList.new(*FILES[0..9])
        @list2 = Magick::ImageList.new   # intersection is 5..9
        @list2 << @list[5]
        @list2 << @list[6]
        @list2 << @list[7]
        @list2 << @list[8]
        @list2 << @list[9]
    end

    def test_delay
        assert_nothing_raised { @list.delay }
        assert_equal(0, @list.delay)
        assert_nothing_raised { @list.delay = 20 }
        assert_equal(20, @list.delay)
        assert_raise(ArgumentError) { @list.delay = 'x' }
    end

    def test_ticks_per_second
        assert_nothing_raised { @list.ticks_per_second }
        assert_equal(100, @list.ticks_per_second)
        assert_nothing_raised { @list.ticks_per_second = 1000 }
        assert_equal(1000, @list.ticks_per_second)
        assert_raise(ArgumentError) { @list.ticks_per_second = 'x' }
    end

    def test_iterations
        assert_nothing_raised { @list.iterations }
        assert_equal(1, @list.iterations)
        assert_nothing_raised { @list.iterations = 20 }
        assert_equal(20, @list.iterations)
        assert_raise(ArgumentError) { @list.iterations = 'x' }
    end

    # also tests #size
    def test_length
        assert_nothing_raised { @list.length }
        assert_equal(10, @list.length)
        assert_raise(NoMethodError) { @list.length = 2 }
    end

    def test_scene
        assert_nothing_raised { @list.scene }
        assert_equal(9, @list.scene)
        assert_nothing_raised { @list.scene = 0 }
        assert_equal(0, @list.scene)
        assert_nothing_raised { @list.scene = 1 }
        assert_equal(1, @list.scene)
        assert_raise(IndexError) { @list.scene = -1 }
        assert_raise(IndexError) { @list.scene = 1000 }
        assert_raise(IndexError) { @list.scene = nil }

        # allow nil on empty list
        empty_list = Magick::ImageList.new
        assert_nothing_raised { empty_list.scene = nil }
    end

    def test_undef_array_methods
        assert_raise(NoMethodError) { @list.assoc }
        assert_raise(NoMethodError) { @list.flatten }
        assert_raise(NoMethodError) { @list.flatten! }
        assert_raise(NoMethodError) { @list.join }
        assert_raise(NoMethodError) { @list.pack }
        assert_raise(NoMethodError) { @list.rassoc }
    end

    def test_all
        q = nil
        assert_nothing_raised { q = @list.all? { |i| i.class == Magick::Image } }
        assert(q)
    end

    def test_any
        q = nil
        assert_nothing_raised { q = @list.all? { |i| false } }
        assert(!q)
        assert_nothing_raised { q = @list.all? { |i| i.class == Magick::Image } }
        assert(q)
    end

    def test_aref
        assert_nothing_raised { @list[0] }
        assert_instance_of(Magick::Image, @list[0])
        assert_instance_of(Magick::Image, @list[-1])
        assert_instance_of(Magick::ImageList, @list[0,1])
        assert_instance_of(Magick::ImageList, @list[0..2])
        assert_nil(@list[20])
    end

    def test_aset
        img = Magick::Image.new(5,5)
        assert_nothing_raised do
            rv = @list[0] = img
            assert_same(img, rv)
            assert_same(img, @list[0])
            assert_equal(0, @list.scene)
        end

        # replace 2 images with 1
        assert_nothing_raised do
            img = Magick::Image.new(5,5)
            rv = @list[1,2] = img
            assert_same(img, rv)
            assert_equal(9, @list.length)
            assert_same(img, @list[1])
            assert_equal(1, @list.scene)
        end

        # replace 1 image with 2
        assert_nothing_raised do
            img = Magick::Image.new(5,5)
            img2 = Magick::Image.new(5,5)
            ary = [img, img2]
            rv = @list[3,1] = ary
            assert_same(ary, rv)
            assert_equal(10, @list.length)
            assert_same(img, @list[3])
            assert_same(img2, @list[4])
            assert_equal(4, @list.scene)
        end

        assert_nothing_raised do
            img = Magick::Image.new(5,5)
            rv = @list[5..6] = img
            assert_same(img, rv)
            assert_equal(9, @list.length)
            assert_same(img, @list[5])
            assert_equal(5, @list.scene)
        end

        assert_nothing_raised do
            ary = [img, img]
            rv = @list[7..8] = ary
            assert_same(ary, rv)
            assert_equal(9, @list.length)
            assert_same(img, @list[7])
            assert_same(img, @list[8])
            assert_equal(8, @list.scene)
        end

        assert_nothing_raised do
            rv = @list[-1] = img
            assert_same(img, rv)
            assert_equal(9, @list.length)
            assert_same(img, @list[8])
            assert_equal(8, @list.scene)
        end

        assert_raise(ArgumentError) { @list[0] = 1 }
        assert_raise(ArgumentError) { @list[0,1] = [1,2] }
        assert_raise(ArgumentError) { @list[2..3] = 'x' }
    end

    def test_and
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised do
            res = @list & @list2
            assert_instance_of(Magick::ImageList, res)
            assert_not_same(res, @list)
            assert_not_same(res, @list2)
            assert_equal(5, res.length)
            assert_equal(2, res.scene)
            assert_same(cur, res.cur_image)
        end

        # current scene not in the result, set result scene to last image in result
        @list.scene = 2
        assert_nothing_raised do
            res = @list & @list2
            assert_instance_of(Magick::ImageList, res)
            assert_equal(4, res.scene)
        end

        assert_raise(ArgumentError) { @list & 2 }
    end

    def test_at
         assert_nothing_raised do
            cur = @list.cur_image
            img = @list.at(7)
            assert_same(img, @list[7])
            assert_same(cur, @list.cur_image)
            img = @list.at(10)
            assert_nil(img)
            assert_same(cur, @list.cur_image)
            img = @list.at(-1)
            assert_same(img, @list[9])
            assert_same(cur, @list.cur_image)
        end
    end

    def test_star
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised do
            res = @list * 2
            assert_instance_of(Magick::ImageList, res)
            assert_equal(20, res.length)
            assert_not_same(res, @list)
            assert_same(cur, res.cur_image)
        end

        assert_raise(ArgumentError) { @list * 'x' }
    end

    def test_plus
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised do
            res = @list + @list2
            assert_instance_of(Magick::ImageList, res)
            assert_equal(15, res.length)
            assert_not_same(res, @list)
            assert_not_same(res, @list2)
            assert_same(cur, res.cur_image)
        end

        assert_raise(ArgumentError) { @list + [2] }
    end

    def test_minus
        @list.scene = 0
        cur = @list.cur_image
        assert_nothing_raised do
            res = @list - @list2
            assert_instance_of(Magick::ImageList, res)
            assert_equal(5, res.length)
            assert_not_same(res, @list)
            assert_not_same(res, @list2)
            assert_same(cur, res.cur_image)
        end

        # current scene not in result - set result scene to last image in result
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised do
            res = @list - @list2
            assert_instance_of(Magick::ImageList, res)
            assert_equal(5, res.length)
            assert_equal(4, res.scene)
        end
    end

    def test_catenate
        assert_nothing_raised do
            @list2.each { |img| @list << img }
            assert_equal(15, @list.length)
            assert_equal(14, @list.scene)
        end

        assert_raise(ArgumentError) { @list << 2 }
        assert_raise(ArgumentError) { @list << [2] }
    end

    def test_or
        assert_nothing_raised do
            @list.scene = 7
            # The or of these two lists should be the same as @list
            # but not be the *same* list
            res = @list | @list2
            assert_instance_of(Magick::ImageList, res)
            assert_not_same(res, @list)
            assert_not_same(res, @list2)
            assert_equal(res, @list)
        end

        # Try or'ing disjoint lists
        temp_list = Magick::ImageList.new(*FILES[10..14])
        res = @list | temp_list
        assert_instance_of(Magick::ImageList, res)
        assert_equal(15, res.length)
        assert_equal(7, res.scene)

        assert_raise(ArgumentError) { @list | 2 }
        assert_raise(ArgumentError) { @list | [2] }
    end

    def test_clear
        assert_nothing_raised { @list.clear }
        assert_instance_of(Magick::ImageList, @list)
        assert_equal(0, @list.length)
        assert_nil(@list.scene)
    end

    def test_collect
        assert_nothing_raised do
            cur = @list.cur_image
            scene = @list.scene
            res = @list.collect { |img| img.negate }
            assert_instance_of(Magick::ImageList, res)
            assert_not_same(res, @list)
            assert_equal(scene, res.scene)
        end
        assert_nothing_raised do
            scene = @list.scene
            @list.collect! { |img| img.negate }
            assert_instance_of(Magick::ImageList, @list)
            assert_equal(scene, @list.scene)
        end
    end

    def test_compact
        assert_nothing_raised do
            res = @list.compact
            assert_not_same(res, @list)
            assert_equal(res, @list)
        end
        assert_nothing_raised do
            res = @list
            @list.compact!
            assert_instance_of(Magick::ImageList, @list)
            assert_equal(res, @list)
            assert_same(res, @list)
        end
    end

    def test_concat
        assert_nothing_raised do
            res = @list.concat(@list2)
            assert_instance_of(Magick::ImageList, res)
            assert_equal(15, res.length)
            assert_same(res[14], res.cur_image)
        end
        assert_raise(ArgumentError) { res = @list.concat(2) }
        assert_raise(ArgumentError) { res = @list.concat([2]) }
    end

    def test_delete
        assert_nothing_raised do
            cur = @list.cur_image
            img = @list[7]
            assert_same(img, @list.delete(img))
            assert_equal(9, @list.length)
            assert_same(cur, @list.cur_image)

            # Try deleting the current image.
            assert_same(cur, @list.delete(cur))
            assert_same(@list[-1], @list.cur_image)

            assert_raise(ArgumentError) { @list.delete(2) }
            assert_raise(ArgumentError) { @list.delete([2]) }

            # Try deleting something that isn't in the list.
            # Should return the value of the block.
            assert_nothing_raised do
                img = Magick::Image.read(FILES[10]).first
                res = @list.delete(img) { 1 }
                assert_equal(1, res)
            end
        end
    end

    def test_delete_at
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised { @list.delete_at(9) }
        assert_same(cur, @list.cur_image)
        assert_nothing_raised { @list.delete_at(7) }
        assert_same(@list[-1], @list.cur_image)
    end

    def test_delete_if
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised do
            @list.delete_if { |img| img.filename =~ /5/ }
            assert_instance_of(Magick::ImageList, @list)
            assert_equal(9, @list.length)
            assert_same(cur, @list.cur_image)
        end

        # Delete the current image
        assert_nothing_raised do
            @list.delete_if { |img| img.filename =~ /7/ }
            assert_instance_of(Magick::ImageList, @list)
            assert_equal(8, @list.length)
            assert_same(@list[-1], @list.cur_image)
        end
    end

    # defined by Enumerable
    def test_enumerables
      assert_nothing_raised { @list.detect { true } }
      assert_nothing_raised do
        @list.each_with_index { |img, n| assert_instance_of(Magick::Image, img) }
      end
      assert_nothing_raised { @list.entries }
      assert_nothing_raised { @list.include?(@list[0]) }
      assert_nothing_raised { @list.inject(0) { 0 } }
      assert_nothing_raised { @list.max }
      assert_nothing_raised { @list.min }
      assert_nothing_raised { @list.sort }
      assert_nothing_raised { @list.sort_by {|img| img.signature} }
      assert_nothing_raised { @list.zip }
    end

    def test_eql?
      list2 = @list
      assert(@list.eql?(list2))
      list2 = @list.copy
      assert(! @list.eql?(list2))
    end

    def test_fill
        list = @list.copy
        img = list[0].copy
        assert_nothing_raised do
            assert_instance_of(Magick::ImageList, list.fill(img))
        end
        list.each {|el| assert_same(el, img) }

        list = @list.copy
        list.fill(img, 0, 3)
        0.upto(2) {|i| assert_same(img, list[i]) }

        list = @list.copy
        list.fill(img, 4..7)
        4.upto(7) {|i| assert_same(img, list[i]) }

        list = @list.copy
        list.fill { |i| list[i] = img }
        list.each {|el| assert_same(el, img) }

        list = @list.copy
        list.fill(0, 3) { |i| list[i] = img }
        0.upto(2) {|i| assert_same(img, list[i]) }

        assert_raise(ArgumentError) { list.fill('x', 0) }
    end

    def test_find
      assert_nothing_raised { @list.find { true } }
    end

    def find_all
        assert_nothing_raised do
            res = @list.select { |img| img.filename =~ /Button_2/ }
            assert_instance_of(Magick::ImageList, res)
            assert_equal(1, res.length)
            assert_same(res[0], @list[2])
        end
    end

    def test_insert
        assert_nothing_raised do
            @list.scene = 7
            cur = @list.cur_image
            assert_instance_of(Magick::ImageList, @list.insert(1, @list[2]))
            assert_same(cur, @list.cur_image)
            @list.insert(1, @list[2], @list[3], @list[4])
            assert_same(cur, @list.cur_image)
        end

        assert_raise(ArgumentError) { @list.insert(0, 'x') }
        assert_raise(ArgumentError) { @list.insert(0, 'x', 'y') }
    end

    def test_last
        img = Magick::Image.new(5,5)
        @list << img
        img2 = nil
        assert_nothing_raised { img2 = @list.last }
        assert_instance_of(Magick::Image, img2)
        assert_equal(img2, img)
        img2 = Magick::Image.new(5,5)
        @list << img2
        ilist = nil
        assert_nothing_raised { ilist = @list.last(2) }
        assert_instance_of(Magick::ImageList, ilist)
        assert_equal(2, ilist.length)
        assert_equal(1, ilist.scene)
        assert_equal(img, ilist[0])
        assert_equal(img2, ilist[1])
    end

    def test___map__
        img = @list[0]
        assert_nothing_raised do
            @list.__map__ { |x| img }
        end
        assert_instance_of(Magick::ImageList, @list)
        assert_raise(ArgumentError) { @list.__map__ { 2 } }
    end

    def test_map!
        img = @list[0]
        assert_nothing_raised do
            @list.map! { img }
        end
        assert_instance_of(Magick::ImageList, @list)
        assert_raise(ArgumentError) { @list.map! { 2 } }
    end

    if RUBY_VERSION != '1.9.1'
       def test_nitems
         n = nil
         assert_nothing_raised { n = @list.nitems }
         assert_equal(10, n)
       end
    end

    def test_partition
      a = nil
      n = -1
      assert_nothing_raised { a = @list.partition { n += 1; (n&1).zero? } }
      assert_instance_of(Array, a)
      assert_equal(2, a.size)
      assert_instance_of(Magick::ImageList, a[0])
      assert_instance_of(Magick::ImageList, a[1])
      assert_equal(4, a[0].scene)
      assert_equal(5, a[0].length)
      assert_equal(4, a[1].scene)
      assert_equal(5, a[1].length)
    end

    def test_pop
        @list.scene = 8
        cur = @list.cur_image
        last = @list[-1]
        assert_nothing_raised do
            assert_same(last, @list.pop)
            assert_same(cur, @list.cur_image)
        end

        assert_same(cur, @list.pop)
        assert_same(@list[-1], @list.cur_image)
    end

    def test_push
        list = @list
        img1 = @list[0]
        img2 = @list[1]
        assert_nothing_raised { @list.push(img1, img2) }
        assert_same(list, @list)    # push returns self
        assert_same(img2, @list.cur_image)
    end

    def test_reject
        @list.scene = 7
        cur = @list.cur_image
        list = @list
        assert_nothing_raised do
            res = @list.reject {|img| img.filename =~ /Button_9/ }
            assert_equal(9, res.length)
            assert_instance_of(Magick::ImageList, res)
            assert_same(cur, res.cur_image)
        end
        assert_same(list, @list)
        assert_same(cur, @list.cur_image)

        # Omit current image from result list - result cur_image s/b last image
        res = @list.reject {|img| img.filename =~ /Button_7/}
        assert_equal(9, res.length)
        assert_same(res[-1], res.cur_image)
        assert_same(cur, @list.cur_image)
    end

    def test_reject!
        @list.scene = 7
        cur = @list.cur_image
        assert_nothing_raised do
            @list.reject! { |img| img.filename =~ /5/ }
            assert_instance_of(Magick::ImageList, @list)
            assert_equal(9, @list.length)
            assert_same(cur, @list.cur_image)
        end

        # Delete the current image
        assert_nothing_raised do
            @list.reject! { |img| img.filename =~ /7/ }
            assert_instance_of(Magick::ImageList, @list)
            assert_equal(8, @list.length)
            assert_same(@list[-1], @list.cur_image)
        end

        # returns nil if no changes are made
        assert_nil(@list.reject! { false })
    end

    def test_replace1
        # Replace with empty list
        assert_nothing_raised do
            res = @list.replace([])
            assert_same(res, @list)
            assert_equal(0, @list.length)
            assert_nil(@list.scene)
        end

        # Replace empty list with non-empty list
        temp = Magick::ImageList.new
        assert_nothing_raised do
            temp.replace(@list2)
            assert_equal(5, temp.length)
            assert_equal(4, temp.scene)
        end

        # Try to replace with illegal values
        assert_raise(ArgumentError) { @list.replace([1, 2, 3]) }
    end

    def test_replace2
        # Replace with shorter list
        assert_nothing_raised do
            @list.scene = 7
            cur = @list.cur_image
            res = @list.replace(@list2)
            assert_same(res, @list)
            assert_equal(5, @list.length)
            assert_equal(2, @list.scene)
            assert_same(cur, @list.cur_image)
        end
    end

    def test_replace3
        # Replace with longer list
        assert_nothing_raised do
            @list2.scene = 2
            cur = @list2.cur_image
            res = @list2.replace(@list)
            assert_same(res, @list2)
            assert_equal(10, @list2.length)
            assert_equal(7, @list2.scene)
            assert_same(cur, @list2.cur_image)
        end
    end

    def test_reverse
        list = nil
        cur = @list.cur_image
        assert_nothing_raised { list = @list.reverse }
        assert_equal(list.length, @list.length)
        assert_same(cur, @list.cur_image)
    end

    def test_reverse!
        list = @list
        cur = @list.cur_image
        assert_nothing_raised { @list.reverse! }
        assert_same(list, @list)
        assert_same(cur, @list.cur_image)
    end

    # Just validate its existence
    def test_reverse_each
      assert_nothing_raised do
          @list.reverse_each { |img| assert_instance_of(Magick::Image, img) }
      end
    end

    def test_rindex
      img = @list.last
      n = nil
      assert_nothing_raised { n = @list.rindex(img) }
      assert_equal(9, n)
    end

    def test_select
        assert_nothing_raised do
            res = @list.select { |img| img.filename =~ /Button_2/ }
            assert_instance_of(Magick::ImageList, res)
            assert_equal(1, res.length)
            assert_same(res[0], @list[2])
        end
    end

    def test_shift
        assert_nothing_raised do
            @list.scene = 0
            res = @list[0]
            img = @list.shift
            assert_same(res, img)
            assert_equal(8, @list.scene)
        end
        res = @list[0]
        img = @list.shift
        assert_same(res, img)
        assert_equal(7, @list.scene)
    end

    def test_slice
        assert_nothing_raised { @list.slice(0) }
        assert_nothing_raised { @list.slice(-1) }
        assert_nothing_raised { @list.slice(0,1) }
        assert_nothing_raised { @list.slice(0..2) }
        assert_nothing_raised { @list.slice(20) }
    end

    def test_slice!
        @list.scene = 7
        assert_nothing_raised do
            img0 = @list[0]
            img = @list.slice!(0)
            assert_same(img0, img)
            assert_equal(9, @list.length)
            assert_equal(6, @list.scene)
        end
        cur = @list.cur_image
        img = @list.slice!(6)
        assert_same(cur, img)
        assert_equal(8, @list.length)
        assert_equal(7, @list.scene)
        assert_nothing_raised { @list.slice!(-1) }
        assert_nothing_raised { @list.slice!(0,1) }
        assert_nothing_raised { @list.slice!(0..2) }
        assert_nothing_raised { @list.slice!(20) }
    end

    # simply ensure existence
    def test_sort
      assert_nothing_raised { @list.sort }
      assert_nothing_raised { @list.sort! }
    end

    def test_to_a
      a = nil
      assert_nothing_raised { a = @list.to_a }
      assert_instance_of(Array, a)
      assert_equal(10, a.length)
    end

    def test_uniq
        assert_nothing_raised { @list.uniq }
        assert_instance_of(Magick::ImageList, @list.uniq)
        @list[1] = @list[0]
        @list.scene = 7
        list = @list.uniq
        assert_equal(9, list.length)
        assert_equal(6, list.scene)
        assert_equal(7, @list.scene)
        @list[6] = @list[7]
        list = @list.uniq
        assert_equal(8, list.length)
        assert_equal(5, list.scene)
        assert_equal(7, @list.scene)
    end

    def test_uniq!
        assert_nothing_raised do
            assert_nil(@list.uniq!)
        end
        @list[1] = @list[0]
        @list.scene = 7
        cur = @list.cur_image
        list = @list
        @list.uniq!
        assert_same(list, @list)
        assert_same(cur, @list.cur_image)
        assert_equal(6, @list.scene)
        @list[5] = @list[6]
        @list.uniq!
        assert_same(cur, @list.cur_image)
        assert_equal(5, @list.scene)
    end

    def test_unshift
        img = @list[9]
        @list.scene = 7
        @list.unshift(img)
        assert_equal(0, @list.scene)
        assert_raise(ArgumentError) { @list.unshift(2) }
        assert_raise(ArgumentError) { @list.unshift([1,2]) }
    end

    def test_values_at
        ilist = nil
        assert_nothing_raised { ilist = @list.values_at(1,3,5) }
        assert_instance_of(Magick::ImageList, ilist)
        assert_equal(3, ilist.length)
        assert_equal(2, ilist.scene)
    end

    def test_spaceship
        list2 = @list.copy
        assert_equal(@list.scene, list2.scene)
        assert_equal(@list, list2)
        list2.scene = 0
        assert_not_equal(@list, list2)
        list2 = @list.copy
        list2[9] = list2[0]
        assert_not_equal(@list, list2)
        list2 = @list.copy
        list2 << @list[9]
        assert_not_equal(@list, list2)

        assert_raise(TypeError) { @list <=> 2 }
        list = Magick::ImageList.new
        list2 = Magick::ImageList.new
        assert_raise(TypeError) { list2 <=> @list }
        assert_raise(TypeError) { @list <=> list2 }
        assert_nothing_raised(TypeError) { list <=> list2 }
    end

end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif'].sort
Test::Unit::UI::Console::TestRunner.run(ImageList1_UT) if RUBY_VERSION != '1.9.1'
end
