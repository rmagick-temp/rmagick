#! /usr/local/bin/ruby -w
require 'RMagick'

# Demonstrate the Image#oil_paint method

img = Magick::Image.read('images/Grandma.jpg').first
img.scale!(350.0/img.rows)

# oil_paint is a slow method. Track how far along it is.
monitor = Proc.new { |text, quantum, span|
    printf("%s %3.0f%% complete\n", text, ((quantum/span.to_f)*100.0))
    }
Magick.set_monitor(monitor)

img = img.oil_paint
Magick.set_monitor(nil)
#img.display
img.write('oil_paint.jpg')
exit
