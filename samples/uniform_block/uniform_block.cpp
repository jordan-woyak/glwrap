
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::context glc;

	// used to connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto input1_loc = locs.allot<gl::int_t>();

	// connect buffers to varyings in a type-safe manner
	gl::transform_feedback_binding_alloter tfbs(glc);
	auto feedback_out = tfbs.allot<gl::float_t>();

	struct Params
	{
		gl::float_t scale;
		gl::int_t bias;
	};

	// need to specify all members manually :/
	gl::uniform_block_definition<Params> params_def;
	params_def
		(&Params::scale, "scale")
		(&Params::bias, "bias");

	// connect buffers to uniform blocks in a type-safe manner
	gl::uniform_block_binding_alloter blks(glc);
	auto params1_loc = blks.allot<Params>();

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto input1_attrib = prog.create_attribute<gl::int_t>("input1");

	auto output1_varying = prog.create_vertex_out_varying<gl::float_t>("output1");

	auto params1_block = prog.create_uniform_block<Params>("params1", params_def);

	prog.set_vertex_shader_source(
		"void main(void)"
		"{"
			"output1 = input1 * scale + bias;"
		"}"
	);

	prog.compile();

	// bind things before linking
	prog.bind_transform_feedback(output1_varying, feedback_out);

	prog.bind_attribute(input1_attrib, input1_loc);

	prog.link();

	prog.set_uniform_block(params1_block, params1_loc);

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// input buffer
	gl::buffer<gl::int_t> input_buffer(glc);
	input_buffer.assign((gl::int_t[])
	{
		45, 2, 7, 23, 42, 1, 89, 12, 3
	});

	// set buffers, strides, data types, etc
	gl::vertex_array input_vertices(glc);
	input_vertices.bind_vertex_attribute(input1_loc, input_buffer.begin());

	// uniform block values
	gl::buffer<gl::uniform_block_align<Params>> params_buffer(glc);
	params_buffer.assign((Params[])
	{
		{45, 1000}, {2, 302}, {7.2, 7000}, {23, 200}, {42, 74}, {1.5, 499}, {89.999, 300}, {12.12, 7}, {3, 3}
	});

	// output buffer
	gl::buffer<gl::float_t> output_buffer(glc);
	output_buffer.storage(input_buffer.size() * params_buffer.size());

	glc.bind_buffer(feedback_out, output_buffer.begin(), output_buffer.size());

	glc.use_program(prog);
	glc.use_vertex_array(input_vertices);
	glc.use_primitive_mode(gl::primitive::points);

	// transform feedback
	glc.start_transform_feedback(gl::primitive::points);

	for (std::size_t i = 0; i != params_buffer.size(); ++i)
	{
		//std::cerr << glGetError() << std::endl;
		glc.bind_buffer(params1_loc, params_buffer.begin() + 0, 1);
		//std::cerr << glGetError() << std::endl;
		glc.draw_arrays(0, input_buffer.size());
		//std::cerr << glGetError() << std::endl;
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
