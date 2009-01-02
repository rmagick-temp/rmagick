#! /usr/local/bin/ruby -w
require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'  if RUBY_VERSION != '1.9.1'


module Test
    module Unit
        class TestCase
            alias :_old_run_ :run
            def run(result, &blk)
                method_name = RUBY_VERSION == '1.9.1' ? self.name : @method_name
                puts "Running #{method_name}"
                _old_run_(result, &blk)
            end
        end
    end
end


$:.push('.')

IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif'].sort
FLOWER_HAT = IMAGES_DIR+'/Flower_Hat.jpg'

require 'Image1.rb'
require 'Image2.rb'
require 'Image3.rb'
require 'ImageList1.rb'
require 'ImageList2.rb'
require 'Image_attributes.rb'
require 'Import_Export.rb'
require 'Pixel.rb'
require 'Preview.rb'
require 'Info.rb'
require 'Magick.rb'
require 'Draw.rb'

