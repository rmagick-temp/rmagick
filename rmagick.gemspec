require 'date'
Gem::Specification.new do |s|
  s.name = %q{rmagick}
  s.version = "MAJOR.MINOR.TEENY"
  s.date = Date.today.to_s
  s.summary = %q{RMagick is an interface between the Ruby programming language and the ImageMagick and GraphicsMagick image processing libraries.}
  s.description =<<DESCRIPTION
RMagick is an interface between the Ruby programming language and the
ImageMagick and GraphicsMagick image processing libraries.
DESCRIPTION
  s.author = %q{Tim Hunter}
  s.email = %q{rmagick@rubyforge.org}
  s.homepage = %q{http://rubyforge.org/projects/rmagick}
  s.files = Dir.glob('**/*')
  s.require_paths = %w{lib .}
  s.autorequire = %q{RMagick}
  s.rubyforge_project = %q{rmagick}
  s.extensions = %w{configure}
end
