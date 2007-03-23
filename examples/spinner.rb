# Make a 32x32 animated GIF that resembles the OS X animation.
# This produces a very small (~2.6Kb) GIF file.

require 'RMagick'

puts <<END_INFO
This example creates an animated GIF that resembles the OS X "waiting" icon.
You can view the GIF with the command

    animate spinner.gif
END_INFO


NFRAMES = 12                    # number of frames in the animation
DIM = 32                        # width & height of image in pixels
DELAY = 100.0 / (NFRAMES/2)     # 2 rotations per second

# 'frame' is a single frame in the animation.
frame = Magick::Image.new(DIM, DIM) {self.background_color = 'transparent'}

# 'spinner' will contain the frames that make up the animated GIF
spinner = Magick::ImageList.new

# 'level' is the change from darkest gray to white
level = Magick::QuantumRange / 2 / NFRAMES

NFRAMES.times do |x|
    gc = Magick::Draw.new
    gc.translate(DIM/2, DIM/2)
    gc.stroke('none')
    intensity = 0.58 * Magick::QuantumRange
    fill_color = Magick::Pixel.new(intensity, intensity, intensity).to_color
    gc.fill(fill_color)
    angle = x * (360/NFRAMES)
    NFRAMES.times do
        gc.rotate(angle)
        intensity = [intensity, Magick::QuantumRange].min
        fill_color = Magick::Pixel.new(intensity, intensity, intensity).to_color
        gc.fill(fill_color)
        gc.roundrectangle(7, -1, 15, 0, 1, 1)
        angle = -(360/NFRAMES)
        intensity += level
    end
    spinner << frame.copy
    gc.draw(spinner)
end

spinner.delay = DELAY
spinner.compression = Magick::LZWCompression
spinner.write('spinner.gif')