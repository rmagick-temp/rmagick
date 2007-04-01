require 'date'
Gem::Specification.new do |s|
  s.name = %q{rmagick}
  s.version = "0.0.0"
  s.date = Date.today.to_s
  s.summary = %q{Ruby binding to ImageMagick}
  s.description = %q{RMagick is an interface between Ruby and ImageMagick.}
  s.author = %q{Tim Hunter}
  s.email = %q{rmagick@rubyforge.org}
  s.homepage = %q{http://rubyforge.org/projects/rmagick}
  s.files = Dir.glob('**/*')
  s.require_paths = %w{lib .}
  s.rubyforge_project = %q{rmagick}
  s.extensions = %w{ext/RMagick/extconf.rb}
  s.has_rdoc = false
  s.required_ruby_version = '>= 1.8.2'
  s.requirements << 'ImageMagick 6.2.6 or later'
end
