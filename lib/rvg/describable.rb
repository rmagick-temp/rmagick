#--
# $Id: describable.rb,v 1.1 2005/03/12 17:02:00 rmagick Exp $
# Copyright (C) 2005 Timothy P. Hunter
#++

class Magick::RVG

    #--
    # Corresponds to SVG's Description.class
    #++
    # This module defines a number of metadata attributes.
    module Describable

      private

        def initialize(*args, &block)       #:nodoc:
            super
            @title, @desc, @metadata = nil
        end

      public

        # Sets the object description
        attr_writer :desc
        # Sets the object title
        attr_writer :title
        # Sets the object metadata
        attr_writer :metadata

        # Returns the title of this object. The RVG object title is stored as
        # the 'title' property on the image
        def title
            @title.to_s
        end

        # Returns the description of this object. The RVG object description is
        # stored as the 'desc' property on the image
        def desc
            @desc.to_s
        end

        # Returns additional metadata of this object. The RVG object metadata
        # are stored as the 'metadata' property on the image
        def metadata
            @metadata.to_s
        end

    end     # module Describable

end     # class Magick::RVG

