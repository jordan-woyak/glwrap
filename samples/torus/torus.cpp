
#include <iostream>

// lame
#include "../bunny/ply.hpp"
#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{480, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-torus");

	// custom vertex type
	struct FooVertex
	{
		gl::vec3 pos, norm;
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

	ply::load("./torus.ply", vert_fmt, vertices, indices);

	// allot generic vetex attrib locations
	gl::attribute_location_alloter locs(glc);
	auto pos_loc = locs.allot<gl::vec3>();
	auto norm_loc = locs.allot<gl::vec3>();

	// create program
	gl::program prog(glc);

	auto modelview_uni = prog.create_uniform<gl::mat4>("modelview");
	auto projection_uni = prog.create_uniform<gl::mat4>("projection");

	auto light_dir_uni = prog.create_uniform<gl::vec3>("light_dir");
	auto diff_color_uni = prog.create_uniform<gl::vec4>("diff_color");
	auto spec_color_uni = prog.create_uniform<gl::vec4>("spec_color");
	auto mat_color_uni = prog.create_uniform<gl::vec4>("mat_color");
	auto ambient_uni = prog.create_uniform<gl::vec4>("ambient");
	auto shininess_uni = prog.create_uniform<gl::float_t>("shininess");

	gl::vertex_shader vshad(glc);
	auto pos_attrib = vshad.create_input<gl::vec3>("pos");
	//auto norm_attrib = vshad.create_input<gl::vec3>("norm");

	// requires manually adding input to source
	auto norm_attrib = vshad.assume_input<gl::vec3>("norm");

	vshad.set_source(
		"smooth out vec3 norm_light_dir, vertex_normal, Ia, E;"

		// manually added to demonstrate "assume_input"
		"in vec3 norm;"

		"void main(void)"
		"{"
			"norm_light_dir = normalize(light_dir);"
			"vertex_normal = mat3(modelview) * norm;"
			"E = normalize(mat3(projection) * vec3(0, 0, -1));"

			"Ia = ambient.rgb * ambient.a;"

			"gl_Position = modelview * projection * vec4(pos, 1);"
		"}"
	);

	gl::fragment_shader fshad(glc);
	auto fragdata = fshad.create_output<gl::vec4>("fragdata");

	fshad.set_source(
		"smooth in vec3 norm_light_dir, vertex_normal, Ia, E;"

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

	prog.attach(vshad);
	prog.attach(fshad);
	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(norm_attrib, norm_loc);

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

	gl::float_t rotate = 0;

	prog.set_uniform(diff_color_uni, {1, 1, 0.75, 1});
	prog.set_uniform(spec_color_uni, {1, 1, 1, 0.1});
	prog.set_uniform(ambient_uni, {1, 1, 1, 0.2});
	prog.set_uniform(light_dir_uni, {0, 0, -1});
	prog.set_uniform(shininess_uni, 5);

	prog.set_uniform(mat_color_uni, {0, 0.5, 1, 1});

	prog.set_uniform(projection_uni, {});

	glc.enable(gl::capability::depth_test);
	glc.enable(gl::capability::cull_face);
	glc.front_face(gl::orientation::ccw);

	glc.use_program(prog);
	glc.use_vertex_array(arr);
	glc.use_element_array(indbuf);
	glc.use_primitive_mode(gl::primitive::triangles);

	dsp.set_display_func([&]
	{
		// rotating projection
		gl::mat4 modelview = gl::rotate(rotate, 0.f, 1.f, 0.f) * gl::rotate(rotate * 2.f, 1.f, 0.f, 0.f) *
			gl::translate(0.f, 0.f, -4.f) *
			gl::perspective(45.f, (gl::float_t)window_size.x / window_size.y, 1.f, 100.f);

		prog.set_uniform(modelview_uni, modelview);

		if ((rotate += 3.14 / 360) >= 3.14 * 2)
			rotate -= 3.14 * 2;

		glc.clear_depth(1.0);
		glc.clear_color({0.2, 0.2, 0.2, 1});
		glc.draw_elements(0, indices.size());
	});

	dsp.set_resize_func([&](gl::ivec2 const& _size)
	{
		glc.viewport({0, 0}, window_size = _size);
	});

	dsp.run_loop();
}
