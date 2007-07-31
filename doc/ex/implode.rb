#! /usr/local/bin/ruby -w

require 'RMagick'

img = Magick::Image.read('images/Flower_Hat.jpg').first

legend = Magick::Draw.new
legend.stroke = 'transparent'
legend.fill = 'white'
legend.gravity = Magick::SouthGravity

frames = Magick::ImageList.new

implosion = 0.25
8.times {
    frames << img.implode(implosion)
    legend.annotate(frames, 0,0,10,20, sprintf("% 4.2f", implosion))
    frames.matte = false
    implosion -= 0.10
    }

7.times {
    implosion += 0.10
    frames << img.implode(implosion)
    legend.annotate(frames, 0,0,10,20, sprintf("% 4.2f", implosion))
    frames.matte = false
    }

frames.delay = 10
frames.iterations = 0
puts "Producing animation..."

frames.write("implode.gif")
exit
