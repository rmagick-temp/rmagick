# setup RMagick documentation

require 'ftools'
require 'tempfile'

def err(msg)
    $stderr.puts "#{$0}: #{msg}"
    exit
end

# Where to find the ImageMagick documentation
def imbaseuri
    get_config('imdoc-base-uri')
end

# Various methods to get info from the install.rb config table
def rubyprog
    get_config('ruby-prog')
end

# Allow examples to fail?
def allow_example_errors
    get_config('allow-example-errors') == 'yes'
end

# Edit a file in place, replacing all instances of the re "targ" with the string "rep"
def edit_in_place(src, targ, rep)
    File.open('./rmagick.tmp', 'w') do |tmp|
        File.open(src) do |s|
            s.each do |line|
                tmp.puts line.gsub(targ, rep)
            end
        end
    end
    File.mv('./rmagick.tmp', src)
end

# Edit links to ImageMagick documentation.
def edit_html
    cwd = srcdir()
    begin
        Dir.chdir('doc')
        Dir['*.html'].each { |file|
            next if file =~ /\.rb.html/
#           puts "post-setup.rb: setting up #{file}"
            # include leading quote in regexp so only the href= attribute values are replaced
            edit_in_place(file, /"http:\/\/www.imagemagick.org/, "\"#{imbaseuri()}")
            }
    ensure
        Dir.chdir(cwd)
    end
end

# Change shebang line to use path to installed ruby
def edit_shebang(example)
#   puts "post-setup.rb: editing #{example}"
    edit_in_place(example, /\A\#!\s*\S*ruby\s/, '#!'+rubyprog()+' ')
    File.chmod(0755, example)
end

# Create HTML version of a Ruby example in doc directory.
def copy_to_html(example)
    target = '../'+example+'.html'
    return if FileTest.exist? target
#   puts "post-setup.rb: setting up ../#{example}.html"
    File.open(example) do |src|
        File.open(target, 'w') do |dest|
            dest.puts <<END_EXHTMLHEAD
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML>
<HEAD>
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<TITLE>Example: #{example}</TITLE>
</HEAD>
<BODY style=\"background-color: #fffff0;\">
<PRE>
END_EXHTMLHEAD
                src.each do |line|
                    line.gsub!(/&/,'&amp;')
                    line.gsub!(/>/,'&gt;')
                    line.gsub!(/</,'&lt;')
                    dest.puts line
                end
                dest.puts <<END_EXHTMLTAIL
</PRE>
</BODY>
</HTML>
END_EXHTMLTAIL
        end
        File.chmod(0644, target)
    end
end

def make_example_html
    cwd = srcdir()
    begin
        Dir.chdir('doc/ex')
        Dir['*.rb'].each { |file|
            edit_shebang(file)
            copy_to_html(file)
        }
    ensure
        Dir.chdir(cwd)
    end
end

# Add ./lib and ./ext/RMagick to $LOAD_PATH so that the examples are
# run using built RMagick.rb and RMagick.so, not the installed files.
# Gentoo installs into a temporary directory that is not in $LOAD_PATH.
def gen1_image(n, of, file)
    puts("post-setup.rb: run #{file} example (#{n} of #{of} examples)")
    system(rubyprog(), '-I', srcdir()+'/lib', '-I', srcdir()+'/ext/RMagick', file) 
    $?
end

# Generate the example images
def gen_example_images
    errs = 0
    exn = 0
    cwd = srcdir()
    begin
        Dir.chdir('doc/ex')
        examples = Dir['*.rb'].sort
        nex = examples.length
        examples.each do |file|
            exn += 1
            rc = gen1_image(exn, nex, file)
            if rc != 0
                puts("post-setup.rb: #{file} example returned error code #{rc}")
                errs += 1 unless allow_example_errors()
                if errs > 4
                    err(<<END_EXFAIL
Too many examples failed. The RMagick documentation cannot be installed
successfully. Consult the README.txt file and try again, or send email
to cyclists@nc.rr.com.
END_EXFAIL
                        )
                end
            end
        end
    ensure
        Dir.chdir(cwd)
    end
end

puts "\npost-setup.rb: setting up documentation..."

# No use doing this if base URI is the default.
edit_html unless imbaseuri() == "http://www.imagemagick.org"
make_example_html
gen_example_images
exit
