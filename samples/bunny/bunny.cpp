
#include <iostream>

#include "ply.hpp"
#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{480, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-bunny");

	// custom vertex type
	struct FooVertex
	{
		gl::fvec3 pos;
		gl::float_t intens;
	};

	ply::vertex_format<FooVertex> vert_fmt;
	vert_fmt.bind("x", [](std::istream& s, FooVertex& v){ s >> v.pos.x; });
	vert_fmt.bind("y", [](std::istream& s, FooVertex& v){ s >> v.pos.y; });
	vert_fmt.bind("z", [](std::istream& s, FooVertex& v){ s >> v.pos.z; });
	vert_fmt.bind("intensity", [](std::istream& s, FooVertex& v){ s >> v.intens; });

	std::vector<FooVertex> vertices;
	std::vector<gl::uint_t> indices;

	ply::load("../reconstruction/bun_zipper.ply", vert_fmt, vertices, indices);

	gl::attribute_location_alloter locs(glc);
	auto pos_loc = locs.allot<gl::fvec3>();
	auto intens_loc = locs.allot<gl::float_t>();

	gl::program prog(glc);

	auto mvp_uni = prog.create_uniform<gl::matrix4>("mvp");
	auto color_uni = prog.create_uniform<gl::fvec4>("color");

	auto pos_attrib = prog.create_attribute<gl::fvec3>("pos");
	auto intens_attrib = prog.create_attribute<gl::float_t>("intens");

	auto fragdata = prog.create_fragdata<gl::fvec4>("fragdata");

	prog.set_vertex_shader_source(
		"void main(void)"
		"{"
			"gl_Position = mvp * vec4(pos, 1);"
		"}"
	);

	prog.set_fragment_shader_source(
		"void main(void)"
		"{"
			"fragdata = color;"
		"}"
	);

	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(intens_attrib, intens_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// load vertex data
	gl::array_buffer<FooVertex> verbuf(glc);
	verbuf.assign(vertices);

	// load index data
	gl::index_buffer<gl::uint_t> indbuf(glc);
	indbuf.assign(indices);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(pos_loc, verbuf.get_component(&FooVertex::pos));
	arr.bind_vertex_attribute(intens_loc, verbuf.get_component(&FooVertex::intens));

	// an fbo
	gl::framebuffer fbuf(glc);
	fbuf.bind_draw_buffer(glc.draw_buffer(0), glc.color_buffer(0));

	// multisampled renderbuffer
	gl::renderbuffer rendbuf(glc);
	rendbuf.storage(window_size, 4);
	// attach to fbo
	fbuf.bind_attachment(glc.color_buffer(0), gl::renderbuffer_attachment(rendbuf));

	gl::float_t rotate = 0;

	prog.set_uniform(color_uni, {1, 1, 1, 1});

	dsp.set_display_func([&]
	{
		// TODO: kill this method of framebuffer binding
		fbuf.bind_draw();

		glc.clear_color({0.2, 0.2, 0.2, 1});

		// rotating ortho projection
		gl::matrix4 modelview = gl::ortho(-1, 1, -1, 1, -1000, 1000);
		modelview *= gl::scale(8, 8, 8);
		modelview *= gl::translate(0, 0.75, 0);
		modelview *= gl::rotate(rotate, 0, 0, 1);

		prog.set_uniform(mvp_uni, modelview);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.draw_elements(prog, gl::primitive::triangles, arr, indbuf, 0, indices.size());

		// TODO: kill
		glc.bind_default_framebuffer();

		glc.blit_pixels(fbuf.read_buffer(glc.color_buffer(0)), {0, 0}, window_size,
			{0, 0}, window_size,
			gl::filter::nearest);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
		rendbuf.resize(window_size);
	});

	dsp.run_loop();
}
