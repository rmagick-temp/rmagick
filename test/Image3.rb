#! /usr/local/bin/ruby -w

require 'RMagick'
require 'test/unit'
require 'test/unit/ui/console/testrunner'


class Image3_UT < Test::Unit::TestCase
    
    def setup
        @img = Magick::Image.new(20, 20)
    end
    
    def test_profile!
    	assert_nothing_raised do
    		res = @img.profile!('*', nil)
    		assert_same(@img, res)
    	end
    	assert_nothing_raised { @img.profile!('icc', 'xxx') }
    	assert_nothing_raised { @img.profile!('iptc', 'xxx') }
    	assert_nothing_raised { @img.profile!('icc', nil) }
    	assert_nothing_raised { @img.profile!('iptc', nil) }
    	
    	@img.freeze
    	assert_raise(TypeError) { @img.profile!('icc', 'xxx') }
    	assert_raise(TypeError) { @img.profile!('*', nil) }
    end
    
    def test_quantize
    	assert_nothing_raised do 
    		res = @img.quantize
    		assert_instance_of(Magick::Image, res)
    	end
    	
    	colorspace_types = [
		  Magick::RGBColorspace,
		  Magick::GRAYColorspace,
		  Magick::TransparentColorspace,
		  Magick::OHTAColorspace,
		# Magick::LABColorspace,
		  Magick::XYZColorspace,
		  Magick::YCbCrColorspace,
		  Magick::YCCColorspace,
		  Magick::YIQColorspace,
		  Magick::YPbPrColorspace,
		  Magick::YUVColorspace,
		  Magick::CMYKColorspace,
		  Magick::SRGBColorspace,
		  Magick::HSBColorspace,
		  Magick::HSLColorspace,
		  Magick::HWBColorspace,
		# Magick::Rec601LumaColorspace,
		# Magick::Recc709LumaColorspace,
		# LogColorspace
		]
		  
		colorspace_types.each do |cs|
			assert_nothing_raised { @img.quantize(256, cs) }
		end
		assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, false) }
		assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, true, 2) }
		assert_nothing_raised { @img.quantize(256, Magick::RGBColorspace, true, 2, true) }
		assert_raise(TypeError) { @img.quantize('x') }
		assert_raise(TypeError) { @img.quantize(16, 2) }
		assert_raise(TypeError) { @img.quantize(16, Magick::RGBColorspace, false, 'x') }
	end

	def test_quantum_operator
		quantum_ops = [
			Magick::AddQuantumOperator,
			Magick::AndQuantumOperator,
			Magick::DivideQuantumOperator,
			Magick::LShiftQuantumOperator,
			Magick::MultiplyQuantumOperator,
			Magick::OrQuantumOperator,
			Magick::RShiftQuantumOperator,
			Magick::SubtractQuantumOperator,
			Magick::XorQuantumOperator ]
		
		assert_nothing_raised do
    		res = @img.quantum_operator(Magick::AddQuantumOperator, 2)
    		assert_instance_of(Magick::Image, res)
    	end
    	quantum_ops.each do |op|
    		assert_nothing_raised { @img.quantum_operator(op, 2) }
    	end
    	assert_nothing_raised { @img.quantum_operator(Magick::AddQuantumOperator, 2, Magick::RedChannel) }
    	assert_raise(TypeError) { @img.quantum_operator(2, 2) }
    	assert_raise(TypeError) { @img.quantum_operator(Magick::AddQuantumOperator, 'x') }
    	assert_raise(TypeError) { @img.quantum_operator(Magick::AddQuantumOperator, 2, 2) }
    	assert_raise(ArgumentError) { @img.quantum_operator(Magick::AddQuantumOperator, 2, Magick::RedChannel, 2) }
    end
    
    def test_radial_blur
    	assert_nothing_raised do
    		res = @img.radial_blur(30)
    		assert_instance_of(Magick::Image, res)
    	end
    end
    
    def test_raise
    	assert_nothing_raised do
    		res = @img.raise
    		assert_instance_of(Magick::Image, res)
    	end
    	assert_nothing_raised { @img.raise(4) }
    	assert_nothing_raised { @img.raise(4,4) }
    	assert_nothing_raised { @img.raise(4,4, false) }
    	assert_raise(TypeError) { @img.raise('x') }
    	assert_raise(TypeError) { @img.raise(2, 'x') }
    	assert_raise(ArgumentError) { @img.raise(4, 4, false, 2) }
    end
    
    def test_random_threshold_channel
    	assert_nothing_raised do
    		res = @img.random_threshold_channel('20%')
    		assert_instance_of(Magick::Image, res)
    	end
    	threshold = Magick::Geometry.new(20)
    	assert_nothing_raised { @img.random_threshold_channel(threshold) }
    	assert_nothing_raised { @img.random_threshold_channel(threshold, Magick::RedChannel) }
    	assert_nothing_raised { @img.random_threshold_channel(threshold, Magick::RedChannel, Magick::BlueChannel) }
  		assert_raise(ArgumentError) { @img.random_threshold_channel }
  		assert_raise(TypeError) { @img.random_threshold_channel('20%', 2) }
  	end
  	
  	def test_reduce_noise
  		assert_nothing_raised do
  			res = @img.reduce_noise(0)
  			assert_instance_of(Magick::Image, res)
  		end
  		assert_nothing_raised { @img.reduce_noise(4) }
  	end
  	
  	def test_resize
  		assert_nothing_raised do
  			res = @img.resize(2)
  			assert_instance_of(Magick::Image, res)
  		end
  		assert_nothing_raised { @img.resize(50,50) }
  		filters = [
  		  Magick::PointFilter, 
		  Magick::BoxFilter,
		  Magick::TriangleFilter,
		  Magick::HermiteFilter,
		  Magick::HanningFilter,
		  Magick::HammingFilter,
		  Magick::BlackmanFilter,
		  Magick::GaussianFilter,
		  Magick::QuadraticFilter,
		  Magick::CubicFilter,
		  Magick::CatromFilter,
		  Magick::MitchellFilter,
		  Magick::LanczosFilter,
		  Magick::BesselFilter,
		  Magick::SincFilter ]
		  
		filters.each do |filter|
			assert_nothing_raised { @img.resize(50, 50, filter) }
		end
		assert_nothing_raised { @img.resize(50, 50, Magick::PointFilter, 2.0) }
		assert_raise(TypeError) { @img.resize('x') }
		assert_raise(TypeError) { @img.resize(50, 'x') }
		assert_raise(TypeError) { @img.resize(50, 50, 2) }
		assert_raise(TypeError) { @img.resize(50, 50, Magick::CubicFilter, 'x') }
		assert_raise(ArgumentError) { @img.resize(50, 50, Magick::SincFilter, 2.0, 'x') }
		assert_raise(ArgumentError) { @img.resize }
	end
	
	def test_resize!
		assert_nothing_raised do
			res = @img.resize!(2)
			assert_same(@img, res)
		end
		@img.freeze
		assert_raise(TypeError) { @img.resize!(0.50) }
	end
			
	def test_roll
		assert_nothing_raised do
			res = @img.roll(5, 5)
			assert_instance_of(Magick::Image, res)
		end
	end
	
	def test_rotate
		assert_nothing_raised do
			res = @img.rotate(45)
			assert_instance_of(Magick::Image, res)
		end
		assert_nothing_raised { @img.rotate(-45) }
	end
	
	def test_rotate!
		assert_nothing_raised do
			res = @img.rotate!(45)
			assert_same(@img, res)
		end
		@img.freeze
		assert_raise(TypeError) { @img.rotate!(45) }
	end
	
	def test_sample
		assert_nothing_raised do
			res = @img.sample(10, 10)
			assert_instance_of(Magick::Image, res)
		end
		assert_nothing_raised { @img.sample(2) }
		assert_raise(ArgumentError) { @img.sample }
		assert_raise(ArgumentError) { @img.sample(25, 25, 25) }
		assert_raise(TypeError) { @img.sample('x') }
		assert_raise(TypeError) { @img.sample(10, 'x') }
	end
	
	def test_sample!
		assert_nothing_raised do
			res = @img.sample!(2)
			assert_same(@img, res)
		end
		@img.freeze
		assert_raise(TypeError) { @img.sample!(0.50) }
	end
	
	def test_scale
		assert_nothing_raised do
			res = @img.scale(10, 10)
			assert_instance_of(Magick::Image, res)
		end
		assert_nothing_raised { @img.scale(2) }
		assert_raise(ArgumentError) { @img.scale }
		assert_raise(ArgumentError) { @img.scale(25, 25, 25) }
		assert_raise(TypeError) { @img.scale('x') }
		assert_raise(TypeError) { @img.scale(10, 'x') }
	end
	
	def test_scale!
		assert_nothing_raised do
			res = @img.scale!(2)
			assert_same(@img, res)
		end
		@img.freeze
		assert_raise(TypeError) { @img.scale!(0.50) }
	end
  	
end

if __FILE__ == $0
IMAGES_DIR = '../doc/ex/images'
FILES = Dir[IMAGES_DIR+'/Button_*.gif']
Test::Unit::UI::Console::TestRunner.run(Image3_UT)
end
