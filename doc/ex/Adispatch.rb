#! /usr/local/bin/ruby -w
require 'RMagick'

f = Magick::Image.read("images/Flower_Hat.jpg").first

pixels = f.dispatch(0,0,f.columns,f.rows,"RGB")

# Write the pixels to a file, to be included
# in the constitute.rb example.
File.open('pixels-array', 'w') { |txt|
    txt.puts("Width = #{f.columns}")
    txt.puts("Height = #{f.rows}")
    txt.puts('Pixels = [')
    x = 0
    pixels.each do |p|
        txt.printf("%3d,", p)
        x = x.succ
        txt.printf("\n") if x % 25 == 0
    end
    txt.puts(']')
}
exit
