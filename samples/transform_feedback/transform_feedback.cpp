
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::context glc;

	// used connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto input1_loc = locs.allot<gl::vec3>();
	auto input2_loc = locs.allot<gl::vec3>();

	// transform feedback nonsense
	gl::transform_feedback_binding_alloter tfbs(glc);
	auto feedback_out = tfbs.allot<gl::float_t>();

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto input1_attrib = prog.create_attribute<gl::vec3>("input1");
	auto input2_attrib = prog.create_attribute<gl::vec3>("input2");

	auto fragdata = prog.create_fragdata<gl::vec4>("fragdata");

	// TODO: function name :/
	auto output1_varying = prog.create_vertex_out_varying<gl::float_t>("output1");

	prog.set_vertex_shader_source(
		"void main(void)"
		"{"
			"output1 = 2;"//dot(input1, input2);"
		"}"
	);

	prog.set_fragment_shader_source(
		"void main(void)"
		"{"
			"fragdata = vec4(0);"
		"}"
	);

	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_transform_feedback(output1_varying, feedback_out);

	prog.bind_attribute(input1_attrib, input1_loc);
	prog.bind_attribute(input2_attrib, input2_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// custom vertex type
	struct FooVertex
	{
		gl::vec3 input1;
		gl::vec3 input2;
	};

	// input buffer
	gl::buffer<FooVertex> input_buffer(glc);
	{
	std::vector<FooVertex> verts =
	{
		{{-1, 0, 1}, {1, 0, 0}},
		{{1, 0, 0}, {0, 1, 0}},
		{{0, -1, 0}, {0, 0, 1}},
		{{0, 0, 1}, {0, -1, 0}},
	};
	input_buffer.assign(verts);
	}

	gl::vertex_array input_vertices(glc);
	input_vertices.bind_vertex_attribute(input1_loc, input_buffer.begin() | &FooVertex::input1);
	input_vertices.bind_vertex_attribute(input2_loc, input_buffer.begin() | &FooVertex::input2);

	// output buffer
	gl::buffer<gl::float_t> output_buffer(glc);
	output_buffer.storage(input_buffer.size());

	glc.bind_buffer(feedback_out, output_buffer.begin(), output_buffer.size());

	glc.use_program(prog);
	glc.use_vertex_array(input_vertices);
	glc.use_primitive_mode(gl::primitive::points);

	glc.start_transform_feedback(gl::primitive::points);
	glc.draw_arrays(0, input_buffer.size());
	glc.stop_transform_feedback();

	/*
	auto sync = glc.fence_sync(gl::sync_condition::gpu_commands_complete, 0);
	sync.client_wait_for(std::chrono::seconds(5));
	*/

	glc.finish();

	std::cout << "DONE" << std::endl << std::endl;

	gl::mapped_buffer<gl::float_t> output_view(output_buffer);
	for (auto& vert : output_view)
		std::cout << vert << std::endl;
}
