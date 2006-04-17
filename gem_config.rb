#   gem_config.rb - This script is executed by the gem command
#   to generate the top-level RMagick Makefile. Ref: rmagick.gemspec

#   The gem command calls us with some extra arguments we don't use.
#   Filter out anything that doesn't look like a configure option.
args = []
ARGV.each { |arg| args << arg if arg =~ /\A--/ }

cmd = "sh configure #{args.join(' ')}"
puts "\n#{cmd}\n\n"

rc = system(cmd)
if rc
    puts "RMagick configuration completed successfully."
else
    puts "RMagick configuration failed with status #{$?.exitstatus}."
end

exit $?.exitstatus

