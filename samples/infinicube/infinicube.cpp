
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{480, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-infinicube");

	// allot generic vetex attrib locations
	gl::attribute_location_alloter locs(glc);
	auto pos_loc = locs.allot<gl::vec3>();
	auto texpos_loc = locs.allot<gl::vec2>();

	gl::texture_unit_alloter tunits(glc);
	auto texture_loc = tunits.allot<gl::texture_2d>();

	// create program
	gl::program prog(glc);

	auto mvp_uni = prog.create_uniform<gl::mat4>("mvp");
	auto texture_uni = prog.create_uniform<gl::texture_2d>("texture");

	auto pos_attrib = prog.create_attribute<gl::vec3>("pos");
	auto texpos_attrib = prog.create_attribute<gl::vec2>("texpos");

	auto fragdata = prog.create_fragdata<gl::vec4>("fragdata");

	prog.set_vertex_shader_source(
		"out vec2 uv;"

		"void main(void)"
		"{"
			"uv = texpos;"
			"gl_Position = mvp * vec4(pos, 1);"
		"}"
	);

	prog.set_fragment_shader_source(
		"in vec2 uv;"

		"void main(void)"
		"{"
			"fragdata = vec4(texture2D(texture, uv).rgb, 1.0);"
			//"fragdata = vec4(uv, 1.0, 1.0);"
		"}"
	);

	prog.compile();

	// bind stuff linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(texpos_attrib, texpos_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// custom vertex type
	struct FooVertex
	{
		gl::vec3 pos;
		gl::vec2 texpos;
	};

	// load vertex data
	gl::buffer<FooVertex> vertex_data(glc);
	vertex_data.assign((FooVertex[])
	{
		{{-1, -1, -1}, {0, 0}},
		{{-1, -1, +1}, {1, 0}},
		{{-1, +1, -1}, {0, 1}},
		{{-1, +1, +1}, {1, 1}},
		{{+1, -1, -1}, {0, 0}},
		{{+1, -1, +1}, {1, 0}},
		{{+1, +1, -1}, {0, 1}},
		{{+1, +1, +1}, {1, 1}},
	});

	// load index data
	gl::buffer<gl::uint_t> indices(glc);
	indices.assign((gl::uint_t[])
	{
		0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
	});

	auto const index_count = indices.size();

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array vertices(glc);
	vertices.bind_vertex_attribute(pos_loc, vertex_data.begin() | &FooVertex::pos);
	vertices.bind_vertex_attribute(texpos_loc, vertex_data.begin() | &FooVertex::texpos);

	gl::framebuffer fbo(glc);
	fbo.bind_draw_buffer(glc.draw_buffer(0), glc.color_buffer(0));
	fbo.bind_read_buffer(glc.color_buffer(0));	// TODO: kill line

	gl::texture_2d
		texture1(glc),
		texture2(glc);
	texture1.storage(window_size, gl::base_format::rgba);
	texture2.storage(window_size, gl::base_format::rgba);

	prog.set_uniform(texture_uni, texture_loc);

	glc.use_program(prog);
	glc.use_vertex_array(vertices);
	glc.use_element_array(indices);
	glc.use_primitive_mode(gl::primitive::triangle_strip);
	glc.use_draw_framebuffer(fbo);

	glc.enable(gl::capability::cull_face);
	glc.front_face(gl::orientation::ccw);

	gl::float_t rotate = 0;

	dsp.set_display_func([&]
	{
		// rotating projection
		gl::mat4 modelview = gl::rotate(rotate, 0, 1, 0) * gl::rotate(rotate * 2, 1, 0, 0) *
			gl::translate(0, 0, -6) *
			gl::perspective(45, (float_t)window_size.x / window_size.y, 1, 100);

		prog.set_uniform(mvp_uni, modelview);

		if ((rotate += 3.14 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		fbo.bind_attachment(glc.color_buffer(0), gl::texture_attachment(texture1, 0));
		glc.use_draw_framebuffer(fbo);
		glc.clear_color({1, 0, 0, 1});
		glc.draw_elements(0, index_count);

		//fbo.bind_attachment(glc.color_buffer(0), gl::texture_attachment(texture2, 0));
		//glc.clear_color({1, 0, 0, 1});

		// draw cube to texture
		//fbo.bind_attachment(glc.color_buffer(0), gl::texture_attachment(texture1, 0));
		//glc.bind_texture(texture_loc, texture2);
		//glc.draw_elements(0, index_count);

		// draw cube to main fb
		/*
		glc.use_draw_framebuffer(nullptr);
		fbo.unbind_attachment(glc.color_buffer(0));

		glc.clear_color({0.2, 0.2, 0.2, 1});
		glc.bind_texture(texture_loc, texture1);
		glc.bind_texture(texture_loc, texture2);
		*/

		glc.use_draw_framebuffer(nullptr);
		glc.blit_pixels({0, 0}, window_size, {0, 0}, window_size, gl::filter::nearest);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
		//texture1.storage(window_size, gl::base_format::rgba);
		//texture2.storage(window_size, gl::base_format::rgba);
	});

	dsp.run_loop();
}
