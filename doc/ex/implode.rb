#! /usr/local/bin/ruby -w

require 'RMagick'

jj = Magick::Image.read('images/Jean_Jacket.jpg').first
jj.scale!(250.0/jj.rows)

legend = Magick::Draw.new
legend.stroke = 'transparent'
legend.fill = 'white'
legend.gravity = Magick::SouthGravity

frames = Magick::ImageList.new

implosion = 0.25
8.times {
    frames << jj.implode(implosion)
    legend.annotate(frames, 0,0,10,20, sprintf("% 4.2f", implosion))
    implosion -= 0.10
    }

7.times {
    implosion += 0.10
    frames << jj.implode(implosion)
    legend.annotate(frames, 0,0,10,20, sprintf("% 4.2f", implosion)) 
    }

frames.delay = 10 
frames.iterations = 0
puts "Producing animation..."
#frames.animate
frames.write("implode.gif")
exit
