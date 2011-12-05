
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::context glc;

	// used to connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto input1_loc = locs.allot<gl::int_t>();
	auto input2_loc = locs.allot<gl::int_t>();

	// connect buffers to varyings in a type-safe manner
	gl::transform_feedback_binding_alloter tfbs(glc);
	auto feedback_out = tfbs.allot<gl::float_t>();

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
			"output1 = input1 * input2 + operand1;"
		"}"
	);

	prog.attach(vshad);
	prog.compile();

	// bind things before linking
	prog.bind_transform_feedback(output1_varying, feedback_out);

	prog.bind_attribute(input1_attrib, input1_loc);
	prog.bind_attribute(input2_attrib, input2_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// custom vertex type
	struct Input
	{
		gl::int_t input1, input2;
	};

	// input buffer
	gl::buffer<Input> input_buffer(glc);
	input_buffer.assign((Input[])
	{
		{45, 1}, {2, 3}, {7, 7}, {23, 2}, {42, 7}, {1, 499}, {89, 3}, {12, 7}, {3, 3}
	});

	// set buffers, strides, data types, etc
	gl::vertex_array input_vertices(glc);
	input_vertices.bind_vertex_attribute(input1_loc, input_buffer.begin() | &Input::input1);
	input_vertices.bind_vertex_attribute(input2_loc, input_buffer.begin() | &Input::input2);

	std::vector<gl::float_t> operands =
	{
		1000, 2000, 7, 500, -1000
	};

	// output buffer
	gl::buffer<gl::float_t> output_buffer(glc);
	output_buffer.storage(input_buffer.size() * operands.size());

	glc.bind_buffer(feedback_out, output_buffer.begin(), output_buffer.size());

	glc.use_program(prog);
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
	for (auto& vert : gl::mapped_buffer<gl::float_t>(output_buffer))
		std::cout << vert << "\t" << std::string(0 == (++i % input_buffer.size()), '\n');

	std::cout << std::endl;
}
