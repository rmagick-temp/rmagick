class Magick::RVG

  private
    def header_text(pgm, name)
        pgm.puts <<"END_HEADER"
/*
Version: #{Magick_version}

gcc `Magick-config --cflags --cppflags` #{name}.c `Magick-config --ldflags --libs` -o #{name}
*/

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <magick/api.h>


int main(int argc,char **argv)
{
  Image *image;
  ImageInfo *info;
  DrawInfo *draw;
  const char * const primitives =
END_HEADER
    end

    def list_primitives(pgm, gc)
        primitives = gc.inspect.split("\n")
        indent = 0
        primitives.each do |cmd|
            indent -= 1 if cmd['pop ']
            pgm.print('  ', ('  '*indent), '"', cmd, '\n"', "\n")
            indent += 1 if cmd['push ']
        end
    end

    def trailer_text(pgm, name)
        pgm.puts <<"END_TRAILER"
  ;

  InitializeMagick("#{name}");

  info = CloneImageInfo(NULL);
  if (!info)
  {
    MagickError(ResourceLimitError,"Unable to allocate ImageInfo",
      "Memory allocation failed");
  }

  info->size = AcquireMagickMemory(20);
  sprintf(info->size, "%dx%d", #{@width.to_i}, #{@height.to_i});

  image = AllocateImage(info);
  if (!image)
  {
    MagickError(ResourceLimitError,"Unable to allocate Image",
      "Memory allocation failed");
  }

  SetImage(image, OpaqueOpacity);

  draw = CloneDrawInfo(info, NULL);
  CloneString(&(draw->primitive), primitives);
  DrawImage(image, draw);

  if (image->exception.severity != UndefinedException)
  {
        printf("%s: %s", image->exception.reason, image->exception.description);
        exit(1);
  }

  strcpy(image->filename, "#{name+".gif"}");
  WriteImage(info, image);

  DestroyDrawInfo(draw);
  DestroyImage(image);
  DestroyImageInfo(info);
  DestroyMagick();

  return 0;
}
END_TRAILER
    end

  public

    # Convert an RVG object to a stand-alone C program
    # suitable for reproducing a bug.
    def to_c(name)
        pgm = File.open(name+".c", "w")
        header_text(pgm, name)
        gc = Draw.new
        add_primitives(gc)
        list_primitives(pgm, gc)
        trailer_text(pgm, name)
        pgm.close
        $stderr.puts "Done"
    end

end     # class Magick::RVG

