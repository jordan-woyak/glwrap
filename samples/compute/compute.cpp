
#include <iostream>
#include <algorithm>

#include "glwrap/gl.hpp"

int main()
{
	glewExperimental = true;

	gl::context glc;
	gl::display dsp(glc, {120, 120});
	dsp.set_caption("glwrap-compute");

	std::cout << "Vendor: " << glc.get_vendor_name() << std::endl;
	std::cout << "Renderer: " << glc.get_renderer_name() << std::endl;
	std::cout << "Version: " << glc.get_version() << std::endl;
	std::cout << "GLSL Version: " << glc.get_shading_language_version() << std::endl;
	
	//gl::display dsp(glc, window_size);
	//dsp.set_caption("transform_feedback");

	// define some variables in the shader,
	// they are automatically added to the program source

	struct MyInput
	{
		gl::layout::std140<gl::float_t> input1;
		gl::layout::std140<gl::vec2> input2;
		gl::layout::std140<gl::vec3> input3;

		//gl::layout::std140<gl::mat4> input4;
	};

	gl::compute_shader_builder cshad(glc);

	gl::uniform_location_enumerator uniforms(glc);
	auto operand1_uni = cshad.create_uniform(gl::variable<gl::float_t>("operand1", uniforms));

	cshad.set_source(
R"(

//buffer int foo[2];

void main(void)
{
	output1 = float(input1) * float(input2) + operand1 + (input3[0] + input3[1]) / 2.0;
})"
	);

	auto comp_shader = cshad.create_shader(glc);
	std::cout << "cshad log:\n" << comp_shader.get_log() << std::endl;
	//std::cout << "cshad src:\n" << comp_shader.get_source() << std::endl;

	// create a program
	gl::program prog(glc);
	prog.attach(comp_shader);

	struct Output
	{
		gl::float_t output1;
	};

	gl::transform_feedback_binding_enumerator tfbs(glc);
	auto feedback_out = tfbs.get<Output>();
	
	gl::transform_feedback_descriptor tf_desc;
	//tf_desc.bind_vertex_attribute(output1_varying, feedback_out | &Output::output1);

	//prog.use_transform_feedback_description(tf_desc);

	prog.link();
	std::cout << "program log:\n" << prog.get_log() << std::endl;
	
	// Input Buffer	
	struct Input
	{
		gl::int_t input1, input2;
		
		// TODO: support std::array as well
		// Purposely using wrong type to demonstrate conversion
		gl::int_t input3[2];
	};
	
	gl::buffer<Input> input_buffer(glc);
	input_buffer.assign((Input[])
	{
		{45, 10, {0, 2}},
		{2, 30, {1, 3}},
		{70, 7, {2, 4}},
		{23, 20, {3, 5}},
		{42, 70, {4, 6}},
		{10, 499, {5, 7}},
		{89, 30, {6, 8}},
		{120, 7, {7, 9}}
	}, gl::buffer_usage::static_draw);

	// Holds the state of our vertex formats and input buffers:
	gl::vertex_array input_vertices(glc);

	gl::vertex_buffer_binding_enumerator vbuflocs(glc);
	auto input_loc = vbuflocs.get<Input>();

	//input_vertices.set_attribute_format(input1_attrib, input_loc | &Input::input1);
	//input_vertices.set_attribute_format(input2_attrib, input_loc | &Input::input2);
	//input_vertices.set_attribute_format(input3_attrib, input_loc | &Input::input3);

	input_vertices.set_buffer(input_loc, input_buffer.begin());

	std::vector<gl::float_t> operands =
	{
		1000, 2000, 70000, 50000, 6000
	};

	// output buffer
	gl::buffer<Output> output_buffer(glc);
	output_buffer.storage(input_buffer.size() * operands.size(), gl::buffer_usage::static_read);

	gl::transform_feedback tfeedback(glc);
	tfeedback.bind_buffer(feedback_out, output_buffer.begin(), output_buffer.size());

	// Do the actual drawing:
	glc.use_program(prog);
	glc.use_transform_feedback(tfeedback);
	glc.use_vertex_array(input_vertices);

	// transform feedback
	glc.start_transform_feedback(gl::primitive::points);

	for (auto operand : operands)
	{
		prog.set_uniform(operand1_uni, operand);
		glc.draw_arrays(gl::primitive::points, 0, input_buffer.size());
	}

	glc.stop_transform_feedback();

	std::cout << "DONE" << std::endl << std::endl;

	// print results
	std::size_t i{};
	for (auto& vert : gl::mapped_buffer<Output>(output_buffer))
		std::cout << vert.output1 << "\t" << std::string(0 == (++i % input_buffer.size()), '\n');

	std::cout << std::endl;
}
