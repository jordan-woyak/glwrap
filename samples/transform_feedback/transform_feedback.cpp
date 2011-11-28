
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

	// connect buffers to varyings in a type-safe manner
	gl::transform_feedback_binding_alloter tfbs(glc);
	auto feedback_out = tfbs.allot<gl::vec3>();

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto input1_attrib = prog.create_attribute<gl::vec3>("input1");
	auto input2_attrib = prog.create_attribute<gl::vec3>("input2");

	auto output1_varying = prog.create_vertex_out_varying<gl::vec3>("output1");

	prog.set_vertex_shader_source(
		"void main(void)"
		"{"
			"output1 = cross(input1, input2);"
		"}"
	);

	prog.compile();

	// bind things before linking
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

	// set buffers, strides, data types, etc
	gl::vertex_array input_vertices(glc);
	input_vertices.bind_vertex_attribute(input1_loc, input_buffer.begin() | &FooVertex::input1);
	input_vertices.bind_vertex_attribute(input2_loc, input_buffer.begin() | &FooVertex::input2);

	// output buffer
	gl::buffer<gl::vec3> output_buffer(glc);
	output_buffer.storage(input_buffer.size());

	glc.bind_buffer(feedback_out, output_buffer.begin(), output_buffer.size());

	// TRANSFORM FEEEEDBAAACK!
	glc.use_program(prog);
	glc.use_vertex_array(input_vertices);
	glc.use_primitive_mode(gl::primitive::points);

	glc.start_transform_feedback(gl::primitive::points);
	glc.draw_arrays(0, input_buffer.size());
	glc.stop_transform_feedback();

	glc.finish();
	std::cout << "DONE" << std::endl << std::endl;

	// print results
	gl::mapped_buffer<gl::vec3> output_view(output_buffer);
	for (auto& vert : output_view)
		std::cout << boost::format("{%d, %d, %d}\n") % vert.x % vert.y % vert.z;
}
