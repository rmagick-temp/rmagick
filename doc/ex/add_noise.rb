#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#add_noise method

# Read and scale the Violin.jpg image
violin = Magick::Image.read('images/Violin.jpg').first
violin.scale!(200.0/violin.columns)

# Call add_noise. This returns a copy of the image with noise added.
noisy_violin = violin.add_noise(Magick::MultiplicativeGaussianNoise)

# Crop the new image, leaving just the right half.
noisy_violin.crop!(noisy_violin.columns/2, 0, noisy_violin.columns/2, noisy_violin.rows)

# Composite the new Image over the original image.
result = violin.composite(noisy_violin, Magick::EastGravity, Magick::OverCompositeOp)

# Draw a white line down the middle to emphasize the
# border between the original image and the noisy image.
line = Magick::Draw.new
line.stroke('white')
line.stroke_width(1)
line.line(violin.columns/2, 0, violin.columns/2, violin.rows)
line.draw(result)

#result.display
result.write("add_noise.jpg")
exit
