
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

	glc.enable_debugging();

	// create a texture, load the data (this needs some work)
	gl::texture_2d tex_data_front(glc), tex_data_back(glc);
	tex_data_front.define_storage(1, gl::normalized_internal_format::r8u, grid_size);
	tex_data_back.define_storage(1, gl::normalized_internal_format::r8u, grid_size);

	// initial random grid state
	{
	std::uniform_int_distribution<gl::ubyte_t> distribution(0, 15);
	std::mt19937 rng;
	// TODO: ugg
	rng.seed((std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count());

	std::vector<gl::ubyte_t> initial_grid_data(grid_size.x * grid_size.y);
	for (auto& cell : initial_grid_data)
		cell = (0 == distribution(rng)) * std::numeric_limits<gl::ubyte_t>::max();

	tex_data_back.load_subimage(0, {}, gl::unpack(initial_grid_data.data(), gl::pixel_format::r, grid_size));
	}

	// used connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_enumerator attribs(glc);

	gl::vertex_shader_builder vshad(glc);

	auto position_attrib = vshad.create_input(gl::variable<gl::vec2>("position", attribs));
	
	vshad.set_source(
R"(void main(void)
{
	gl_Position = vec4(position, 0, 1);
})"
	);

	gl::fragment_shader_builder fshad(glc);

	gl::uniform_location_enumerator uniforms(glc);

	auto cell_in_uni = fshad.create_uniform(gl::variable<gl::shader::sampler_2d>("cell_in", uniforms));
	
	auto color_dead_uni = fshad.create_uniform(gl::variable<gl::vec3>("color_dead", uniforms));
	auto color_live_uni = fshad.create_uniform(gl::variable<gl::vec3>("color_live", uniforms));

	gl::fragdata_location_enumerator fragdatas(glc);

	auto cell_out = fshad.create_output(gl::variable<gl::vec4>("cell_out", fragdatas));
	auto color_out = fshad.create_output(gl::variable<gl::vec4>("color_out", fragdatas));
	
	fshad.set_source(
R"(int is_cell_alive(in ivec2 pos)
{
	return int(texelFetch(cell_in, pos, 0).r > 0.5);
}

void main(void)
{
	ivec2 this_pos = ivec2(gl_FragCoord);
	bool old_life = 0 != is_cell_alive(this_pos);

	int neighbors = 
		is_cell_alive(this_pos + ivec2(-1, -1)) +
		is_cell_alive(this_pos + ivec2(0, -1)) +
		is_cell_alive(this_pos + ivec2(+1, -1)) +
		is_cell_alive(this_pos + ivec2(-1, 0)) +
		is_cell_alive(this_pos + ivec2(+1, 0)) +
		is_cell_alive(this_pos + ivec2(-1, +1)) +
		is_cell_alive(this_pos + ivec2(0, +1)) +
		is_cell_alive(this_pos + ivec2(+1, +1));

	bool new_life = 
		3 == neighbors || (old_life && 2 == neighbors);

	cell_out = vec4(new_life, 1, 1, 1);
	color_out = vec4(new_life ? color_live : color_dead, 1);
})"
	);

	gl::program prog(glc);

	auto vshader = vshad.create_shader(glc);
	auto fshader = fshad.create_shader(glc);

	prog.attach(vshader);
	prog.attach(fshader);

	// bind attribute and fragdata location before linking
	//prog.bind_fragdata(cell_out, glc.draw_buffer(0));
	//prog.bind_fragdata(color_out, glc.draw_buffer(1));

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// load vertex data
	gl::buffer<gl::vec2> verbuf(glc);
	verbuf.storage((gl::vec2[])
	{
		{-1, -1}, {-1, 1}, {1, 1}, {1, -1}
	}, gl::buffer_access::none);

	// automatically set data types, sizes, strides
	gl::vertex_array arr(glc);

	gl::vertex_buffer_binding_enumerator vbufs(glc);
	auto input_loc = vbufs.get<gl::vec2>();
	
	arr.set_attribute_format(position_attrib, input_loc);

	arr.set_buffer(input_loc, verbuf.begin());

	gl::color_attachment_enumerator colors(glc);

	auto cell_att = colors.get();
	auto color_att = colors.get();

	gl::framebuffer_builder fbob(glc);

	fbob.bind_draw_buffer(cell_out, cell_att);
	fbob.bind_draw_buffer(color_out, color_att);

	fbob.bind_read_buffer(color_att);

	auto fbo = fbob.create_framebuffer(glc);

	gl::renderbuffer rbuf(glc);
	rbuf.storage(grid_size);
	fbo.bind_attachment(color_att, rbuf);

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_read_framebuffer(fbo);

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used
	gl::texture_unit_enumerator tunits(glc);
	auto cell_in_unit = tunits.get<gl::shader::sampler_2d>();

	prog.set_uniform(cell_in_uni, cell_in_unit);
	// alive and dead colors
	prog.set_uniform(color_dead_uni, {1, 1, 1});
	prog.set_uniform(color_live_uni, {0, 0, 0});

	glc.viewport({0, 0}, grid_size);

	glc.set_clear_color({0, 0, 0, 0});

	dsp.set_display_func([&]
	{
		glc.bind_texture(cell_in_unit, tex_data_back);
		fbo.bind_attachment(cell_att, tex_data_front, 0);

		//tex_data_back.swap(tex_data_front);
		std::swap(tex_data_back, tex_data_front);

		glc.use_draw_framebuffer(fbo);
		glc.draw_arrays(gl::primitive::triangle_fan, 0, 4);

		glc.use_draw_framebuffer(nullptr);
		glc.clear(gl::buffer_mask::color);

		auto const scale = std::min((float)window_size.x / grid_size.x, (float)window_size.y / grid_size.y);

		// lame, need to define more vec2 ops
		gl::ivec2 display_size(grid_size.x * scale, grid_size.y * scale);
		auto offset = (window_size - display_size) / 2;

		glc.blit_pixels({0, 0}, grid_size, offset, offset + display_size,
			gl::buffer_mask::color, gl::filter::nearest);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		window_size = _size;
	});

	dsp.run_loop();
}
