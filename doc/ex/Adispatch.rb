#! /usr/local/bin/ruby -w
require 'RMagick'

f = Magick::Image.read("images/Flower_Hat.jpg").first

pixels = f.dispatch(0,0,f.columns,f.rows,"RGB")

# Write the pixels to a file, to be loaded in the Zconstitute.rb
# example.  Ruby 1.6.8 # loads the Pixels array much faster if we break
# the array into many small pieces and concatenate them together, so this
# program generates such a sequence.

first = true
total = pixels.length

File.open('pixels-array', 'w') { |txt|
    txt.puts("Width = #{f.columns}")
    txt.puts("Height = #{f.rows}")
    txt.puts('Pixels = [')
    x = 0
    pixels.each do |p|
        txt.printf("%3d,", p)
        x = x.succ
        txt.printf("\n") if x % 25 == 0
        if x % 1000 == 0
            if first
                txt.puts(']')
                first = false
            else
                txt.puts('])')
            end
            txt.print('Pixels.concat([')
        end
    end

    if first
        txt.puts(']')
        first = false
    else
        txt.puts('])')
    end
}
exit
