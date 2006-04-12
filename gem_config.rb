require 'English'

#   gem_config.rb - This script is executed by the gem command
#   to generate the RMagick Makefile. Ref: rmagick.gemspec

#   The gem command calls us with some extra arguments we don't use.
#   Filter out anything that doesn't look like a configure option.
args = []
ARGV.each { |arg| args << arg if arg =~ /\A--/ }

cmd = "sh configure #{args.join(' ')} 2>&1"
puts "\n#{cmd}\n\n"

IO.popen(cmd) { |pipe| while s = pipe.gets : puts s; end }

code = $CHILD_STATUS ? $CHILD_STATUS.exitstatus : 0

#   The gem command wants some output on stdout. Tell gem what happened.
puts "RMagick configure returned #{code}"
exit code

