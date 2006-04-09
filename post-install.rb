# install RMagick documentation

require 'ftools'
require 'find'

if defined?(Installer) && self.class == Installer
    $docdir = nil

    # Where to install the documentation
    def docdir
        return $docdir if $docdir
        dir = get_config('doc-dir')+'/'
        dir.sub!(/\A$prefix/, get_config('prefix'))
        $docdir = dir
    end
else
    def docdir
        return ARGV[0]
    end
end

puts "\npost-install.rb: installing documentation..."

Find.find('doc') do |file|
    next if FileTest.directory? file
    target = file.sub(/^doc\//,docdir())
    unless FileTest.exists? File.dirname(target)
        File.makedirs(File.dirname(target), true)
        # Mark this directory as one we created so
        # that uninstall.rb knows it's okay to delete
        f = File.new("#{File.dirname(target)}/.rmagick", "w")
        f.close
    end
    File.install(file, target, 0644)
end
exit
