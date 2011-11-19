
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::context glc;
	gl::display dsp(glc);

	// create a texture, load the data (this needs some work)
	gl::texture_2d tex(glc);
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
	tex.assign(gl::unpack(texdata.data(), gl::pixel_format::red, {8, 8}));

	// used connect array_buffer vertex data and program attributes together
	// via the typeless "generic vertex attributes" in a type-safe manner
	gl::attribute_location_alloter locs(glc);
	auto color_loc = locs.allot<gl::fvec3>();
	auto pos_loc = locs.allot<gl::fvec2>();
	auto texpos_loc = locs.allot<gl::fvec2>();

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used
	gl::texture_unit_alloter tunits(glc);
	auto texunit = tunits.allot<gl::texture_2d>();

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
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

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(color_attrib, color_loc);
	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(texpos_attrib, texpos_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// custom vertex type
	struct FooVertex
	{
		gl::fvec2 pos;
		gl::fvec2 texpos;
		gl::fvec3 color;
	};

	// load vertex data
	gl::array_buffer<FooVertex> verbuf(glc);
	std::vector<FooVertex> verts =
	{
		{{16, 16}, {0, 0}, {1, 0, 0}},
		{{16, 464}, {0, 1}, {0, 1, 0}},
		{{624, 464}, {1, 1}, {0, 0, 1}},
		{{624, 16}, {1, 0}, {0, 0, 0}},
	};
	verbuf.assign(verts);

	// load index data
	gl::index_buffer<gl::ushort_t> indbuf(glc);
	std::vector<gl::ushort_t> indbufdata =
	{
		0, 1, 2, 3
	};
	indbuf.assign(indbufdata);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(pos_loc, verbuf.get_component(&FooVertex::pos));
	arr.bind_vertex_attribute(texpos_loc, verbuf.get_component(&FooVertex::texpos));
	arr.bind_vertex_attribute(color_loc, verbuf.get_component(&FooVertex::color));

	gl::framebuffer fbuf(glc);
	//fbuf.set_read_buffer(glc.color_buffer(0));
	fbuf.set_draw_buffers(
	{
		glc.color_buffer(0)
	});

	gl::texture_2d tex2(glc);
	fbuf.bind_attachment(glc.color_buffer(0), gl::texture_attachment(tex2, 0));

	// ortho projection
	gl::matrix4 modelview = gl::ortho(0, 640, 0, 480, -1000, 1000);

	glc.bind_texture(texunit, tex);
	prog.set_uniform(tex_uni, texunit);

	// TODO: binding of framebuffers

	dsp.set_display_func([&]
	{
		glc.clear_color({1, 1, 1, 1});

		prog.set_uniform(mvp_uni, modelview);

		// TODO: kill this method of framebuffer binding
		//fbuf.bind_draw();
		glc.bind_default_framebuffer();

		//glc.draw_arrays(prog, gl::primitive::triangle_fan, arr, 0, 4);
		//glc.draw_elements(prog, gl::primitive::triangle_fan, arr, indbuf, 0, 4);
		glc.draw_elements_offset(prog, gl::primitive::triangle_fan, arr, indbuf, 0, 4, 0);

		/*
		glc.blit_pixels(fbuf.read_buffer(glc.color_buffer(0)), {0, 0}, {100, 100},
			{100, 100}, {200, 200},
			gl::filter::nearest);
		*/

		modelview *= gl::rotate(6.28f / 60 / 2, 0, 0, 1);
	});

	dsp.run_loop();
}
