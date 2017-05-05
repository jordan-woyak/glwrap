
#include <iostream>

// boost gil not cooperating
//#include <boost/gil/gil_all.hpp>
//#include <boost/gil/extension/io/png_io.hpp>

// lame
#include "../bunny/ply.hpp"

#include "glwrap/gl.hpp"

#include <SFML/Graphics.hpp>

gl::ivec2 getSFImageSize(const sf::Image& img)
{
	return gl::ivec2(img.getSize().x, img.getSize().y);
}

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
	gl::texture_2d tex_color(glc), tex_spec(glc), tex_normal(glc);

	//boost::gil::rgb8_image_t texdata;
	//boost::gil::png_read_image("monkey.png", texdata);

	sf::Image texdata;
	if (texdata.loadFromFile("color.png"))
		tex_color.assign(gl::unpack(texdata.getPixelsPtr(), gl::pixel_format::rgba,
		getSFImageSize(texdata)), gl::base_format::rgb);
	else
		std::cerr << "failed to load color.png" << std::endl;

	if (texdata.loadFromFile("spec.png"))
		tex_spec.assign(gl::unpack(texdata.getPixelsPtr(), gl::pixel_format::rgba,
		getSFImageSize(texdata)), gl::base_format::r);

	if (texdata.loadFromFile("normal.png"))
		tex_normal.assign(gl::unpack(texdata.getPixelsPtr(), gl::pixel_format::rgba,
		getSFImageSize(texdata)), gl::base_format::rgb);

	tex_color.generate_mipmap();
	tex_spec.generate_mipmap();
	tex_normal.generate_mipmap();

	tex_color.set_min_filter(gl::texture_filter::linear);
	tex_color.set_mag_filter(gl::texture_filter::linear);

	//tex_color.generate_mipmap();
	//gl::detail::tex_parameter<false>(GL_TEXTURE_2D,  GL_TEXTURE_BORDER_COLOR, glm::vec4());
	//gl::detail::get_tex_parameter<false, glm::vec4>(GL_TEXTURE_2D,  GL_TEXTURE_BORDER_COLOR);

	tex_spec.set_min_filter(gl::texture_filter::linear);
	tex_spec.set_mag_filter(gl::texture_filter::linear);

	tex_normal.set_min_filter(gl::texture_filter::linear);
	tex_normal.set_mag_filter(gl::texture_filter::linear);
	
	//tex_color.set_wrap_s(gl::wrap_mode::clamp_to_edge);
	//tex_color.set_wrap_t(gl::wrap_mode::clamp_to_edge);

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

	ply::load("model.ply", vert_fmt, vertices, indices);

	// allot generic vetex attrib locations
	gl::attribute_location_alloter locs(glc);
	auto pos_loc = locs.allot<gl::vec3>();
	auto norm_loc = locs.allot<gl::vec3>();
	auto texpos_loc = locs.allot<gl::vec2>();

	gl::texture_unit_alloter units(glc);
	auto tex_color_unit = units.allot<gl::texture_2d>();
	auto tex_spec_unit = units.allot<gl::texture_2d>();
	auto tex_normal_unit = units.allot<gl::texture_2d>();

	// create program
	gl::program prog(glc);

	auto tex_color_uni = prog.create_uniform<gl::texture_2d>("tex_color");
	auto tex_spec_uni = prog.create_uniform<gl::texture_2d>("tex_spec");
	auto tex_normal_uni = prog.create_uniform<gl::texture_2d>("tex_normal");

	auto modelview_uni = prog.create_uniform<gl::mat4>("modelview");
	auto projection_uni = prog.create_uniform<gl::mat4>("projection");

	auto light_dir_uni = prog.create_uniform<gl::vec3>("light_dir");
	auto diff_color_uni = prog.create_uniform<gl::vec4>("diff_color");
	auto spec_color_uni = prog.create_uniform<gl::vec4>("spec_color");
	auto ambient_uni = prog.create_uniform<gl::vec4>("ambient");
	auto shininess_uni = prog.create_uniform<gl::float_t>("shininess");

	gl::vertex_shader vshad(glc);
	auto pos_attrib = vshad.create_input<gl::vec3>("pos");
	auto norm_attrib = vshad.create_input<gl::vec3>("norm");
	auto texpos_attrib = vshad.create_input<gl::vec2>("texpos");
	vshad.set_source(
		"out vec2 tpos;"
		"out vec3 vertex_normal, norm_light_dir, Ia, E;"

		"void main(void)"
		"{"
			"tpos = texpos;"

			"norm_light_dir = normalize(light_dir);"
			"vertex_normal = mat3(modelview) * norm;"

			"Ia = ambient.rgb * ambient.a;"

			"E = normalize(mat3(projection) * vec3(0, 0, -1));"

			"gl_Position = modelview * projection * vec4(pos, 1);"
		"}"
	);

	gl::fragment_shader fshad(glc);
	auto fragdata = fshad.create_output<gl::vec4>("fragdata");
	fshad.set_source(
		"in vec2 tpos;"
		"in vec3 vertex_normal, norm_light_dir, Ia, E;"

		"void main(void)"
		"{"
			"vec3 adjusted_normal = normalize(vertex_normal);"
			//"vec3 adjusted_normal = normalize(vertex_normal + (texture2D(tex_normal, tpos).rgb * 2 - 1));"

			"vec4 mat_color = texture2D(tex_color, tpos);"
			
			"float LambertTerm = max(dot(adjusted_normal, norm_light_dir), 0.0);"
			"vec3 Id = diff_color.rgb * diff_color.a * LambertTerm;"

			"float mat_spec = texture2D(tex_spec, tpos).r;"

			"vec3 R = reflect(-norm_light_dir, adjusted_normal);"
			"vec3 Is = spec_color.rgb * spec_color.a * mat_spec * pow(max(dot(R, E), 0.0), shininess);"

			"vec3 base = mat_color.rgb;"

			"vec4 final_color = vec4((Ia + Id) * base + Is, mat_color.a);"

			"fragdata = final_color;"
		"}"
	);

	prog.attach(vshad);
	prog.attach(fshad);
	prog.compile();

	std::cout << "vshad log:\n" << vshad.get_log() << std::endl;
	std::cout << "fshad log:\n" << fshad.get_log() << std::endl;

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(norm_attrib, norm_loc);
	prog.bind_attribute(texpos_attrib, texpos_loc);

	prog.link();

	//if (!prog.is_good())
		std::cout << "program log:\n" << prog.get_log() << std::endl;

	// load vertex data
	gl::buffer<FooVertex> verbuf(glc);
	verbuf.assign(vertices);

	// load index data
	gl::buffer<gl::uint_t> indbuf(glc);
	indbuf.assign(indices);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);
	arr.bind_vertex_attribute(pos_loc, verbuf.begin() | &FooVertex::pos);
	arr.bind_vertex_attribute(norm_loc, verbuf.begin() | &FooVertex::norm);
	arr.bind_vertex_attribute(texpos_loc, verbuf.begin() | &FooVertex::texpos);

	gl::float_t rotate = 0;

	prog.set_uniform(diff_color_uni, {1, 1, 1, 1});
	prog.set_uniform(spec_color_uni, {1, 1, 1, 0.1});
	prog.set_uniform(ambient_uni, {1, 1, 1, 0.2});
	prog.set_uniform(light_dir_uni, {0, 0.75, -1});
	prog.set_uniform(shininess_uni, 50);

	prog.set_uniform(projection_uni, {});

	prog.set_uniform(tex_color_uni, tex_color_unit);
	prog.set_uniform(tex_spec_uni, tex_spec_unit);
	prog.set_uniform(tex_normal_uni, tex_normal_unit);

	glc.bind_texture(tex_color_unit, tex_color);
	glc.bind_texture(tex_spec_unit, tex_spec);
	glc.bind_texture(tex_normal_unit, tex_normal);

	glc.enable(gl::capability::depth_test);
	glc.enable(gl::capability::cull_face);

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_element_array(indbuf);
	glc.use_primitive_mode(gl::primitive::triangles);

	auto const post_rotate = gl::translate(0.f, 0.f, -3.f);

	dsp.set_display_func([&]
	{
		// rotating projection
		gl::mat4 modelview = gl::rotate(rotate, 0.f, 1.f, 0.f) * post_rotate *
			gl::perspective(45.f, (float_t)window_size.x / window_size.y, 1.f, 100.f);

		prog.set_uniform(modelview_uni, modelview);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.clear_depth(1.0);
		glc.clear_color({0.2, 0.2, 0.2, 1});

		//glnew::context glnc;

		//auto const val = glnc.clear_color().get().r;
		//auto const val = glnc.parameter<glnew::detail::parameter::clear_color>().get().r;
		
		//std::cout << val << std::endl;
		
		glc.draw_elements(0, indices.size());
	});
	
	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
	});

	dsp.run_loop();
}
