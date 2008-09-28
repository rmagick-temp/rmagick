require 'RMagick'

images = Magick::ImageList.new("images/Apple.miff", "images/Rocks_On_Beach.miff", "images/Leaf.miff")
rose = Magick::Image.read("images/Yellow_Rose.miff").first
rose[:Label] = "Remap Image"

result = Magick::ImageList.new
result += images
result << rose

begin
   result += images.copy.affinity(rose)
   montage = result.montage { self.tile = "4x2" }
   montage.alpha Magick::DeactivateAlphaChannel
rescue NotImplementedError
   montage = Magick::Image.read("images/notimplemented.gif").first
end

montage.write("remap_images.jpg")


