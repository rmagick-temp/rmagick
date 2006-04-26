#   gem_config.rb - This script is executed by the gem command
#   to generate the top-level RMagick Makefile. Ref: rmagick.gemspec

#   The gem command calls us with some extra arguments we don't use.
#   Filter out anything that isn't a configure option.
OPTIONS = %w{ --bindir=        --sbindir=
              --libexecdir=    --datadir=
              --sysconfdir=    --sharedstatedir=
              --localstatedir= --libdir=
              --includedir=    --oldincludedir=
              --infodir=       --mandir=
              --disable-       --enable-
              --with-          --without-
              --help           --version
              --quiet          --silent
              --cache-file=    --config-cache
              --no-create      --srcdir=
              --prefix=        --exec-prefix=
              --disable-htmldoc
              -h -V -q -C -n }.join('|')

# Accept gem's --no-rdoc option as if --disable-htmldoc had been specified.
args = []
ARGV.each do |arg|
    if arg =~ /\A--no-rdoc\z/i
        args << '--disable-htmldoc'
    elsif arg =~ /\A(#{OPTIONS})/
        args << arg
    end
end

cmd = "sh configure #{args.join(' ')}"
puts "\n#{cmd}\n\n"

rc = system(cmd)
if rc
    puts "RMagick configuration completed successfully."
else
    puts "RMagick configuration failed with status #{$?.exitstatus}."
end

exit $?.exitstatus

