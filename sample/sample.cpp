
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::context glc;
	gl::display dsp(glc);

	gl::program prog(glc);

	std::vector<gl::ubyte_t> texdata =
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0xff,0x00,0x00,0xff,0x00,0x00,
		0x00,0x00,0xff,0x00,0x00,0xff,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0xff,0x00,0x00,0x00,0x00,0xff,0x00,
		0x00,0xff,0x00,0x00,0x00,0x00,0xff,0x00,
		0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};

	gl::texture_2d tex(glc);
	tex.assign(gl::unpack(texdata.data(), gl::pixel_format::red, {8, 8}));

	//gl::renderbuffer renbuf(glc);

	auto mvp_uni = prog.create_uniform<gl::matrix4>("mvp");
	auto tex_uni = prog.create_uniform<gl::sampler_2d>("tex");

	auto color_attrib = prog.create_attribute<gl::fvec3>("color");
	auto pos_attrib = prog.create_attribute<gl::fvec2>("position");
	auto texpos_attrib = prog.create_attribute<gl::fvec2>("texpos");

	auto fragdata = prog.create_fragdata<gl::fvec4>("fragdata");

	prog.set_vertex_shader_source(
		"out vec3 col;"
		"out vec2 uv;"
		"void main(void)"
		"{"
			"col = color.rgb;"
			"uv = texpos;"
			"gl_Position = mvp * vec4(position, 0, 1);"
		"}"
	);

	prog.set_fragment_shader_source(
		"in vec3 col;"
		"in vec2 uv;"
		"void main(void)"
		"{"
			"fragdata = vec4(col - texture2D(tex, uv).rrr, 1);"
		"}"
	);

	prog.compile();
	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	prog.set_uniform(tex_uni, glc.bind_texture(0, tex));

	struct FooVertex
	{
		gl::fvec2 pos;
		gl::fvec2 texpos;
		gl::fvec3 color;
	};

	gl::array_buffer<FooVertex> verbuf(glc);
	std::vector<FooVertex> verts =
	{
		{{16, 16}, {0, 0}, {1, 0, 0}},
		{{16, 464}, {0, 1}, {0, 1, 0}},
		{{624, 464}, {1, 1}, {0, 0, 1}},
		{{624, 16}, {1, 0}, {0, 0, 0}},
	};
	verbuf.assign(verts);

	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(pos_attrib, verbuf.get_component(&FooVertex::pos));
	arr.bind_vertex_attribute(texpos_attrib, verbuf.get_component(&FooVertex::texpos));
	arr.bind_vertex_attribute(color_attrib, verbuf.get_component(&FooVertex::color));

	gl::matrix4 modelview = gl::ortho(0, 640, 0, 480, -1000, 1000);

	dsp.set_display_func([&]
	{
		glc.clear_color({1, 1, 1, 1});

		prog.set_uniform(mvp_uni, modelview);
		prog.draw_arrays(arr, gl::primitive::triangle_fan, 0, 4);

		modelview *= gl::rotate(6.28f / 60 / 2, 0, 0, 1);
	});

	dsp.run_loop();
}
