# delete RMagick documentation created by post-setup.rb
# doc/*.rb.html
# doc/ex/* (!rb)
require 'ftools'

Dir.chdir('doc')
targets = Dir['*.rb.html']
File.safe_unlink(*targets) unless targets.empty?

Dir.chdir('ex')
targets = Dir['*']
targets.delete_if { |entry| File.directory?(entry) || %r{\.rb\z}.match(entry) }
File.safe_unlink(*targets) unless targets.empty?
