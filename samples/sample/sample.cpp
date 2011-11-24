
#include <iostream>

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{240, 240};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-sample");

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
	auto color_loc = locs.allot<gl::vec3>();
	auto pos_loc = locs.allot<gl::vec2>();
	auto texpos_loc = locs.allot<gl::vec2>();

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used
	gl::texture_unit_alloter tunits(glc);
	auto texunit = tunits.allot<gl::texture_2d>();

	// transform feedback nonsense
	gl::transform_feedback_binding_alloter tfbs(glc);
	auto feedback_test = tfbs.allot<gl::vec3>();

	gl::buffer<gl::vec3> feedback_storage(glc);
	feedback_storage.storage(32);

	glc.bind_buffer(feedback_test, feedback_storage.begin(), feedback_storage.size());

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto mvp_uni = prog.create_uniform<gl::mat4>("mvp");
	auto tex_uni = prog.create_uniform<gl::texture_2d>("tex");

	auto color_attrib = prog.create_attribute<gl::vec3>("color");
	auto pos_attrib = prog.create_attribute<gl::vec2>("position");
	auto texpos_attrib = prog.create_attribute<gl::vec2>("texpos");

	auto fragdata = prog.create_fragdata<gl::vec4>("fragdata");

	// TODO: function name :/
	auto outtest_varying = prog.create_vertex_out_varying<gl::vec2>("outtest");

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
		gl::vec2 pos;
		gl::vec2 texpos;
		gl::vec3 color;
	};

	// load vertex data
	gl::buffer<FooVertex> verbuf(glc);
	{
	std::vector<FooVertex> verts =
	{
		{{-9, 9}, {0, 0}, {1, 0, 0}},
		{{-9, -9}, {0, 1}, {0, 1, 0}},
		{{9, -9}, {1, 1}, {0, 0, 1}},
		{{9, 9}, {1, 0}, {0, 0, 0}},
	};

	// just to show it works
	verbuf.storage(verts.size());
	gl::mapped_buffer<FooVertex> vermap(verbuf);
	std::copy(verts.begin(), verts.end(), vermap.begin());

	//verbuf.assign(verts);
	}

	// load index data
	gl::buffer<gl::ushort_t> indbuf(glc);
	std::vector<gl::ushort_t> indbufdata =
	{
		0, 1, 2, 3
	};
	indbuf.assign(indbufdata);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(pos_loc, verbuf.begin() | &FooVertex::pos);
	arr.bind_vertex_attribute(texpos_loc, verbuf.begin() | &FooVertex::texpos);
	arr.bind_vertex_attribute(color_loc, verbuf.begin() | &FooVertex::color);

	// an fbo
	gl::framebuffer fbuf(glc);
	fbuf.bind_draw_buffer(glc.draw_buffer(0), glc.color_buffer(0));

	// multisampled renderbuffer
	gl::renderbuffer rendbuf(glc);
	rendbuf.storage(window_size, 4);
	// attach to fbo
	fbuf.bind_attachment(glc.color_buffer(0), gl::renderbuffer_attachment(rendbuf));

	glc.bind_texture(texunit, tex);
	prog.set_uniform(tex_uni, texunit);

	gl::sampler samp(glc);
	glc.bind_sampler(texunit, samp);

	samp.set_mag_filter(gl::filter::nearest);

	//glc.enable(gl::capability::multisample);

	gl::float_t rotate = 0;

	dsp.set_display_func([&]
	{
		// TODO: kill this method of framebuffer binding
		fbuf.bind_draw();

		glc.clear_color({1, 1, 1, 1});

		// rotating ortho projection
		gl::mat4 const modelview = gl::rotate(rotate, 0, 0, 1) *
			gl::scale(0.1 * window_size.y / window_size.x, 0.1, 1);
		prog.set_uniform(mvp_uni, modelview);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		//glc.draw(prog, gl::primitive::triangle_fan, arr.begin(), 4);
		glc.draw(prog, gl::primitive::triangle_fan, arr.begin() / indbuf.begin(), 4);

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
