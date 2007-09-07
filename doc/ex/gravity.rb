#! /usr/local/bin/ruby -w
#
#   A RMagick version of Magick++/demo/gravity.cpp
#

require 'RMagick'

x, y = 100, 100

begin

    pic = Magick::ImageList.new

    lines = Magick::Draw.new
    lines.stroke "#600"
    lines.fill_opacity 0
    lines.line 300,100, 300,500
    lines.line 100,300, 500,300
    lines.rectangle 100,100, 500,500

    draw = Magick::Draw.new
    draw.pointsize = 30
    draw.fill = "#600"
    draw.undercolor = "red"

    0.step(330, 30) { |angle|
        puts "angle #{angle}"
        pic.new_image(600, 600) { self.background_color = "white" }

        lines.draw pic

        draw.annotate(pic, 0,0,x,y, "NorthWest") {
            self.gravity = Magick::NorthWestGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,0,y, "North") {
            self.gravity = Magick::NorthGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,x,y, "NorthEast") {
            self.gravity = Magick::NorthEastGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,x,0, "East") {
            self.gravity = Magick::EastGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,0,0, "Center") {
            self.gravity = Magick::CenterGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,x,y, "SouthEast") {
            self.gravity = Magick::SouthEastGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,0,y, "South") {
            self.gravity = Magick::SouthGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,x,y, "SouthWest") {
            self.gravity =  Magick::SouthWestGravity
            self.rotation = angle
        }
        draw.annotate(pic, 0,0,x,0, "West") {
            self.gravity = Magick::WestGravity
            self.rotation = angle
        }
    }

    puts "Writing image \"rm_gravity_out.miff\"..."
    pic.delay = 20
    pic.write "./rm_gravity_out.miff"

rescue
    puts "#{$!} exception raised."
    exit 1
end

exit 0
