require 'RMagick'

# Demonstrate the trim method

# Read the Flower_Hat image and reduce it to 80% of its original size.
img = Magick::Image.read('images/Flower_Hat.jpg').first
cols = img.columns
rows = img.rows
img.resize!(0.80)

# Add a gray border to bring it back up to size
before = img.border((cols-img.columns)/2, (rows-img.rows)/2, '#999')

# Trim away the gray border
after = before.trim

# Add a white border to bring it back up to size
after.border!((cols-img.columns)/2, (rows-img.rows)/2, 'white')

# Need GIF for tranparency
before.write('trim_before.jpg')
after.write('trim_after.jpg')
exit

