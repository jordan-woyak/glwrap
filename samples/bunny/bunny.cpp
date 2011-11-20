
#include <iostream>

#include "ply.hpp"
#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{480, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-bunny");

	// custom vertex type
	struct FooVertex
	{
		gl::fvec3 pos;
		gl::fvec3 norm;
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

	ply::load("../bun.ply", vert_fmt, vertices, indices);

	// allot generic vetex attrib locations
	gl::attribute_location_alloter locs(glc);
	auto pos_loc = locs.allot<gl::fvec3>();
	auto norm_loc = locs.allot<gl::fvec3>();

	// create program
	gl::program prog(glc);

	auto modelview_uni = prog.create_uniform<gl::matrix4>("modelview");
	auto projection_uni = prog.create_uniform<gl::matrix4>("projection");

	auto light_dir_uni = prog.create_uniform<gl::fvec3>("light_dir");
	auto diff_color_uni = prog.create_uniform<gl::fvec4>("diff_color");
	auto spec_color_uni = prog.create_uniform<gl::fvec4>("spec_color");
	auto mat_color_uni = prog.create_uniform<gl::fvec4>("mat_color");
	auto ambient_uni = prog.create_uniform<gl::float_t>("ambient");

	auto pos_attrib = prog.create_attribute<gl::fvec3>("pos");
	auto norm_attrib = prog.create_attribute<gl::fvec3>("norm");

	auto fragdata = prog.create_fragdata<gl::fvec4>("fragdata");

	prog.set_vertex_shader_source(
		"out vec4 final_color;"

		"void main(void)"
		"{"
			"vec3 norm_light_dir = normalize(light_dir);"

			"vec3 vertex_normal = normalize(mat3(modelview) * norm);"
			"vec3 diffuse = diff_color.rgb * max(dot(vertex_normal, norm_light_dir), 0.0);"

			"vec3 eye = normalize(mat3(projection) * vec3(0, 0, -1));"
			"vec3 spec = max(dot(vertex_normal, normalize(eye + norm_light_dir)) * 10 - 9.0, 0.0) * spec_color.rgb;"

			"final_color = vec4(max(max("
				"(mat_color.rgb * ambient),"
				"spec),"
				"diffuse * mat_color.rgb), mat_color.a);"

			"gl_Position = modelview * projection * vec4(pos, 1);"
		"}"
	);

	prog.set_fragment_shader_source(
		"in vec4 final_color;"

		"void main(void)"
		"{"
			"fragdata = final_color;"
		"}"
	);

	prog.compile();

	// bind attribute and fragdata location before linking
	prog.bind_fragdata(fragdata, glc.draw_buffer(0));

	prog.bind_attribute(pos_attrib, pos_loc);
	prog.bind_attribute(norm_attrib, norm_loc);

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
	arr.bind_vertex_attribute(pos_loc, verbuf.get_component(&FooVertex::pos));
	arr.bind_vertex_attribute(norm_loc, verbuf.get_component(&FooVertex::norm));

	gl::float_t rotate = 0;

	prog.set_uniform(diff_color_uni, {1, 1, 0.75, 1});
	prog.set_uniform(spec_color_uni, {1, 1, 1, 1});
	prog.set_uniform(ambient_uni, 0.2);
	prog.set_uniform(light_dir_uni, {-1, 1, -1});

	prog.set_uniform(mat_color_uni, {0.5, 0.25, 0.125, 1});

	prog.set_uniform(projection_uni, {});

	glc.enable(gl::capability::depth_test);
	//glc.enable(gl::capability::cull_face);
	//glc.front_face(gl::orientation::cw);

	// TODO: kill
	glc.bind_default_framebuffer();

	dsp.set_display_func([&]
	{
		// TODO: kill this method of framebuffer binding
		//fbuf.bind_draw();

		glc.clear_color({0.2, 0.2, 0.2, 1});
		glc.clear_depth(1.0);

		// rotating ortho projection
		gl::matrix4 modelview = gl::scale(8, 8, 8) *
			gl::translate(0.2, -0.8, 0) *
			gl::rotate(rotate, 0, 1, 0) *
			gl::rotate(-0.2, 1, 0, 0);

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
