#===============================================================================
# post-setup.rb - setup documentation
#===============================================================================

EXAMPLES   = '.examples'
STD_URI    = 'http:\/\/www.imagemagick.org'
STD_URI_RE = /"#{STD_URI}/
DONT_RUN   = ['fonts.rb']  # never run these examples
ENTITY     = Hash['&' => '&amp;', '>' => '&gt;', '<' => '&lt;']

if defined?(ToplevelInstaller) && self.class == ToplevelInstaller

  IMBASEURI = get_config('imdoc-base-uri')
  RUBYPROG  = get_config('ruby-prog')
  SRCDIR    = srcdir()
  ALLOW_EXAMPLE_ERRORS = get_config('allow-example-errors') == 'yes'

else

  IMBASEURI = 'file:///usr/local/share/ImageMagick'
  RUBYPROG  = 'ruby'
  SRCDIR    = '.'
  ALLOW_EXAMPLE_ERRORS = true

end


#
# A set of example programs in a directory and the output image each example produces.
#

class ExampleSet
  def initialize(of)
    @status_quo = get_status_quo()
    @errs = 0
    begin
      File.open(EXAMPLES) do |f|
        @targets = Marshal.load(f)
      end
    rescue
      @targets = Hash.new
      @n = 0
      @of = of
      @first_time = true
    else
      @first_time = false
    end
  end

  def persist
    File.open(EXAMPLES, 'w') do |f|
      Marshal.dump(@targets, f)
    end
  end

  def targets(example)
    @targets[example] || Array.new
  end

  def get_status_quo
    sq = Dir["*"]
    sq.delete_if { |entry| File.directory?(entry) }
  end

  def update_targets(example, new)
    t = targets(example) + new
    @targets[example] = t.uniq
  end

  def update_status_quo(example)
    new_status_quo = get_status_quo()
    new = new_status_quo - @status_quo
    update_targets(example, new)
    @status_quo = new_status_quo
  end

  def build(example)
    cmd = "#{RUBYPROG} -I #{SRCDIR}/lib -I #{SRCDIR}/ext/RMagick #{example}"
    print cmd
    print " (example #{@n += 1} of #{@of})" if @first_time
    puts
    system cmd

    if $? != 0 then
      puts("post-setup.rb: #{example} example returned error code #{$?}")
      @errs += 1 unless ALLOW_EXAMPLE_ERRORS
      if @errs > 4
         err(<<-END_EXFAIL
            Too many examples failed. The RMagick documentation cannot be installed
            successfully. Consult the README.txt file and try again, or send email
            to rmagick@rubyforge.org.
            END_EXFAIL
            )
      end
    end

    update_status_quo(example)
  end

  def update(example)
    targets = targets(example)
    up_to_date = ! targets.empty?
    targets.each do |target|
      up_to_date &&= File.exists?(target) && (File.stat(target) >= File.stat(example))
    end
    build(example) unless up_to_date
  end

end

#
# print message and exit
#
def err(msg)
    $stderr.puts "#{$0}: #{msg}"
    exit
end


#
# Modify file lines via proc. If no lines changed, discard new version.
#
def filter(filename, backup=nil, &filter)
  if ! File.writable? filename then
    raise ArgumentError, "`#{filename}' is write-protected"
  end

  backup_name = filename + '.' + (backup || 'old')
  File.rename(filename, backup_name)
  changed = false
  begin
    File.open(filename, 'w') do |output|
      File.foreach(backup_name) do |line|
        old = line
        line = filter.call(line)
        output.puts(line)
        changed ||= line != old
      end
    end
  rescue
    File.rename(backup_name, filename)
    raise
  end

  if !changed
    newname = filename + '.xxx'
    File.rename(filename, newname)
    File.rename(backup_name, filename)
    File.unlink(newname)
  elsif !backup
    # Don't remove old copy if a backup extension was specified
    File.unlink(backup_name) rescue nil
  end
end


#
# Copy an example to the doc directory, wrap in HTML tags
#
def filetoHTML(file, html)
  return if File.exists?(html) && File.stat(html) >= File.stat(file)

  File.open(file) do |src|
    File.open(html, 'w') do |dest|
      dest.puts <<-END_EXHTMLHEAD
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>Example: #{file}</TITLE>
</HEAD>
<BODY style=\"background-color: #fffff0;\">
<PRE>
        END_EXHTMLHEAD

      src.each do |line|
        line.gsub!(/[&><]/) { |s| ENTITY[s] }
        dest.puts(line)
      end

      dest.puts <<-END_EXHTMLTAIL
</PRE>
</BODY>
</HTML>
        END_EXHTMLTAIL
    end
    File.chmod(0644, html)
  end

end

puts "install.rb: entering post-setup phase..."

#
# Don't bother if we're in the sandbox
#
if File.exists? 'CVS/Entries'
  puts "post-setup.rb: in CVS sandbox - stopping..."
  exit
end

puts "post-setup.rb: setting up documentation..."


# We're in the source directory. Process the doc in-place. The post-install.rb
# script moves the generated documentation to the ultimate installation directories.


cwd = Dir.getwd()
Dir.chdir('doc')
begin

  # Step 1: replace www.imagemagick.org with local doc uri
  unless IMBASEURI == STD_URI
    files = Dir['*.html']
    files.delete_if { |file| /\.rb\.html\z/.match(file) }
    files.each do |file|
      filter(file) { |line| line.gsub(STD_URI_RE, "\"#{IMBASEURI}") }
    end
  end

  # Step 2A: edit the shebang line in the examples
  Dir.chdir('ex')
  files = Dir['*.rb']
  files.each do |file|
    filter(file) { |line| line.sub(/\A\#!\s*\S*ruby\s/, '#!'+RUBYPROG+' ') }

    # Step 2B: Make a copy of the example as HTML in the doc directory
    filetoHTML(file, "../#{file}.html")
  end

  # Step 3: run the examples
  examples = Dir['*.rb'].sort
  examples -= DONT_RUN
  es = ExampleSet.new(examples.length)
  begin
    examples.each { |example| es.update(example) }
  ensure
    es.persist
  end

ensure
  Dir.chdir(cwd)
end

