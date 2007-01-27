#! /usr/local/bin/ruby -w
require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'


module Test
    module Unit
        class TestCase
            alias :_old_run_ :run
            def run(result, &blk)
                puts "Running #{@method_name}"
                _old_run_(result, &blk)
            end
        end
    end
end


$:.push('.')

IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
FLOWER_HAT = IMAGES_DIR+'/Flower_Hat.jpg'


require 'Image1.rb'
require 'Image2.rb'
require 'Image3.rb'
require 'ImageList1.rb'
require 'ImageList2.rb'
require 'Image_attributes.rb'
require 'Pixel.rb'
require 'Info.rb'
require 'Magick.rb'
