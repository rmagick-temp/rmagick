require 'rvg/rvg'

rvg = Magick::RVG.new(450, 300) do |canvas|
    canvas.desc = 'Example Preserve Aspect Ratio'
    canvas.background_fill = 'white'

    canvas.rect(448, 298, 1, 1).styles(:fill=>'none', :stroke=>'blue')

    # Define the smiley-face
    smile = Magick::RVG::Group.new do |grp|
        grp.translate(0, 5)
        grp.circle(10, 15, 15).styles(:fill=>'yellow', :stroke=>'none')
        grp.circle(1.5, 12, 12).styles(:fill=>'black', :stroke=>'none')
        grp.circle(1.5, 17, 12).styles(:fill=>'black', :stroke=>'none')
        grp.path("M10 19 A 8 8 0 0 0 20 19").styles(:stroke=>'black', :stroke_width=>2)
    end

    viewport1 = Magick::RVG::Group.new do |grp|
        grp.rect(49, 29, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
    end

    viewport2 = Magick::RVG::Group.new do |grp|
        grp.rect(29, 39, 0.5, 0.5).styles(:fill=>'black', :stroke=>'red')
    end

    # SVG to fit
    grp = canvas.g.styles(:font_size=>9) do |grp|
        grp.text(10, 30, "SVG to fit")
        grp.g.translate(20, 40) do |grp2|
            grp2.use(viewport2)
            grp2.use(smile)
        end

        # Viewport 1
        grp.g.translate(10, 120) do |grp2|
           grp2.use(viewport1)
        end
        grp.text(10, 110, 'Viewport 1')

        # Viewport 2
        grp.g.translate(20, 190) do |grp2|
           grp2.rect(29, 50, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
        end
        grp.text(10, 180, 'Viewport 2')

        # meet-group-1
        grp.g.translate(100, 60) do |grp2|
            grp2.text(0, -30, "--------------- meet ---------------")
            grp2.g do |grp3|
                grp3.text(0, -10, "xMin*")
                grp3.use(viewport1)

                # xMin
                grp3.rvg(50, 30).viewbox(0,0,30,40).preserve_aspect_ratio('xMinYMin', 'meet') do |canvas2|
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end

            # xMid
            grp2.g.translate(70, 0) do |grp3|
                grp3.text(0, -10, "xMid*")
                grp3.use(viewport1)
                grp3.rvg(50, 30).viewbox(0, 0, 30, 40).preserve_aspect_ratio('xMidYMid', 'meet') do |canvas2|
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end

            # xMax
            grp2.g.translate(0, 70) do |grp3|
                grp3.text(0, -10, "xMax*")
                grp3.use(viewport1)
                grp3.rvg(50, 30).viewbox(0,0,30,40).preserve_aspect_ratio('xMaxYMax', 'meet') do |canvas2|
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end
        end

        # meet-group-2
        grp.g.translate(250, 60) do |grp2|
            grp2.text(0, -30, "--------------- meet ---------------")

            # xMin
            grp2.g do |grp3|
                grp3.text(0, -10, "*YMin")
                grp3.rect(29, 59, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
                grp3.rvg(30, 60).viewbox(0,0,30,40).preserve_aspect_ratio('xMinYMin', 'meet') do |canvas2|
                    canvas2.use(viewport2)
                    canvas2.use(smile)
                end
            end

            # xMid
            grp2.g.translate(50, 0) do |grp3|
                grp3.text(0, -10, "*YMid")
                grp3.rect(29, 59, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
                grp3.rvg(30, 60).viewbox(0, 0, 30, 40).preserve_aspect_ratio('xMidYMid', 'meet') do |canvas2|
                    canvas2.use(viewport2)
                    canvas2.use(smile)
                end
            end

            # xMax
            grp2.g.translate(100, 0) do |grp3|
                grp3.text(0, -10, "*YMax")
                grp3.rect(29, 59, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
                grp3.rvg(30, 60).viewbox(0,0,30,40).preserve_aspect_ratio('xMaxYMax', 'meet') do |canvas2|
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end
        end

        # slice-group-1
        grp.g.translate(100, 220) do |grp2|
            grp2.text(0, -30, "--------------- slice ---------------")

            # xMin
            grp2.g do |grp3|
                grp3.text(0, -10, "xMin*")
                grp3.rect(29, 59, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
                grp3.rvg(30, 60) do |canvas2|
                    canvas2.preserve_aspect_ratio('xMinYMin', 'slice')
                    canvas2.viewbox(0,0,30,40)
                    canvas2.use(viewport2)
                    canvas2.use(smile)
                end
            end

            # xMid
            grp2.g do |grp3|
                grp3.text(0, -10, "xMid*")
                grp3.translate(50, 0)
                grp3.rect(29, 59, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
                grp3.rvg(30, 60) do |canvas2|
                    canvas2.preserve_aspect_ratio('xMidYMid', 'slice')
                    canvas2.viewbox(0,0,30,40)
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end

            # xMax
            grp2.g do |grp3|
                grp3.text(0, -10, "xMax*")
                grp3.translate(100, 0)
                grp3.rect(29, 59, 0.5, 0.5).styles(:fill => 'none', :stroke => 'blue')
                grp3.rvg(30, 60) do |canvas2|
                    canvas2.preserve_aspect_ratio('xMaxYMax', 'slice')
                    canvas2.viewbox(0,0,30,40)
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end
        end

        # slice-group-2
        grp.g.translate(250, 200) do |grp2|
            grp2.text(0, -30, "--------------- slice ---------------")

            # YMin
            grp2.g do |grp3|
                grp3.text(0, -10, "*YMin")
                grp3.use(viewport1)
                grp3.rvg(50, 30) do |canvas2|
                    canvas2.preserve_aspect_ratio('xMinYMin', 'slice')
                    canvas2.viewbox(0,0,30,40)
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end

            # YMid
            grp2.g do |grp3|
                grp3.text(0, -10, "*YMid")
                grp3.translate(70, 0)
                grp3.use(viewport1)
                grp3.rvg(50, 30) do |canvas2|
                    canvas2.preserve_aspect_ratio('xMidYMid', 'slice')
                    canvas2.viewbox(0,0,30,40)
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end

            # YMax
            grp2.g do |grp3|
                grp3.text(0, -10, "*YMax")
                grp3.translate(140, 0)
                grp3.use(viewport1)
                grp3.rvg(50, 30) do |canvas2|
                    canvas2.preserve_aspect_ratio('xMaxYMax', 'slice')
                    canvas2.viewbox(0,0,30,40)
                    canvas2.rect(29, 39, 0.5, 0.5).styles(:fill => 'black', :stroke => 'red')
                    canvas2.use(smile)
                end
            end
        end
    end
end

rvg.draw.write('PreserveAspectRatio.gif')

