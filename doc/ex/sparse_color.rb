require "RMagick"

def draw_centers(img, all_four = true)
   gc = Magick::Draw.new
   gc.fill("white")
   gc.stroke("black")
   gc.circle(30, 10, 30, 12)
   gc.circle(10, 80, 10, 82)
   gc.circle(70, 60, 70, 62)
   gc.circle(80, 20, 80, 22) if all_four
   gc.draw(img)
   img
end

imgl = Magick::ImageList.new
img = Magick::Image.new(100, 100)

begin

  img2 = img.sparse_color(Magick::VoronoiColorInterpolate, 30, 10, "red",
                             10, 80, "blue", 70, 60, "lime", 80, 20, "yellow")
  img2["Label"] = "Voroni"
  imgl << draw_centers(img2)

  img2 = img.sparse_color(Magick::ShepardsColorInterpolate, 30, 10, "red",
                             10, 80, "blue", 70, 60, "lime", 80, 20, "yellow")
  img2["Label"] = "Shepards"
  imgl << draw_centers(img2)

  img2 = img.sparse_color(Magick::BilinearColorInterpolate, 30, 10, "red",
                             10, 80, "blue", 70, 60, "lime", 80, 20, "yellow")
  img2["Label"] = "Bilinear"
  imgl << draw_centers(img2)

  img2 = img.sparse_color(Magick::BarycentricColorInterpolate, 30, 10, "red",
                             10, 80, "blue", 70, 60, "lime")
  img2["Label"] = "Barycentric"
  imgl << draw_centers(img2, false)

  montage = imgl.montage do
              self.background_color = "none"
              self.geometry = "100x100+10+10"
              self.tile = "2x2"
            end

  montage.write("sparse_color.png")

rescue NotImplementedError, NameError

  img = Magick::Image.read("images/notimplemented.gif").first
  img.resize!(240, 272)
  img.write("sparse_color.png")

end

