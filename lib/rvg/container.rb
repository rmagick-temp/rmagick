#--
# $Id: container.rb,v 1.5 2009/02/28 23:52:13 rmagick Exp $
# Copyright (C) 2009 Timothy P. Hunter
#++

module Magick
    class RVG

        # Content is simply an Array with a deep_copy method.
        # When unit-testing, it also has a deep_equal method.
        class Content < Array       #:nodoc:

            def deep_copy(h = {})
                me = self.__id__
                copy = h[me]
                unless copy
                    copy = self.class.new
                    each do |c|
                        copy << case
                            when c.nil?
                                nil
                            when c.respond_to?(:deep_copy)
                                c.deep_copy(h)
                            when c.respond_to?(:dup)
                                c.dup rescue c
                            else
                                c
                            end
                    end
                    copy.freeze if frozen?
                    h[me] = copy
                end
                return copy
            end

        end     # class Content

        # Define a collection of shapes, text, etc. that can be reused.
        # Group objects are _containers_. That is, styles and transforms defined
        # on the group are used by contained objects such as shapes, text, and
        # nested groups unless overridden by a nested container or the object itself.
        # Groups can be reused with the RVG::UseConstructors#use method.
        # Create groups within
        # containers with the RVG::StructureConstructors#g method.
        #
        # Example:
        #   # All elements in the group will be translated by 50 in the
        #   # x-direction and 10 in the y-direction.
        #   rvg.g.translate(50, 10).styles(:stroke=>'red',:fill=>'none') do |grp|
        #       # The line will be red.
        #       grp.line(10,10, 20,20)
        #       # The circle will be blue.
        #       grp.circle(10, 20, 20).styles(:stroke=>'blue')
        #   end
        class Group
            include Stylable
            include Transformable
            include Embellishable
            include Describable
            include Duplicatable

            def initialize
                super
                @content = Content.new
                yield(self) if block_given?
            end

            def add_primitives(gc)          #:nodoc:
                gc.push
                add_transform_primitives(gc)
                add_style_primitives(gc)
                @content.each { |element| element.add_primitives(gc) }
                gc.pop
            end

            # Translate container according to #use arguments
            def ref(x, y, width, height)    #:nodoc:
                translate(x, y) if (x != 0 || y != 0)
            end

            # Append an arbitrary object to the group's content. Called
            # by #use to insert a non-container object into a group.
            def <<(obj)                     #:nodoc:
                @content << obj
            end

        end     # class Group


        # A Use object allows the re-use of RVG and RVG::Group
        # objects within a container. Create a Use object with the
        # RVG::UseConstructors#use method.
        class Use
            include Stylable
            include Transformable
            include Duplicatable

            # In a container, Use objects are created indirectly via the
            # RVG::UseConstructors#use method.
            # The +x+ and +y+ arguments
            # can be used to specify an additional translation for
            # the group. The +width+ and +height+ arguments specify
            # a width and height for referenced RVG objects.
            def initialize(element, x=0, y=0, width=nil, height=nil)
                super()

                # If the element is not a group, defs, symbol, or rvg,
                # wrap a group around it so it can get a transform and
                # possibly a new viewport.
                if ! element.respond_to?(:ref)
                    @element = Group.new
                    @element << element.deep_copy
                else
                    @element = element.deep_copy
                end
                @element.ref(x, y, width, height)
            end

            def add_primitives(gc)      #:nodoc:
                gc.push
                add_transform_primitives(gc)
                add_style_primitives(gc)
                @element.add_primitives(gc)
                gc.pop
            end

        end # class Use

    end # class RVG
end # module Magick

