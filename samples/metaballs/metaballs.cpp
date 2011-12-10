
#include <iostream>
#include <random>
#include <chrono>

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{640, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-metaballs");

	gl::buffer<gl::vec4> metaball_data(glc);
	metaball_data.assign((gl::vec4[])
	{
		{0, 0, 128, 0},
		{300, 32, 200, 0},
		{123, 99, 128, 0},
		{150, 400, 128, 0},
		{480, 224, 300, 0},
		{500, 300, 200, 0},
		{32, 300, 200, 0},
		{300, 300, 128, 0},
		{600, 128, 220, 0},
		{550, 400, 180, 0},
		{400, 200, 200, 0},
	});

	gl::texture_buffer metaball_data_tex(glc);
	metaball_data_tex.bind_buffer(metaball_data);

	// used connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto position_loc = locs.allot<gl::vec2>();

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used
	gl::texture_unit_alloter tunits(glc);
	auto metaball_data_loc = tunits.allot<gl::texture_buffer>();

	// silly
	tunits = gl::texture_unit_alloter(glc);
	auto metaball_energy_loc = tunits.allot<gl::texture_rectangle>();

	// create a program
	gl::program energy_prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto mvp_uni = energy_prog.create_uniform<gl::mat4>("mvp");
	auto metaball_data_uni = energy_prog.create_uniform<gl::texture_buffer>("metaball_data");

	gl::vertex_shader energy_vert_shader(glc);
	auto position_energy_attrib = energy_vert_shader.create_input<gl::vec2>("position");
	energy_vert_shader.create_output<gl::vec2>("pos");
	energy_vert_shader.set_source(
		"void main(void)"
		"{"
			"vec3 this_metaball = texelFetch(metaball_data, gl_InstanceID).xyz;"

			"pos = position;"
			"gl_Position = mvp * vec4(this_metaball.xy + position * this_metaball.z, 0, 1);"
		"}"
	);

	gl::fragment_shader energy_frag_shader(glc);
	//energy_frag_shader.create_input<gl::vec2>("pos");
	auto energy_out = energy_frag_shader.create_output<gl::vec4>("energy_out");
	energy_frag_shader.set_source(
		"in vec2 pos;"

		"void main(void)"
		"{"
			"float energy = 1 - length(pos);"

			"energy_out = vec4(1, 1, 1, energy);"
			//"color_out = vec4(metaball_color, energy);"
		"}"
	);

	energy_prog.attach(energy_vert_shader);
	energy_prog.attach(energy_frag_shader);
	energy_prog.compile();

	// bind attribute and fragdata location before linking
	energy_prog.bind_attribute(position_energy_attrib, position_loc);

	energy_prog.bind_fragdata(energy_out, glc.draw_buffer(0));

	energy_prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << energy_prog.get_log() << std::endl;

	// 2nd pass
	gl::program color_prog(glc);
	auto energy_threshold_uni = color_prog.create_uniform<gl::float_t>("energy_threshold");
	auto metaball_color_uni = color_prog.create_uniform<gl::vec3>("metaball_color");
	auto metaball_energy_uni = color_prog.create_uniform<gl::texture_rectangle>("metaball_energy");

	gl::vertex_shader color_vert_shader(glc);
	auto position_color_attrib = color_vert_shader.create_input<gl::vec2>("position");
	color_vert_shader.set_source(
		"void main(void)"
		"{"
			"gl_Position = vec4(position, 0, 1);"
		"}"
	);

	gl::fragment_shader color_frag_shader(glc);
	auto color_out = color_frag_shader.create_output<gl::vec4>("color_out");
	color_frag_shader.set_source(
		"void main(void)"
		"{"
			"float energy = texture(metaball_energy, vec2(gl_FragCoord)).r;"

			"color_out = vec4(metaball_color, energy > energy_threshold);"
		"}"
	);

	color_prog.attach(color_vert_shader);
	color_prog.attach(color_frag_shader);
	color_prog.compile();

	// bind attribute and fragdata location before linking
	color_prog.bind_attribute(position_color_attrib, position_loc);

	color_prog.bind_fragdata(color_out, glc.draw_buffer(0));

	color_prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << color_prog.get_log() << std::endl;

	// load vertex data
	gl::buffer<gl::vec2> verbuf(glc);
	verbuf.assign((gl::vec2[])
	{
		{-1, -1}, {-1, +1}, {+1, +1}, {+1, -1}
	});

	// automatically set data types, sizes, strides
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(position_loc, verbuf.begin());

	gl::framebuffer fbo(glc);
	fbo.bind_draw_buffer(glc.draw_buffer(0), glc.color_buffer(0));
	fbo.bind_read_buffer(glc.color_buffer(0));

	gl::texture_rectangle metaball_energy_tex(glc);
	//metaball_energy_tex.set_swizzle(gl::swizzle_component::rgba, gl::swizzle_value::r);
	metaball_energy_tex.storage(window_size, gl::base_format::r);
	fbo.bind_attachment(glc.color_buffer(0), gl::texture_attachment(metaball_energy_tex, 0));

	glc.use_vertex_array(arr);
	glc.use_primitive_mode(gl::primitive::triangle_fan);
	glc.use_read_framebuffer(fbo);

	glc.enable(gl::capability::blend);
	glc.blend_func(gl::blend_factor::src_alpha, gl::blend_factor::inverse_src_alpha);

	energy_prog.set_uniform(metaball_data_uni, metaball_data_loc);

	color_prog.set_uniform(metaball_energy_uni, metaball_energy_loc);
	color_prog.set_uniform(energy_threshold_uni, 0.7);
	color_prog.set_uniform(metaball_color_uni, {0, 0, 0});

	dsp.set_display_func([&]
	{
		// first pass, energy levels
		glc.use_program(energy_prog);
		glc.bind_texture(metaball_data_loc, metaball_data_tex);
		energy_prog.set_uniform(mvp_uni, gl::ortho(0, window_size.x, window_size.y, 0, -1, 1));

		glc.use_draw_framebuffer(fbo);
		glc.clear_color({0, 0, 0, 0});
		glc.draw_arrays_instanced(0, 4, metaball_data.size());

		// second pass, energy threshold/colors
		glc.use_program(color_prog);
		glc.bind_texture(metaball_energy_loc, metaball_energy_tex);

		glc.use_draw_framebuffer(nullptr);
		glc.clear_color({1, 1, 1, 1});
		glc.draw_arrays(0, 4);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		window_size = _size;
		glc.viewport({0, 0}, window_size);
	});

	dsp.run_loop();
}
