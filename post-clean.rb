# delete RMagick documentation created by post-setup.rb
# doc/*.rb.html
# doc/ex/* (!rb)
# Bug #246: Don't use chdir!
require 'fileutils'

targets = Dir['doc/*.rb.html']
FileUtils.safe_unlink(targets) unless targets.empty?

targets = Dir['doc/ex/*']
targets.delete_if { |entry| File.directory?(entry) || %r{\.rb\z}.match(entry) }
FileUtils.safe_unlink(targets) unless targets.empty?
