
#include <iostream>
#include <random>
#include <chrono>

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 const grid_size{640, 640};
	gl::ivec2 window_size{640, 640};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-game_of_life");

	// create a texture, load the data (this needs some work)
	gl::texture_rectangle tex_data_front(glc), tex_data_back(glc);
	tex_data_front.storage(grid_size, gl::base_format::r);

	// initial random grid state
	{
	std::uniform_int_distribution<gl::ubyte_t> distribution(0, 16);	// 1/8 alive
	std::mt19937 rng;
	// TODO: ugg
	rng.seed((std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count());

	std::vector<gl::ubyte_t> initial_grid_data(grid_size.x * grid_size.y);
	for (auto& cell : initial_grid_data)
		cell = (0 == distribution(rng)) * std::numeric_limits<gl::ubyte_t>::max();

	tex_data_back.assign(gl::unpack(initial_grid_data.data(), gl::pixel_format::r, grid_size), gl::base_format::r);
	}

	// used connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto position_loc = locs.allot<gl::vec2>();

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used
	gl::texture_unit_alloter tunits(glc);
	auto cell_in_loc = tunits.allot<gl::texture_rectangle>();

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto cell_in_uni = prog.create_uniform<gl::texture_rectangle>("cell_in");

	auto color_dead_uni = prog.create_uniform<gl::vec3>("color_dead");
	auto color_live_uni = prog.create_uniform<gl::vec3>("color_live");

	gl::vertex_shader vert_shader(glc);
	auto position_attrib = vert_shader.create_input<gl::vec2>("position");
	vert_shader.set_source(
		"void main(void)"
		"{"
			"gl_Position = vec4(position, 0, 1);"
		"}"
	);

	gl::fragment_shader frag_shader(glc);
	auto cell_out = frag_shader.create_output<gl::vec4>("cell_out");
	auto color_out = frag_shader.create_output<gl::vec4>("color_out");
	frag_shader.set_source(
		"int is_cell_alive(in ivec2 pos)"
		"{"
			"return int(texelFetch(cell_in, pos).r > 0.5);"
		"}"

		"void main(void)"
		"{"
			"ivec2 this_pos = ivec2(gl_FragCoord);"
			"bool old_life = 0 != is_cell_alive(this_pos);"

			"int neighbors = "
				"is_cell_alive(this_pos + ivec2(-1, -1)) + "
				"is_cell_alive(this_pos + ivec2(0, -1)) + "
				"is_cell_alive(this_pos + ivec2(+1, -1)) + "
				"is_cell_alive(this_pos + ivec2(-1, 0)) + "
				"is_cell_alive(this_pos + ivec2(+1, 0)) + "
				"is_cell_alive(this_pos + ivec2(-1, +1)) + "
				"is_cell_alive(this_pos + ivec2(0, +1)) + "
				"is_cell_alive(this_pos + ivec2(+1, +1));"

			"bool new_life = "
				"3 == neighbors || (old_life && 2 == neighbors);"

			"cell_out = vec4(new_life, 1, 1, 1);"
			"color_out = vec4(new_life ? color_live : color_dead, 1);"
		"}"
	);

	prog.attach(vert_shader);
	prog.attach(frag_shader);
	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(cell_out, glc.draw_buffer(0));
	prog.bind_fragdata(color_out, glc.draw_buffer(1));

	prog.bind_attribute(position_attrib, position_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// load vertex data
	gl::buffer<gl::vec2> verbuf(glc);
	verbuf.assign((gl::vec2[])
	{
		{-1, -1}, {-1, 1}, {1, 1}, {1, -1}
	});

	// automatically set data types, sizes, strides
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(position_loc, verbuf.begin());

	gl::framebuffer fbo(glc);
	fbo.bind_draw_buffer(glc.draw_buffer(0), glc.color_buffer(0));
	fbo.bind_draw_buffer(glc.draw_buffer(1), glc.color_buffer(1));

	fbo.bind_read_buffer(glc.color_buffer(1));

	gl::renderbuffer rbuf(glc);
	rbuf.storage(grid_size);
	fbo.bind_attachment(glc.color_buffer(1), gl::renderbuffer_attachment(rbuf));

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_primitive_mode(gl::primitive::triangle_fan);
	glc.use_read_framebuffer(fbo);

	prog.set_uniform(cell_in_uni, cell_in_loc);
	// alive and dead colors
	prog.set_uniform(color_dead_uni, {1, 1, 1});
	prog.set_uniform(color_live_uni, {0, 0, 0});

	glc.viewport({0, 0}, grid_size);

	dsp.set_display_func([&]
	{
		glc.bind_texture(cell_in_loc, tex_data_back);
		fbo.bind_attachment(glc.color_buffer(0), gl::texture_attachment(tex_data_front, 0));
		tex_data_back.swap(tex_data_front);

		glc.use_draw_framebuffer(fbo);
		glc.draw_arrays(0, 4);

		glc.use_draw_framebuffer(nullptr);
		glc.clear_color({0, 0, 0, 0});

		auto const scale = std::min((float)window_size.x / grid_size.x, (float)window_size.y / grid_size.y);

		// lame, need to define more vec2 ops
		gl::ivec2 display_size(grid_size.x * scale, grid_size.y * scale);
		auto offset = (window_size - display_size) / 2;

		glc.blit_pixels({0, 0}, grid_size, offset, offset + display_size, gl::filter::nearest);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		window_size = _size;
	});

	dsp.run_loop();
}
