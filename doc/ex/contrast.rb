#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#contrast method

dog = Magick::ImageList.new('images/Dog2.jpg')
dog[0] = dog.scale(250.0/dog.rows)

# Prepare to label each image with a number from 1 to 4
legend = Magick::Draw.new
legend.font_family = 'Helvetica'
legend.stroke = 'transparent'
legend.gravity = Magick::SouthWestGravity

# Add 3 images, each one having slightly less contrast
f = 1
3.times {
    dog << dog.contrast

    # Annotate the previous image
    legend.annotate(dog[f-1], 0,0,10,20, f.to_s)
    f += 1
    }

# Annotate the last image
legend.annotate(dog, 0,0,10,20, f.to_s)

# Montage into a single image
dogs = dog.montage {
    self.geometry = "#{dog.columns}x250+0+0"
    self.tile = "2x2"
    }
dogs.write('contrast.jpg')
#dogs.display
exit
