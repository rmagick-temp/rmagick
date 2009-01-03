#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first
imgs = Magick::ImageList.new

imgs << img
imgs << img.channel(Magick::RedChannel)
imgs.cur_image['Label'] = 'RedChannel'
imgs <<  img.channel(Magick::GreenChannel)
imgs.cur_image['Label'] = 'GreenChannel'
imgs << img.channel(Magick::BlueChannel)
imgs.cur_image['Label'] = 'BlueChannel'

result = imgs.montage {
    self.tile = "2x2"
    self.background_color = 'black'
    self.stroke = 'transparent'
    self.fill = 'white'
    self.pointsize =9
    self.geometry = Magick::Geometry.new(img.columns/2, img.rows/2, 5, 5)
    }

result.write('channel.jpg')

