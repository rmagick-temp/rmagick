# uninstall RMagick - called from Makefile uninstall target

require 'ftools'

class Dir
  def Dir.safe_unlink(dir)
    begin
      File.chmod 0777, dir
      unlink dir
      $stderr.puts dir
    rescue
    end
  end
end

# remove directory & contents if the directory was created by post-install.rb
def rmdir(dir)
  # This can 't happen, but you can never be too safe...
  if dir == '/' then
    raise RuntimeError, "rm -rf /? I don't think so!"
  end
  if File.file? dir+'/.rmagick' then
    targets = Dir[dir+'/*']
    targets += Dir[dir+'/.*'].delete_if { |f| FileTest.directory?(f) }
    if not targets.empty?
        File.safe_unlink(*targets)
    end
    Dir.safe_unlink(dir)
  end
end

# Load up default values
rbconfig = 'rbconfig'

while arg = ARGV.shift
  case arg
    when /\A--rbconfig=(.*)\z/    # Get overriding rbconfig file name
      rbconfig = $1
    when /\A--prefix=(.*)\z/
      path = $1
      path = File.expand_path(path) unless path[0,1] == '/'
      prefix = path
    when /\A--site-ruby=(.*)\z/   # where RMagick.rb is
      site_ruby = $1
    when /\A--so-dir=(.*)\z/      # where RMagick.so is
      so_dir = $1
    when /\A--doc-dir=(.*)\z/     # where doc is
      doc_dir = $1
  end
end

require rbconfig                      # get specified/default rbconfig.rb

version = ::Config::CONFIG['MAJOR'] + '.' + ::Config::CONFIG['MINOR']
arch    = ::Config::CONFIG['arch']

prefix    ||= ::Config::CONFIG['prefix']
site_ruby ||= prefix+'/lib/ruby/site_ruby/'+version
so_dir    ||= prefix+'/lib/ruby/site_ruby/'+version+'/'+arch
doc_dir   ||= prefix+'/share/RMagick'

File.safe_unlink("#{site_ruby}/RMagick.rb", true)
File.safe_unlink("#{so_dir}/RMagick.so", true)

rmdir(doc_dir+'/ex/images')
rmdir(doc_dir+'/ex')
rmdir(doc_dir)

exit
