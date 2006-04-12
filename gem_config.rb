#   gem_config.rb - This script is executed by the gem command
#   to generate the RMagick Makefile. Ref: rmagick.gemspec

#   The gem command calls us with some extra arguments we don't use.
#   Filter out anything that doesn't look like a configure option.
args = []
ARGV.each { |arg| args << arg if arg =~ /\A--/ }

cmd = "sh configure #{args.join(' ')}"
puts "\n#{cmd}\n\n"

rc = system(cmd)

#   The gem command wants some output on stdout. Tell gem what happened.
if rc
    puts "RMagick configuration completed successfully"
    rc = 0
else
    puts "RMagick configuration completed with status #{$?.exitstatus}"
    rc = $?.exitstatus
end

exit rc

