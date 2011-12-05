
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{240, 240};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-game_of_life");

	// create a texture, load the data (this needs some work)
	gl::texture_2d tex(glc);
	tex.storage({8, 8}, gl::base_format::r);

	// used connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto position_loc = locs.allot<gl::vec2>();

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used
	gl::texture_unit_alloter tunits(glc);
	auto texture_loc = tunits.allot<gl::texture_2d>();

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto tex_uni = prog.create_uniform<gl::texture_2d>("tex");

	auto color_dead_uni = prog.create_uniform<gl::vec3>("color_dead");
	auto color_live_uni = prog.create_uniform<gl::vec3>("color_live");

	gl::vertex_shader vert_shader(glc);
	auto position_attrib = vert_shader.create_input<gl::vec2>("position");
	vert_shader.set_source(
		"out vec2 pos;"

		"void main(void)"
		"{"
			"gl_Position = vec4(position, 0, 1);"
		"}"
	);

	gl::fragment_shader frag_shader(glc);
	auto fragdata = frag_shader.create_output<gl::vec4>("fragdata");
	frag_shader.set_source(
		"in vec2 pos;"

		"void main(void)"
		"{"
			"fragdata = texture2D(tex, pos);"
		"}"
	);

	prog.attach(vert_shader);
	prog.attach(frag_shader);
	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));
	prog.bind_attribute(position_attrib, position_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// load vertex data
	gl::buffer<gl::vec2> verbuf(glc);
	verbuf.assign((gl::vec2[])
	{
		{0, 0}, {0, 1}, {1, 1}, {1, 0}
	});

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(position_loc, verbuf.begin());

	glc.bind_texture(texture_loc, tex);
	prog.set_uniform(tex_uni, texture_loc);

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_primitive_mode(gl::primitive::triangle_fan);

	// alive and dead colors
	prog.set_uniform(color_dead_uni, {1, 1, 1});
	prog.set_uniform(color_live_uni, {0, 0, 0});

	dsp.set_display_func([&]
	{
		// rotating ortho projection
		gl::mat4 const modelview = gl::scale(window_size.x, window_size.y, 1);

		glc.clear_color({1, 1, 1, 1});
		glc.draw_arrays(0, 4);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
	});

	dsp.run_loop();
}
