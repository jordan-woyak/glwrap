
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
	tex.assign(gl::unpack(texdata.data(), gl::pixel_format::r, {8, 8}), gl::base_format::r);

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

	// create a program
	gl::program prog(glc);

	// define some variables in the program,
	// they are automatically added to the program source
	auto mvp_uni = prog.create_uniform<gl::mat4>("mvp");
	auto tex_uni = prog.create_uniform<gl::texture_2d>("tex");

	gl::vertex_shader vert_shader(glc);

	auto color_attrib = vert_shader.create_input<gl::vec3>("color");
	auto pos_attrib = vert_shader.create_input<gl::vec2>("position");
	auto texpos_attrib = vert_shader.create_input<gl::vec2>("texpos");

	vert_shader.set_source(
		"out vec3 col;"
		"out vec2 uv;"
		"void main(void)"
		"{"
			"col = color.rgb;"
			"uv = texpos;"
			"gl_Position = mvp * vec4(position, 0, 1);"
		"}"
	);

	gl::fragment_shader frag_shader(glc);

	auto fragdata = frag_shader.create_output<gl::vec4>("fragdata");

	frag_shader.set_source(
		"in vec3 col;"
		"in vec2 uv;"
		"void main(void)"
		"{"
			"fragdata = vec4(col - texture2D(tex, uv).rrr, 1);"
		"}"
	);

	prog.attach(vert_shader);
	prog.attach(frag_shader);
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
	verbuf.assign(verts);
	}

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(pos_loc, verbuf.begin() | &FooVertex::pos);
	arr.bind_vertex_attribute(texpos_loc, verbuf.begin() | &FooVertex::texpos);
	arr.bind_vertex_attribute(color_loc, verbuf.begin() | &FooVertex::color);

	// an fbo
	gl::framebuffer fbuf(glc);
	fbuf.bind_draw_buffer(glc.draw_buffer(0), glc.color_buffer(0));
	fbuf.bind_read_buffer(glc.color_buffer(0));

	// multisampled renderbuffer
	gl::renderbuffer rendbuf(glc);
	rendbuf.storage(window_size, 4);
	fbuf.bind_attachment(glc.color_buffer(0), gl::renderbuffer_attachment(rendbuf));

	glc.bind_texture(texunit, tex);
	prog.set_uniform(tex_uni, texunit);

	gl::sampler samp(glc);
	glc.bind_sampler(texunit, samp);

	samp.set_min_filter(gl::texture_filter::linear);
	samp.set_mag_filter(gl::texture_filter::nearest);

	//glc.enable(gl::capability::multisample);

	gl::float_t rotate = 0;

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_primitive_mode(gl::primitive::triangle_fan);
	glc.use_read_framebuffer(fbuf);

	dsp.set_display_func([&]
	{
		auto const ratio = (float)window_size.y / window_size.x;

		// rotating ortho projection
		gl::mat4 const modelview = gl::rotate(rotate, 0, 0, 1) *
			gl::scale(0.1 * gl::clamp(ratio, ratio, 1), 0.1 / gl::clamp(ratio, 1, ratio), 1);
		prog.set_uniform(mvp_uni, modelview);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.use_draw_framebuffer(fbuf);
		glc.clear_color({1, 1, 1, 1});
		glc.draw_arrays(0, 4);

		glc.use_draw_framebuffer(nullptr);
		glc.blit_pixels({0, 0}, window_size, {0, 0}, window_size, gl::filter::nearest);
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
		rendbuf.resize(window_size);
	});

	dsp.run_loop();
}
