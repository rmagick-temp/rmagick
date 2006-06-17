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
  s.rubyforge_project = %q{rmagick}
  s.extensions = %w{gem_extconf.rb}
  s.has_rdoc = false
  s.required_ruby_version = '>= 1.6.7'
  s.requirements << 'ImageMagick 6.0.0 or later, or GraphicsMagick 1.0.0 or later'
end
