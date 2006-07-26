require 'RMagick'

#   The Image#find_similar_region searches for a region in the image
#   similar to the target. This example uses a rectangle from the image
#   as the target, assuring that find_similar_region will succeed.

#   Draw a red rectangle over the image that shows where the target matched.

img = Magick::Image.read('../doc/ex/images/Flower_Hat.jpg').first
target = img.crop(21, 94, 118, 126)

begin
    res = img.find_similar_region(target)
    if res
        gc = Magick::Draw.new
        gc.stroke('red')
        gc.stroke_width(2)
        gc.fill('none')
        gc.rectangle(res[0], res[1], res[0]+target.columns, res[1]+target.rows)
        gc.draw(img)
        img.matte = false
        puts "Found similar region. Writing `find_similar_region.gif'..."
        img.write('find_similar_region.gif')
    else
        puts "No match!"
    end
rescue NotImplementedError
    $stderr.puts <<-END_MSG
    The find_similar_region method is not supported by this version of
    ImageMagick/GraphicsMagick.
    END_MSG
end

exit
