#! /usr/local/bin/ruby -w
require 'RMagick'

# Compute column widths
name_length = 0
family_length = 0
Magick::fonts { |font|
    if font.name.length > name_length
        name_length = font.name.length
    end
    if font.family.length > family_length
        family_length = font.family.length
    end
}

# Print all fonts
Magick::fonts { |font|
    printf("%-*s %-*s %d %s\t%s\n", name_length, font.name,
        family_length, font.family, font.weight, font.style, font.stretch)
}
