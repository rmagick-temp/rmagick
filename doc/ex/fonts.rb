#! /usr/local/bin/ruby -w
require 'RMagick'

Styles = {Magick::NormalStyle => 'NormalStyle',
          Magick::ItalicStyle => 'ItalicStyle',
          Magick::ObliqueStyle => 'ObliqueStyle',
          Magick::AnyStyle => 'AnyStyle'}
Stretches = { Magick::NormalStretch => 'NormalStretch',
              Magick::UltraCondensedStretch => 'UltraCondensedStretch',
              Magick::ExtraCondensedStretch => 'ExtraCondensedStretch',
              Magick::CondensedStretch => 'CondensedStretch',
              Magick::SemiCondensedStretch => 'SemiCondensedStretch',
              Magick::SemiExpandedStretch => 'SemiExpandedStretch',
              Magick::ExpandedStretch => 'ExpandedStretch',
              Magick::ExtraExpandedStretch => 'ExtraExpandedStretch',
              Magick::UltraExpandedStretch => 'UltraExpandedStretch',
              Magick::AnyStretch => 'AnyStretch'}

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
    printf("%-*s %-*s %d %s\t%s\n",
        name_length, font.name,
        family_length, font.family,
        font.weight, Styles[font.style], Stretches[font.stretch])
}
