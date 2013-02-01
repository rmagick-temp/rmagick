# uninstall RMagick - called from Makefile uninstall target

require 'fileutils'

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
def rmdir(dir, no_check=false)
  # This can 't happen, but you can never be too safe...
  if dir == '/' then
    raise RuntimeError, "rm -rf /? I don't think so!"
  end
  if no_check || File.file?(dir+'/.rmagick') then
    targets = Dir[dir+'/*']
    targets += Dir[dir+'/.*'].delete_if { |f| FileTest.directory?(f) }
    if not targets.empty?
        FileUtils.safe_unlink(targets)
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
config = defined?(RbConfig) ? ::RbConfig : ::Config
version = config::CONFIG['ruby_version']
arch    = config::CONFIG['arch']

prefix    ||= config::CONFIG['prefix']
site_ruby ||= config::CONFIG['sitelibdir']
so_dir    ||= config::CONFIG['sitearchdir']
doc_dir   ||= File.join(prefix, 'share', 'RMagick')
dlext       = config::CONFIG['DLEXT']

FileUtils.safe_unlink File.join(site_ruby, 'RMagick.rb'), :verbose => true
FileUtils.safe_unlink File.join(so_dir, 'RMagick.' + dlext), :verbose =>  true
puts site_ruby
puts doc_dir

rmdir File.join(site_ruby, 'rvg'), true
rmdir File.join(doc_dir, 'ex', 'images')
rmdir File.join(doc_dir, 'ex')
rmdir File.join(doc_dir, 'css')
rmdir File.join(doc_dir, 'scripts')
rmdir doc_dir

exit
