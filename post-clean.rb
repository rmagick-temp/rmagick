# delete RMagick documentation created by post-setup.rb
# doc/*.rb.html
# doc/ex/* (!rb)
require 'ftools'

targets = Dir['doc/*.rb.html']
File.safe_unlink(*targets) unless targets.empty?

targets = Dir['doc/ex/*']
targets.delete_if { |entry| !File.file?(entry) || %r{\.rb\z}.match(entry) }
File.safe_unlink(*targets) unless targets.empty?

