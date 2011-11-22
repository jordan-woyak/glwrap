
#include <iostream>

// boost gil not cooperating
//#include <boost/gil/gil_all.hpp>
//#include <boost/gil/extension/io/png_io.hpp>

// lame
#include "../bunny/ply.hpp"

#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{640, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-monkey");

	// custom vertex type
	struct FooVertex
	{
		gl::vec3 pos, norm;
		gl::vec2 texpos;
	};

	// load textures
	gl::texture_2d tex_color(glc);
	gl::texture_2d tex_shininess(glc);

	//boost::gil::rgb8_image_t texdata;
	//boost::gil::png_read_image("monkey.png", texdata);

	sf::Image texdata;
	if (texdata.LoadFromFile("../color.png"))
		tex_color.assign(gl::unpack(texdata.GetPixelsPtr(), gl::pixel_format::rgba, {texdata.GetWidth(), texdata.GetHeight()}));

	if (texdata.LoadFromFile("../shininess.png"))
		tex_shininess.assign(gl::unpack(texdata.GetPixelsPtr(), gl::pixel_format::rgba, {texdata.GetWidth(), texdata.GetHeight()}));

	// nonsense to load ply
	ply::vertex_format<FooVertex> vert_fmt;
	vert_fmt.bind("x", [](std::istream& s, FooVertex& v){ s >> v.pos.x; });
	vert_fmt.bind("y", [](std::istream& s, FooVertex& v){ s >> v.pos.y; });
	vert_fmt.bind("z", [](std::istream& s, FooVertex& v){ s >> v.pos.z; });
	vert_fmt.bind("nx", [](std::istream& s, FooVertex& v){ s >> v.norm.x; });
	vert_fmt.bind("ny", [](std::istream& s, FooVertex& v){ s >> v.norm.y; });
	vert_fmt.bind("nz", [](std::istream& s, FooVertex& v){ s >> v.norm.z; });
	vert_fmt.bind("s", [](std::istream& s, FooVertex& v){ s >> v.texpos.x; });
	vert_fmt.bind("t", [](std::istream& s, FooVertex& v){ s >> v.texpos.y; });

	std::vector<FooVertex> vertices;
	std::vector<gl::uint_t> indices;

	ply::load("../model.ply", vert_fmt, vertices, indices);

	// allot generic vetex attrib locations
	gl::attribute_location_alloter locs(glc);
	auto pos_loc = locs.allot<gl::vec3>();
	auto norm_loc = locs.allot<gl::vec3>();
	auto texpos_loc = locs.allot<gl::vec2>();

	gl::texture_unit_alloter units(glc);
	auto tex_color_unit = units.allot<gl::texture_2d>();
	auto tex_shininess_unit = units.allot<gl::texture_2d>();

	// create program
	gl::program prog(glc);

	auto tex_color_uni = prog.create_uniform<gl::texture_2d>("tex_color");
	auto tex_shininess_uni = prog.create_uniform<gl::texture_2d>("tex_shininess");

	auto modelview_uni = prog.create_uniform<gl::mat4>("modelview");
	auto projection_uni = prog.create_uniform<gl::mat4>("projection");

	auto light_dir_uni = prog.create_uniform<gl::vec3>("light_dir");
	auto diff_color_uni = prog.create_uniform<gl::vec4>("diff_color");
	auto spec_color_uni = prog.create_uniform<gl::vec4>("spec_color");
	auto ambient_uni = prog.create_uniform<gl::float_t>("ambient");

	auto pos_attrib = prog.create_attribute<gl::vec3>("pos");
	auto norm_attrib = prog.create_attribute<gl::vec3>("norm");
	auto texpos_attrib = prog.create_attribute<gl::vec2>("texpos");

	auto fragdata = prog.create_fragdata<gl::vec4>("fragdata");

	prog.set_vertex_shader_source(
		"out vec2 tpos;"
		"out float spec_amt;"
		"out float diffuse_amt;"

		"void main(void)"
		"{"
			"tpos = texpos;"

			"vec3 norm_light_dir = normalize(light_dir);"

			"vec3 vertex_normal = normalize(mat3(modelview) * norm);"
			"diffuse_amt = max(dot(vertex_normal, norm_light_dir), 0.0);"

			"vec3 eye = normalize(mat3(projection) * vec3(0, 0, -1));"
			"spec_amt = dot(vertex_normal, normalize(eye + norm_light_dir));"

			"float spec_cutoff = 0.9;"

			"if (spec_amt < spec_cutoff)"
				"spec_amt = 0.0;"
			"else "
				"spec_amt = mix(0.0, 1.0, (spec_amt - spec_cutoff) / (1 - spec_cutoff));"

			"gl_Position = modelview * projection * vec4(pos, 1);"
		"}"
	);

	prog.set_fragment_shader_source(
		"in vec2 tpos;"
		"in float spec_amt;"
		"in float diffuse_amt;"

		"void main(void)"
		"{"
			"vec4 base_color = texture2D(tex_color, tpos);"

			"vec3 spec = spec_amt * texture2D(tex_shininess, tpos).r * spec_color.rgb;"

			"vec4 final_color = vec4("
				"(ambient + diffuse_amt * diff_color.rgb) * base_color.rgb + spec, base_color.a);"

			"fragdata = final_color;"
		"}"
	);

	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(norm_attrib, norm_loc);
	prog.bind_attribute(texpos_attrib, texpos_loc);

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
	arr.bind_vertex_attribute(pos_loc, verbuf.attrib() | &FooVertex::pos);
	arr.bind_vertex_attribute(norm_loc, verbuf.attrib() | &FooVertex::norm);
	arr.bind_vertex_attribute(texpos_loc, verbuf.attrib() | &FooVertex::texpos);

	gl::float_t rotate = 0;

	prog.set_uniform(diff_color_uni, {1, 1, 1, 1});
	prog.set_uniform(spec_color_uni, {1, 1, 1, 0.1});
	prog.set_uniform(ambient_uni, 0.2);
	prog.set_uniform(light_dir_uni, {0, 0.75, -1});

	prog.set_uniform(projection_uni, {});

	prog.set_uniform(tex_color_uni, tex_color_unit);
	prog.set_uniform(tex_shininess_uni, tex_shininess_unit);

	glc.bind_texture(tex_color_unit, tex_color);
	glc.bind_texture(tex_shininess_unit, tex_shininess);

	// TODO: kill
	glc.bind_default_framebuffer();

	glc.enable(gl::capability::depth_test);
	glc.enable(gl::capability::cull_face);

	auto const post_rotate = gl::translate(0, 0, -3);

	dsp.set_display_func([&]
	{
		glc.clear_color({0.2, 0.2, 0.2, 1});
		glc.clear_depth(1.0);

		// rotating projection
		gl::mat4 modelview = gl::rotate(rotate, 0, 1, 0) * post_rotate *
			gl::perspective(45, (float_t)window_size.x / window_size.y, 1, 100);

		prog.set_uniform(modelview_uni, modelview);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.draw_elements(prog, gl::primitive::triangles, arr, indbuf, 0, indices.size());
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
	});

	dsp.run_loop();
}
