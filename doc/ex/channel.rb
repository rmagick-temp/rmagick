#! /usr/local/bin/ruby -w

require 'RMagick'
include Magick

img = Image.read('images/Flower_Hat.jpg').first
imgs = ImageList.new

imgs << img
imgs << img.channel(RedChannel)
imgs.cur_image['Label'] = 'RedChannel'
imgs <<  img.channel(GreenChannel)
imgs.cur_image['Label'] = 'GreenChannel'
imgs << img.channel(BlueChannel)
imgs.cur_image['Label'] = 'BlueChannel'

result = imgs.montage {
    self.tile = "2x2"
    self.background_color = 'black'
    self.stroke = 'transparent'
    self.fill = 'white'
    self.geometry = Geometry.new(img.columns/2, img.rows/2, 5, 5)
    }

result.write('channel.jpg')

