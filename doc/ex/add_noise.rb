#! /usr/local/bin/ruby -w
require 'RMagick'
include Magick

# Demonstrate the Image#add_noise method
NOISE_TYPES = [UniformNoise, GaussianNoise,
               MultiplicativeGaussianNoise,
               ImpulseNoise, LaplacianNoise,
               PoissonNoise]

img = Magick::Image.read('images/Flower_Hat.jpg').first

NOISE_TYPES.each do |noise|
    copy = img.add_noise(noise)
    copy.write "add_noise_#{noise.to_s}.jpg"

end
exit
