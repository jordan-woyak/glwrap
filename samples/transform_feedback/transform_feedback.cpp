
#include <iostream>
#include <algorithm>

#include "glwrap/gl.hpp"

int main()
{
	glewExperimental = true;
	
	//gl::ivec2 window_size{120, 120};

	gl::context glc;
	//gl::display dsp(glc, window_size);
	//dsp.set_caption("transform_feedback");

	// create a program
	gl::program prog(glc);

	// define some variables in the shader,
	// they are automatically added to the program source

	gl::vertex_shader vshad(glc);
	auto input1_attrib = vshad.create_input<gl::int_t>("input1");
	auto input2_attrib = vshad.create_input<gl::int_t>("input2");

	auto output1_varying = vshad.create_output<gl::float_t>("output1");

	auto operand1_uni = prog.create_uniform<gl::float_t>("operand1");

	vshad.set_source(
		"void main(void)"
		"{"
			"output1 = float(input1) * float(input2) + operand1;"
		"}"
	);

	prog.attach(vshad);
	prog.compile();

	std::cout << "vshad log:\n" << vshad.get_log() << std::endl;

	// used to connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_enumerator locs(glc);
	// TODO: allow creation based on type of attrib above?
	auto input1_loc = locs.get<gl::int_t>();
	auto input2_loc = locs.get<gl::int_t>();

	prog.bind_attribute(input1_attrib, input1_loc);
	prog.bind_attribute(input2_attrib, input2_loc);

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
	};
	
	gl::buffer<Input> input_buffer(glc);
	input_buffer.assign((Input[])
	{
		{45, 1}, {2, 3}, {7, 7}, {23, 2}, {42, 7}, {1, 499}, {89, 3}, {12, 7}, {3, 3}
	});

	// Holds the state of our vertex formats and input buffers:
	gl::vertex_array input_vertices(glc);

	input_vertices.enable_vertex_attribute(input1_loc);
	input_vertices.enable_vertex_attribute(input2_loc);

#if 0
	input_vertices.bind_vertex_attribute(input1_loc, input_buffer.begin() | &Input::input1);
	input_vertices.bind_vertex_attribute(input2_loc, input_buffer.begin() | &Input::input2);

#else	
	gl::vertex_buffer_binding_enumerator vbuflocs(glc);
	auto input_loc = vbuflocs.get<Input>();

	input_vertices.bind_vertex_attribute(input1_loc, input_loc | &Input::input1);
	input_vertices.bind_vertex_attribute(input2_loc, input_loc | &Input::input2);

	input_vertices.bind_vertex_buffer(input_loc, input_buffer.begin());
#endif

	std::vector<gl::float_t> operands =
	{
		1000, 2000, 70000, 50000, -1000
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
