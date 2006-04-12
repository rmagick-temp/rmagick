#   gem_config.rb - This script is executed by the gem command
#   to generate the RMagick Makefile. Ref: rmagick.gemspec
#   It resides in ext/RMagick, 2 directories down from the
#   main install directory. The gem command wants to use the
#   Makefile in this directory, not the one at the top directory.

#   Since the gem command runs this script the rbconfig will be
#   the same config that gem is using. We need the "ruby" command
require 'rbconfig'

rubycmd = File.join(::Config::CONFIG['bindir'], ::Config::CONFIG['ruby_install_name'] + ::Config::CONFIG['EXEEXT'])

#   The gem command calls us with some extra arguments we don't use.
#   Filter out anything that doesn't look like a configure option.
args = []
ARGV.each { |arg| args << arg if arg =~ /\A--/ }

cmd = "sh ../../configure #{args.join(' ')}"
puts "\n#{cmd}\n\n"

rc = system(cmd)
if rc
    puts "RMagick configuration completed successfully"
    cmd = "#{rubycmd} ../../setup.rb config"
    puts cmd
    rc = system(cmd)
    if rc
        puts "RMagick Makefile generated\n\n"
        rc = 0
    else
        puts "RMagick setup config failed with status #{$?.exitstatus}"
        rc = $?.exitstatus
    end
else
    puts "RMagick configuration failed with status #{$?.exitstatus}"
    rc = $?.exitstatus
end

exit rc

