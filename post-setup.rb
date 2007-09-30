#===============================================================================
# post-setup.rb - setup documentation
#===============================================================================

EXAMPLES   = '.examples'
STD_URI    = 'http:\/\/www.imagemagick.org'
STD_URI_RE = /"#{STD_URI}/
DONT_RUN   = ['fonts.rb']  # never run these examples
ENTITY     = Hash['&' => '&amp;', '>' => '&gt;', '<' => '&lt;']

if defined?(Installer) && self.class == Installer

  RUBYPROG  = get_config('ruby-prog')
  SRCDIR    = curr_srcdir()
  ALLOW_EXAMPLE_ERRORS = get_config('allow-example-errors') == 'yes'
  BUILD_HTMLDOC = get_config('disable-htmldoc') != 'yes'

else

  RUBYPROG  = 'ruby'
  SRCDIR    = '.'
  ALLOW_EXAMPLE_ERRORS = true
  BUILD_HTMLDOC = true

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
            Too many examples failed. Search for "Help!" at
            http://rmagick.rubyforge.org/install-faq.html.
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

  #
  # print message and exit
  #
  def err(msg)
      $stderr.puts "#{$0}: #{msg}"
      exit 1
  end

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
<!DOCTYPE public PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN""http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="ex2html.rb" />
  <meta http-equiv="Content-Type" content=
  "text/html; charset=us-ascii" />
  <link rel="stylesheet" type="text/css" href="css/popup.css" />

  <title>RMagick example: #{file}</title>
</head>

<body>
<h1>#{file}</h1>
<div class="bodybox">
<div class="bodyfloat">
<pre>
        END_EXHTMLHEAD

      src.each do |line|
        line.gsub!(/[&><]/) { |s| ENTITY[s] }
        dest.puts(line)
      end

      dest.puts <<-END_EXHTMLTAIL
</pre>
</div>
</div>
<div id="close"><a href="javascript:window.close();">Close window</a></div>
</body>
</html>
        END_EXHTMLTAIL
    end
    File.chmod(0644, html)
  end

end

puts "setup.rb: entering post-setup phase..."


if BUILD_HTMLDOC

  #
  # Don't bother if we're in the sandbox
  #
  if File.exist? 'CVS/Entries'
    puts "post-setup.rb: in CVS sandbox - stopping..."
    exit
  end

  puts "post-setup.rb: setting up documentation..."

  # We're in the source directory. Process the doc in-place. The post-install.rb
  # script moves the generated documentation to the ultimate installation directories.

  cwd = Dir.getwd()
  Dir.chdir('doc')          # need to work with 1.6.x, can't use block form
  begin

    # Step 1A: edit the shebang line in the examples
    Dir.chdir('ex')
    files = Dir['*.rb']
    files.each do |file|
      filter(file) { |line| line.sub(/\A\#!\s*\S*ruby\s/, '#!'+RUBYPROG+' ') }

      # Step 1B: Make a copy of the example as HTML in the doc directory
      filetoHTML(file, "../#{file}.html")
    end

    # Step 2: run the examples
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

else

  puts "post-setup.rb: --disable-htmldoc specified. No documentation will be set up."

end

exit
