#! /usr/local/bin/ruby -w
require 'RMagick'

f = Magick::Image.read("images/Flower_Hat.jpg").first

pixels = f.dispatch(0,0,f.columns,f.rows,"RGB")

# Write the pixels to a file, to be included
# in the constitute.rb example.
File.open('pixels-array', 'w') { |txt|
    txt.puts("Width = #{f.columns}")
    txt.puts("Height = #{f.rows}")
    txt.print('Pixels = [')
    pixels = pixels.join(',')
    pixels.gsub!(/(\d+,){1,25}/) { "#{$&}\n" }
    txt.print(pixels)
    txt.puts(']')
}
exit
