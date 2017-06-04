
#include <iostream>

#include "ply.hpp"
#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{480, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-bunny");

	glc.enable_debugging();

	// custom vertex type
	struct FooVertex
	{
		gl::vec3 pos;
		gl::vec3 norm;
	};

	// nonsense to load ply
	ply::vertex_format<FooVertex> vert_fmt;
	vert_fmt.bind("x", [](std::istream& s, FooVertex& v){ s >> v.pos.x; });
	vert_fmt.bind("y", [](std::istream& s, FooVertex& v){ s >> v.pos.y; });
	vert_fmt.bind("z", [](std::istream& s, FooVertex& v){ s >> v.pos.z; });
	vert_fmt.bind("nx", [](std::istream& s, FooVertex& v){ s >> v.norm.x; });
	vert_fmt.bind("ny", [](std::istream& s, FooVertex& v){ s >> v.norm.y; });
	vert_fmt.bind("nz", [](std::istream& s, FooVertex& v){ s >> v.norm.z; });

	std::vector<FooVertex> vertices;
	std::vector<gl::uint_t> indices;

	ply::load("bun.ply", vert_fmt, vertices, indices);

	gl::attribute_location_enumerator attribs(glc);
	gl::uniform_location_enumerator uniforms(glc);
	gl::fragdata_location_enumerator fragdatas(glc);

	gl::vertex_shader_builder vshad(glc);

	auto model_uni = vshad.create_uniform(gl::variable<gl::mat4>("model", uniforms));
	auto projection_uni = vshad.create_uniform(gl::variable<gl::mat4>("projection", uniforms));

	auto light_dir_uni = vshad.create_uniform(gl::variable<gl::vec3>("light_dir", uniforms));
	auto ambient_uni = vshad.create_uniform(gl::variable<gl::vec4>("ambient", uniforms));

	auto pos_attrib = vshad.create_input(gl::variable<gl::vec3>("pos", attribs));
	auto norm_attrib = vshad.create_input(gl::variable<gl::vec3>("norm", attribs));

	vshad.set_source(
		"out vec3 vertex_normal, norm_light_dir, Ia, E;"

		"void main(void)"
		"{"
			"norm_light_dir = normalize(light_dir);"
			"vertex_normal = mat3(model) * norm;"
			"E = normalize(mat3(projection) * vec3(0, 0, -1));"

			"Ia = ambient.rgb * ambient.a;"

			"gl_Position = projection * model * vec4(pos, 1);"
		"}"
	);

	gl::fragment_shader_builder fshad(glc);

	auto diff_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("diff_color", uniforms));
	auto spec_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("spec_color", uniforms));
	auto mat_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("mat_color", uniforms));
	auto shininess_uni = fshad.create_uniform(gl::variable<gl::float_t>("shininess", uniforms));
	
	auto fragdata = fshad.create_output(gl::variable<gl::vec4>("fragdata", fragdatas));
	(void)fragdata;

	fshad.set_source(
		"in vec3 vertex_normal, norm_light_dir, Ia, E;"

		"void main(void)"
		"{"
			"vec3 adjusted_normal = normalize(vertex_normal);"

			"float LambertTerm = max(dot(adjusted_normal, norm_light_dir), 0.0);"
			"vec3 Id = diff_color.rgb * diff_color.a * LambertTerm;"

			"vec3 R = reflect(-norm_light_dir, adjusted_normal);"
			"vec3 Is = spec_color.rgb * spec_color.a * pow(max(dot(R, E), 0.0), shininess);"

			"vec3 base = mat_color.rgb;"

			"vec4 final_color = vec4((Ia + Id) * base + Is, mat_color.a);"

			"fragdata = final_color;"
		"}"
	);

	gl::program prog(glc);

	prog.attach(vshad.create_shader(glc));
	prog.attach(fshad.create_shader(glc));

	//std::cout << "vshad log:\n" << vshad.get_log() << std::endl;
	//std::cout << "fshad log:\n" << fshad.get_log() << std::endl;

	prog.link();

	std::cout << "program log:\n" << prog.get_log() << std::endl;
	std::cout << "program good: " << prog.is_good() << std::endl;

	// load vertex data
	auto verbuf = gl::make_buffer(glc, vertices, gl::buffer_access::none);

	// load index data
	auto indbuf = gl::make_buffer(glc, indices, gl::buffer_access::none);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);

	gl::vertex_buffer_binding_enumerator vbufs(glc);
	auto input_loc = vbufs.get<FooVertex>();

	arr.set_attribute_format(pos_attrib, input_loc | &FooVertex::pos);
	arr.set_attribute_format(norm_attrib, input_loc | &FooVertex::norm);

	arr.set_buffer(input_loc, verbuf.begin());

	gl::float_t rotate = 0;

	prog.set_uniform(diff_color_uni, {1, 1, 0.75f, 1});
	prog.set_uniform(spec_color_uni, {1, 1, 1, 0.1f});
	prog.set_uniform(ambient_uni, {1, 1, 1, 0.2f});
	prog.set_uniform(light_dir_uni, {-1, 1, 1});
	prog.set_uniform(shininess_uni, 5);

	prog.set_uniform(mat_color_uni, {0.5f, 0.25f, 0.125f, 1});

	glc.enable(gl::capability::depth_test);
	//glc.enable(gl::capability::cull_face);
	//glc.front_face(gl::orientation::cw);

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_element_array(indbuf);

	// point the rabbit down towards the camera
	auto const pre_rotate = gl::rotate(0.2f, 1.f, 0.f, 0.f);
	// shift the rabbit out from it's rotation point a bit and scale up
	auto const post_rotate = gl::translate(0.1f, -0.8f, 0.f) * gl::scale(8.f, 8.f, 8.f);

	auto const proj =
		gl::perspective(glm::radians(45.f), (float_t)window_size.x / window_size.y, 1.f, 100.f) *
		glm::lookAt(glm::vec3{0.f, 0.f, 3.f}, glm::vec3{}, glm::vec3{0.f, 1.f, 0.f});
		//gl::ortho(-1.f, 1.f, -1.f, 1.f);
		
	prog.set_uniform(projection_uni, proj);

	glc.set_clear_depth(1.0f);
	glc.set_clear_color({0.2f, 0.2f, 0.2f, 1});

	dsp.set_display_func([&]
	{
		// rotating model
		gl::mat4 model =
			pre_rotate *
			gl::rotate(rotate, 0.f, 1.f, 0.f) *
			post_rotate;			

		prog.set_uniform(model_uni, model);

		if ((rotate += 3.14f * 2 / 360) >= 3.14 * 2)
			rotate -= 3.14f * 2;

		glc.clear(gl::buffer_mask::color | gl::buffer_mask::depth);
		glc.draw_elements(gl::primitive::triangles, 0, indices.size());
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
	});

	dsp.run_loop();
}
