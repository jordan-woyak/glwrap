
#include <iostream>

// boost gil not cooperating
//#include <boost/gil/gil_all.hpp>
//#include <boost/gil/extension/io/png_io.hpp>

// lame
#include "../bunny/ply.hpp"

#include "glwrap/gl.hpp"

#include <SFML/Graphics.hpp>

gl::uvec2 getSFImageSize(const sf::Image& img)
{
	return gl::uvec2(img.getSize().x, img.getSize().y);
}

int main()
{
	glewExperimental = true;
	
	gl::ivec2 window_size{640, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-monkey");

	std::cout << "Vendor: " << glc.get_vendor_name() << std::endl;
	std::cout << "Renderer: " << glc.get_renderer_name() << std::endl;
	std::cout << "Version: " << glc.get_version() << std::endl;
	std::cout << "GLSL Version: " << glc.get_shading_language_version() << std::endl;

	//glc.enable_debugging();
	//glc.disable_debugging();

	// load textures
	gl::texture_2d tex_color(glc), tex_spec(glc), tex_normal(glc);

	//boost::gil::rgb8_image_t texdata;
	//boost::gil::png_read_image("monkey.png", texdata);

	sf::Image texdata;
	if (texdata.loadFromFile("color.png"))
	{
		auto sz = getSFImageSize(texdata);

		tex_color.define_storage(1, gl::normalized_internal_format::rgb8u, sz);
		tex_color.load_subimage(0, {}, gl::unpack(texdata.getPixelsPtr(), gl::pixel_format::rgba, sz));
	}
	else
		std::cerr << "failed to load color.png" << std::endl;

	if (texdata.loadFromFile("spec.png"))
	{
		auto sz = getSFImageSize(texdata);

		tex_spec.define_storage(1, gl::normalized_internal_format::rgb8u, sz);
		tex_spec.load_subimage(0, {}, gl::unpack(texdata.getPixelsPtr(), gl::pixel_format::rgba, sz));
	}

	if (texdata.loadFromFile("normal.png"))
	{
		auto sz = getSFImageSize(texdata);

		tex_normal.define_storage(1, gl::normalized_internal_format::rgb8u, sz);
		tex_normal.load_subimage(0, {}, gl::unpack(texdata.getPixelsPtr(), gl::pixel_format::rgba, sz));
	}

	tex_color.generate_mipmaps();
	tex_spec.generate_mipmaps();
	tex_normal.generate_mipmaps();
	
	// custom vertex type
	struct FooVertex
	{
		gl::vec3 pos, norm;
		gl::vec2 texpos;
	};

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

	// allot locations automatically
	gl::attribute_location_enumerator attribs(glc);
	gl::uniform_location_enumerator uniforms(glc);
	gl::fragdata_location_enumerator fragdatas(glc);

	gl::texture_unit_enumerator units(glc);
	auto tex_color_unit = units.get<gl::shader::sampler_2d>();
	auto tex_spec_unit = units.get<gl::shader::sampler_2d>();
	auto tex_normal_unit = units.get<gl::shader::sampler_2d>();

	gl::vertex_shader_builder vshad(glc);

	auto pos_attrib = vshad.create_input(gl::variable<gl::vec3>("pos", attribs));
	auto norm_attrib = vshad.create_input(gl::variable<gl::vec3>("norm", attribs));
	auto texpos_attrib = vshad.create_input(gl::variable<gl::vec2>("texpos", attribs));
	
	auto model_uni = vshad.create_uniform(gl::variable<gl::mat4>("model", uniforms));
	auto projection_uni = vshad.create_uniform(gl::variable<gl::mat4>("projection", uniforms));

	auto light_dir_uni = vshad.create_uniform(gl::variable<gl::vec3>("light_dir", uniforms));
	auto ambient_uni = vshad.create_uniform(gl::variable<gl::vec4>("ambient", uniforms));

	// TESTING
#if 0
	auto test_uni = vshad.create_uniform(gl::variable<gl::float_t[4]>("test_var", uniforms));
	auto test2_uni = vshad.create_uniform(gl::variable<gl::shader::sampler_2d[2]>("test2_var", uniforms));
	auto test3_uni = vshad.create_uniform(gl::variable<gl::bool_t>("test3_var", uniforms));
	auto test4_uni = vshad.create_uniform(gl::variable<gl::bool_t[2]>("test4_var", uniforms));
	auto test5_uni = vshad.create_uniform(gl::variable<gl::float_t[2][2]>("test5_var", uniforms));
#endif
	
	vshad.set_source(
		R"(out vec2 tpos;
		out vec3 vertex_normal, norm_light_dir, Ia, E;

		void main(void)
		{
			tpos = texpos;

			norm_light_dir = normalize(light_dir);
			vertex_normal = mat3(model) * norm;

			Ia = ambient.rgb * ambient.a;

			E = normalize(mat3(projection) * vec3(0, 0, -1));

			gl_Position = projection * model * vec4(pos, 1);
		})"
	);

	gl::fragment_shader_builder fshad(glc);

	auto fragdata = fshad.create_output(gl::variable<gl::vec4>("fragdata", fragdatas));
	(void)fragdata;

	auto tex_color_uni = fshad.create_uniform(gl::variable<gl::shader::sampler_2d>("tex_color", uniforms));
	auto tex_spec_uni = fshad.create_uniform(gl::variable<gl::shader::sampler_2d>("tex_spec", uniforms));
	auto tex_normal_uni = fshad.create_uniform(gl::variable<gl::shader::sampler_2d>("tex_normal", uniforms));

	auto diff_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("diff_color", uniforms));
	auto spec_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("spec_color", uniforms));
	auto shininess_uni = fshad.create_uniform(gl::variable<gl::float_t>("shininess", uniforms));
	
	fshad.set_source(
		R"(in vec2 tpos;
		in vec3 vertex_normal, norm_light_dir, Ia, E;

		void main(void)
		{
			vec3 adjusted_normal = normalize(vertex_normal);
			//vec3 adjusted_normal = normalize(vertex_normal + (texture2D(tex_normal, tpos).rgb * 2 - 1));

			vec4 mat_color = texture2D(tex_color, tpos);
			
			float LambertTerm = max(dot(adjusted_normal, norm_light_dir), 0.0);
			vec3 Id = diff_color.rgb * diff_color.a * LambertTerm;

			float mat_spec = texture2D(tex_spec, tpos).r;

			vec3 R = reflect(-norm_light_dir, adjusted_normal);
			vec3 Is = spec_color.rgb * spec_color.a * mat_spec * pow(max(dot(R, E), 0.0), shininess);

			vec3 base = mat_color.rgb;

			vec4 final_color = vec4((Ia + Id) * base + Is, mat_color.a);

			fragdata = final_color;
		})"
	);

	auto vert_shader = vshad.create_shader(glc);
	if (!vert_shader.compile_status())
		std::cout << "vshad log:\n" << vert_shader.get_log() << std::endl;

	auto frag_shader = fshad.create_shader(glc);
	if (!frag_shader.compile_status())
		std::cout << "fshad log:\n" << frag_shader.get_log() << std::endl;

	// create program pipeline
	gl::program prog(glc);
	
	prog.attach(vert_shader);
	prog.attach(frag_shader);

	prog.link();
	
	std::cout << "program log:\n" << prog.get_log() << std::endl;

	if (!prog.is_valid())
		return 1;

	// load vertex data
	gl::buffer<FooVertex> verbuf(glc);

	//verbuf.assign(vertices, gl::buffer_usage::static_draw);
	// Using buffer mapping just because
	verbuf.storage(vertices.size(), gl::buffer_usage::static_draw);
	std::copy(vertices.begin(), vertices.end(), gl::map_buffer(verbuf).begin());

	// load index data
	gl::buffer<gl::uint_t> indbuf(glc);
	indbuf.assign(indices, gl::buffer_usage::static_draw);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);

	gl::vertex_buffer_binding_enumerator vbufs(glc);
	auto input_loc = vbufs.get<FooVertex>();
	
	arr.set_attribute_format(pos_attrib, input_loc | &FooVertex::pos);
	arr.set_attribute_format(norm_attrib, input_loc | &FooVertex::norm);
	arr.set_attribute_format(texpos_attrib, input_loc | &FooVertex::texpos);

	arr.set_buffer(input_loc, verbuf.begin());

	gl::float_t rotate = 0;

	prog.set_uniform(diff_color_uni, {1, 1, 1, 1});
	prog.set_uniform(spec_color_uni, {1, 1, 1, 0.1});
	prog.set_uniform(ambient_uni, {1, 1, 1, 0.2});
	prog.set_uniform(light_dir_uni, {-1, 0.75, 1});
	prog.set_uniform(shininess_uni, 50);

	prog.set_uniform(tex_color_uni, tex_color_unit);
	prog.set_uniform(tex_spec_uni, tex_spec_unit);
	prog.set_uniform(tex_normal_uni, tex_normal_unit);

#if 0
	prog.set_uniform(test_uni, {1.5f, 2.f});
	prog.set_uniform(test2_uni, {tex_color_unit, tex_spec_unit});
	prog.set_uniform(test3_uni, true);
	prog.set_uniform(test4_uni, {false, true});
	prog.set_uniform(test5_uni, {});
#endif

	glc.bind_texture(tex_color_unit, tex_color);
	glc.bind_texture(tex_spec_unit, tex_spec);
	glc.bind_texture(tex_normal_unit, tex_normal);

	glc.enable(gl::capability::depth_test);
	glc.enable(gl::capability::cull_face);

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_element_array(indbuf);

	auto const proj =
		gl::perspective(glm::radians(45.f), (float_t)window_size.x / window_size.y, 1.f, 100.f) *
		glm::lookAt(glm::vec3{0.f, 0.f, 3.5f}, glm::vec3{}, glm::vec3{0.f, 1.f, 0.f});
		//gl::ortho(-1.f, 1.f, -1.f, 1.f);
	
	prog.set_uniform(projection_uni, proj);

	dsp.set_display_func([&]
	{
		// rotating model
		gl::mat4 model = gl::rotate(rotate, 0.f, 1.f, 0.f);
		prog.set_uniform(model_uni, model);

		if ((rotate += 3.14 * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.clear_depth(1.0);
		glc.clear_color({0.2, 0.2, 0.2, 1});

		//glnew::context glnc;

		//auto const val = glnc.clear_color().get().r;
		//auto const val = glnc.parameter<glnew::detail::parameter::clear_color>().get().r;
		
		//std::cout << val << std::endl;
		
		glc.draw_elements(gl::primitive::triangles, 0, indices.size());
	});
	
	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
	});

	dsp.run_loop();
}
