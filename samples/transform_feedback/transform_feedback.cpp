
#include <iostream>
#include <algorithm>

#include "glwrap/gl.hpp"

int main()
{
	glewExperimental = true;

	gl::context glc;
	gl::display dsp(glc, {120, 120});
	dsp.set_caption("glwrap-transform-feedback");
	
	//gl::display dsp(glc, window_size);
	//dsp.set_caption("transform_feedback");

	// define some variables in the shader,
	// they are automatically added to the program source

	gl::attribute_location_enumerator attribs(glc);

	gl::vertex_shader_builder vshad(glc);
	auto input1_attrib = vshad.create_input(gl::variable<gl::int_t>("input1", attribs));
	auto input2_attrib = vshad.create_input(gl::variable<gl::int_t>("input2", attribs));

	auto input3_attrib = vshad.create_input(gl::variable<gl::float_t[2]>("input3", attribs));

	gl::uniform_location_enumerator uniforms(glc);
	auto operand1_uni = vshad.create_uniform(gl::variable<gl::float_t>("operand1", uniforms));

	auto output1_varying =
		gl::variable_description<gl::float_t, int>("output1", 0);
		//vshad.create_output<gl::variable<gl::float_t>("output1"));

	vshad.set_source(
		"out float output1;"
		"void main(void)"
		"{"
			"output1 = float(input1) * float(input2) + operand1 + (input3[0] + input3[1]) / 2.0;"
		"}"
	);

	auto vert_shader = vshad.create_shader(glc);
	std::cout << "vshad log:\n" << vert_shader.get_log() << std::endl;

	// create a program
	gl::program prog(glc);
	prog.attach(vert_shader);

	struct Output
	{
		gl::float_t output1;
	};

	gl::transform_feedback_binding_enumerator tfbs(glc);
	auto feedback_out = tfbs.get<Output>();
	
	gl::transform_feedback_descriptor tf_desc;
	tf_desc.bind_vertex_attribute(output1_varying, feedback_out | &Output::output1);

	prog.use_transform_feedback_description(tf_desc);

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
	});

	// Holds the state of our vertex formats and input buffers:
	gl::vertex_array input_vertices(glc);

	input_vertices.enable_vertex_attribute(input1_attrib);
	input_vertices.enable_vertex_attribute(input2_attrib);
	input_vertices.enable_vertex_attribute(input3_attrib);

	gl::vertex_buffer_binding_enumerator vbuflocs(glc);
	auto input_loc = vbuflocs.get<Input>();

	input_vertices.bind_vertex_attribute(input1_attrib, input_loc | &Input::input1);
	input_vertices.bind_vertex_attribute(input2_attrib, input_loc | &Input::input2);
	input_vertices.bind_vertex_attribute(input3_attrib, input_loc | &Input::input3);

	input_vertices.bind_vertex_buffer(input_loc, input_buffer.begin());

	std::vector<gl::float_t> operands =
	{
		1000, 2000, 70000, 50000, 6000
	};

	// output buffer
	gl::buffer<Output> output_buffer(glc);
	output_buffer.storage(input_buffer.size() * operands.size());

	gl::transform_feedback tfeedback(glc);
	tfeedback.bind_buffer(feedback_out, output_buffer.begin(), output_buffer.size());

	// Do the actual drawing:
	glc.use_program(prog);
	glc.use_transform_feedback(tfeedback);
	glc.use_vertex_array(input_vertices);
	glc.use_primitive_mode(gl::primitive::points);

	// transform feedback
	glc.start_transform_feedback(gl::primitive::points);

	for (auto operand : operands)
	{
		prog.set_uniform(operand1_uni, operand);
		glc.draw_arrays(0, input_buffer.size());
	}

	glc.stop_transform_feedback();

	glc.finish();
	std::cout << "DONE" << std::endl << std::endl;

	// print results
	std::size_t i{};
	for (auto& vert : gl::mapped_buffer<Output>(output_buffer))
		std::cout << vert.output1 << "\t" << std::string(0 == (++i % input_buffer.size()), '\n');

	std::cout << std::endl;
}
