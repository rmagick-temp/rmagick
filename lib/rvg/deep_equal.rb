module Magick
    class RVG

        [PathData, Styles, Transforms].each do |c|
            c.class_eval do
                def deep_equal(other)
                    if self != other
                        puts "#{c.inspect} not equal.\nself:#{self} != other:#{other}"
                        return false
                    end
                    return true
                end
            end
        end

        [Shape, TextBase, Image, Group, Content, Use, ClipPath, Pattern, self].each do |c|
            c.class_eval do
                def deep_equal(other)
                    ivs = self.instance_variables

                    ivs.each do |iv|
                        itv = self.instance_variable_get(iv)
                        otv = other.instance_variable_get(iv)
                        if itv.respond_to?(:deep_equal)
                            if itv.equal?(otv)
                                puts "#{iv} has deep_equal but self.#{iv} and other.#{iv} are the same object."
                                return false
                            end
                            if !itv.deep_equal(otv)
                                puts "Not equal.\nself.#{iv}=#{itv.inspect}\nother.#{iv}=#{otv.inspect}"
                                return false
                            end
                        else
                            case itv
                                when Float, Symbol, TrueClass, FalseClass, Fixnum, NilClass
                                    return false if itv != otv
                                else
                                    if itv.equal?(otv)
                                        puts "#{iv} is dup-able but self.#{iv} and other.#{iv} are the same object."
                                        return false
                                    end
                                    if itv != otv
                                        puts "Not equal.\nself.#{iv}=#{itv.inspect}\nother.#{iv}=#{otv.inspect}"
                                        return false
                                    end
                            end
                        end
                    end

                    return true
                end
            end
        end

    end # class RVG
end # module Magick
