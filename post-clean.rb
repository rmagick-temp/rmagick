# delete RMagick documentation created by post-setup.rb
# doc/*.rb.html
# doc/ex/* (!rb)

require 'find'
require 'ftools'

Dir.foreach('doc') do |entry|
    next if FileTest.directory? 'doc'+File::SEPARATOR+entry
    next unless /\.rb\.html?/.match(entry)
#    puts "deleting #{'doc'+File::SEPARATOR+entry}"
    File.delete('doc'+File::SEPARATOR+entry)
end

dir = 'doc'+File::SEPARATOR+'ex'
Dir.foreach(dir) do |entry|
    next if FileTest.directory? dir+File::SEPARATOR+entry
    next if /\.rb?/.match(entry)
#    puts "deleting #{dir+File::SEPARATOR+entry}"
    File.delete(dir+File::SEPARATOR+entry)
end
