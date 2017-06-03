
#include <iostream>
#include <algorithm>

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{640, 640};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-sample");

	glc.enable_profiling();
	glc.enable_debugging();

	std::cout << "Vendor: " << glc.get_vendor_name() << std::endl;
	std::cout << "Renderer: " << glc.get_renderer_name() << std::endl;
	std::cout << "Version: " << glc.get_version() << std::endl;
	std::cout << "GLSL Version: " << glc.get_shading_language_version() << std::endl;

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

	tex.define_storage(1, gl::normalized_internal_format::r8u, {8, 8});
	tex.load_subimage(0, {}, gl::unpack(texdata.data(), gl::pixel_format::r, gl::ivec2{8, 8}));

	gl::vertex_shader_builder vshad(glc);

	// Used to enumerate various opengl indices and connect various objects in a type safe manner
	gl::attribute_location_enumerator attribs(glc);
	gl::uniform_location_enumerator uniforms(glc);
	gl::fragdata_location_enumerator fragdatas(glc);
	gl::texture_unit_enumerator tunits(glc);
	
	// define some variables in the program,
	// they are automatically added to the program source
	//auto model_uni = vshad.create_uniform(gl::variable<gl::mat4>("model", uniforms));

	// Just testing matrix attribs
	auto model_attrib = vshad.create_input(gl::variable<gl::mat4>("model", attribs));

	auto color_attrib = vshad.create_input(gl::variable<gl::vec3>("color", attribs));
	auto pos_attrib = vshad.create_input(gl::variable<gl::vec2>("position", attribs));
	auto texpos_attrib = vshad.create_input(gl::variable<gl::vec2>("texpos", attribs));

	vshad.set_source(
		"out vec3 col;"
		"out vec2 uv;"
		"void main(void)"
		"{"
			"col = color.rgb;"
			"uv = texpos;"
			"gl_Position = model * vec4(position, 0, 1);"
		"}"
	);

	gl::fragment_shader_builder fshad(glc);

	auto tex_uni = fshad.create_uniform(gl::variable<gl::shader::sampler_2d>("tex", uniforms));
	auto fragdata = fshad.create_output(gl::variable<gl::vec4>("fragdata", fragdatas));

	fshad.set_source(
		"in vec3 col;"
		"in vec2 uv;"
		"void main(void)"
		"{"
			"fragdata = vec4(col - texture(tex, uv).rrr, 1);"
		"}"
	);

	auto vert_shader = vshad.create_shader(glc);
	if (!vert_shader.compile_status())
		std::cout << "vshad log:\n" << vert_shader.get_log() << std::endl << vert_shader.get_source();

	auto frag_shader = fshad.create_shader(glc);
	if (!frag_shader.compile_status())
		std::cout << "fshad log:\n" << frag_shader.get_log() << std::endl << frag_shader.get_source();
	
	// create a program
	gl::program prog(glc);

	prog.attach(vert_shader);
	prog.attach(frag_shader);

	prog.link();

	if (!prog.is_good())
	{
		std::cout << "program log:\n" << prog.get_log() << std::endl;
		return 1;
	}

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
	verbuf.assign(verts, gl::buffer_usage::static_draw);
	}

	// Testing if a rotating buffer helps performance
	gl::uint_t rotating_buffer_count = 3;
	gl::uint_t rotating_buffer_iter = 0;

	gl::buffer<gl::mat4> matbuf(glc);
	matbuf.storage(rotating_buffer_count, gl::buffer_usage::stream_draw);

	gl::vertex_buffer_binding_enumerator vbuflocs(glc);
	auto input_loc = vbuflocs.get<FooVertex>();
	auto mat_loc = vbuflocs.get<gl::mat4>();

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.set_attribute_format(pos_attrib, input_loc | &FooVertex::pos);
	arr.set_attribute_format(texpos_attrib, input_loc | &FooVertex::texpos);
	arr.set_attribute_format(color_attrib, input_loc | &FooVertex::color);
	arr.set_buffer(input_loc, verbuf.begin());

	arr.set_attribute_format(model_attrib, mat_loc);
	//arr.set_buffer(mat_loc, matbuf.begin());
	arr.set_divisor(mat_loc, 1);

	gl::color_attachment_enumerator col_attachments(glc);
	auto color0 = col_attachments.get();

	// Create a fbo
	gl::framebuffer_builder fbuf_builder(glc);
	fbuf_builder.bind_draw_buffer(fragdata, color0);
	fbuf_builder.bind_read_buffer(color0);
	
	gl::framebuffer fbuf = fbuf_builder.create_framebuffer(glc);

	// multisampled renderbuffer
	gl::renderbuffer rendbuf(glc);
	rendbuf.storage(window_size, 4);
	fbuf.bind_attachment(color0, rendbuf);

	// used to connect texture objects and sampler uniforms together via texture unit.
	// ensures the correct type of texture is used

	auto texunit = tunits.get<gl::shader::sampler_2d>();
		
	glc.bind_texture(texunit, tex);
	prog.set_uniform(tex_uni, texunit);

	gl::sampler samp(glc);
	glc.bind_sampler(texunit, samp);

	samp.set_min_filter(gl::texture_filter::linear);
	samp.set_mag_filter(gl::texture_filter::nearest);

	// TODO: is this needed?
	//glc.enable(gl::capability::multisample);

	gl::float_t rotate = 0;

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_read_framebuffer(fbuf);

	gl::buffer<gl::draw_arrays_indirect_cmd> cmdbuf(glc);
	cmdbuf.storage(1, gl::buffer_usage::static_draw);

	gl::draw_arrays_indirect_cmd draw_cmd = {};
	draw_cmd.count = 4;
	draw_cmd.instance_count = 1;
	draw_cmd.first = 0;

	cmdbuf.assign_range((gl::draw_arrays_indirect_cmd[]){ {draw_cmd} }, 0);

	glc.set_clear_color({1, 1, 1, 1});

	dsp.set_display_func([&]
	{
		auto const ratio = (float)window_size.y / window_size.x;

		// rotating ortho projection
		gl::mat4 const model = gl::rotate(rotate, 0.f, 0.f, 1.f) *
			gl::scale(0.1f * gl::clamp(ratio, ratio, 1), 0.1f / gl::clamp(ratio, 1, ratio), 1.f);

		//prog.set_uniform(model_uni, model);
		matbuf.assign_range((gl::mat4[]){ model }, rotating_buffer_iter);
		arr.set_buffer(mat_loc, matbuf.begin() + rotating_buffer_iter);

		rotating_buffer_iter = ((rotating_buffer_iter + 1) % rotating_buffer_count);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.use_draw_framebuffer(fbuf);
		glc.clear(gl::buffer_mask::color);

		//glc.draw_arrays(gl::primitive::triangle_fan, 0, 4);
		glc.draw_arrays_indirect(gl::primitive::triangle_fan, cmdbuf.begin());
		//glc.draw_arrays_indirect(gl::primitive::triangle_fan, &draw_cmd);

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
