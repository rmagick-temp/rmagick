#! /usr/local/bin/ruby -w
require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'

$:.push('.')

IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
FLOWER_HAT = IMAGES_DIR+'/Flower_Hat.jpg'


require 'Image1.rb'
require 'Image2.rb'
require 'ImageList1.rb'
require 'ImageList2.rb'
require 'Image_attributes.rb'
