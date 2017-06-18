
#include <iostream>

// lame
#include "../bunny/ply.hpp"
#include "glwrap/gl.hpp"

int main()
{
	gl::ivec2 window_size{640, 480};

	gl::context glc;
	gl::display dsp(glc, window_size);
	dsp.set_caption("glwrap-dragon");

	glc.enable_debugging();

	// custom vertex type
	struct FooVertex
	{
		gl::vec3 pos;
		gl::vec3 norm;
	};

	// nonsense to load ply
	ply::vertex_format<FooVertex> vert_fmt;
	vert_fmt.bind("x") | &FooVertex::pos | &gl::vec3::x;
	vert_fmt.bind("y") | &FooVertex::pos | &gl::vec3::y;
	vert_fmt.bind("z") | &FooVertex::pos | &gl::vec3::z;
	vert_fmt.bind("nx") | &FooVertex::norm | &gl::vec3::x;
	vert_fmt.bind("ny") | &FooVertex::norm | &gl::vec3::y;
	vert_fmt.bind("nz") | &FooVertex::norm | &gl::vec3::z;

	std::vector<FooVertex> vertices;
	std::vector<gl::uint_t> indices;

	ply::load("dragon.ply", vert_fmt, vertices, indices);

	std::cout << "loaded vertices: " << vertices.size() << std::endl;
	std::cout << "loaded indices: " << indices.size() << std::endl;

	gl::attribute_location_enumerator attribs(glc);
	gl::uniform_location_enumerator uniforms(glc);
	gl::fragdata_location_enumerator fragdatas(glc);
	gl::shader_storage_location_enumerator storages(glc);
	gl::image_unit_enumerator image_units(glc);
	gl::shader_storage_binding_enumerator storage_bindings(glc);

	auto const layer_counts_image_unit = image_units.get<gl::shader::uimage_2d>();
	auto const base_index_image_unit = image_units.get<gl::shader::uimage_2d>();

	gl::vertex_shader_builder vshad(glc);

	auto model_uni = vshad.create_uniform(gl::variable<gl::mat4>("model", uniforms));
	auto projection_uni = vshad.create_uniform(gl::variable<gl::mat4>("projection", uniforms));

	auto light_dir_uni = vshad.create_uniform(gl::variable<gl::vec3>("light_dir", uniforms));
	auto ambient_uni = vshad.create_uniform(gl::variable<gl::vec4>("ambient", uniforms));

	auto pos_attrib = vshad.create_input(gl::variable<gl::vec3>("pos", attribs));
	auto norm_attrib = vshad.create_input(gl::variable<gl::vec3>("norm", attribs));

	vshad.set_source(
R"(
out vec3 vertex_normal, norm_light_dir, Ia, E;

void main()
{
	norm_light_dir = normalize(light_dir);
	vertex_normal = mat3(model) * norm;
	E = normalize(mat3(projection) * vec3(0, 0, -1));

	Ia = ambient.rgb * ambient.a;

	gl_Position = projection * model * vec4(pos, 1);
})"
	);

	auto vshader = vshad.create_shader_program(glc);

	gl::fragment_shader_builder count_layers_fshad(glc);

	auto const counter_format = gl::unsigned_image_format::r32ui;

	auto layer_counts_image_desc = gl::variable<gl::shader::uimage_2d>("layer_counts", uniforms, counter_format);
	auto base_index_image_desc = gl::variable<gl::shader::uimage_2d>("base_indices", uniforms, counter_format);

	auto layer_counts_image_uni = count_layers_fshad.create_uniform(layer_counts_image_desc);

	count_layers_fshad.set_source(
R"(

layout(early_fragment_tests) in;

void main()
{
	imageAtomicAdd(layer_counts, ivec2(gl_FragCoord.xy), 1u);
})"
	);

	auto count_layers_fshader = count_layers_fshad.create_shader_program(glc);

	count_layers_fshader.set_uniform(layer_counts_image_uni, layer_counts_image_unit);

	gl::compute_shader_builder cshad(glc);

	cshad.create_uniform(layer_counts_image_desc);
	auto base_index_image_uni = cshad.create_uniform(base_index_image_desc);

	cshad.set_source(
R"(

layout(local_size_x = 32, local_size_y = 32) in;
//layout(local_size_x = 1) in;

shared uint base_index;

void main()
{
	if (0 == gl_LocalInvocationIndex)
	{
		base_index = 0;
	}

	memoryBarrierShared();
	barrier();

	const ivec2 image_size = imageSize(layer_counts);
	const ivec2 start_pos = ivec2(gl_LocalInvocationID.xy);
	const ivec2 step = ivec2(gl_WorkGroupSize.xy);

	ivec2 pos;
	for (pos.x = start_pos.x; pos.x < image_size.x; pos.x += step.x)
	{
		for (pos.y = start_pos.y; pos.y < image_size.y; pos.y += step.y)
		{
			const uint layer_count = imageAtomicExchange(layer_counts, pos, 0u);

			const uint bi = atomicAdd(base_index, layer_count);
			imageStore(base_indices, pos, uvec4(bi));
		}
	}
})"
	);

	auto base_index_cshader = cshad.create_shader_program(glc);

	base_index_cshader.set_uniform(layer_counts_image_uni, layer_counts_image_unit);
	base_index_cshader.set_uniform(base_index_image_uni, base_index_image_unit);

	gl::fragment_shader_builder fshad(glc);

	fshad.create_uniform(base_index_image_desc);
	fshad.create_uniform(layer_counts_image_desc);

	auto fragment_buf_desc = gl::variable<gl::vec4[]>("fragments", storages);

	auto frag_buf_loc = fshad.create_storage_array(fragment_buf_desc);

	auto diff_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("diff_color", uniforms));
	auto spec_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("spec_color", uniforms));
	auto mat_color_uni = fshad.create_uniform(gl::variable<gl::vec4>("mat_color", uniforms));
	auto shininess_uni = fshad.create_uniform(gl::variable<gl::float_t>("shininess", uniforms));

	fshad.set_source(
R"(
in vec3 vertex_normal, norm_light_dir, Ia, E;

layout(early_fragment_tests) in;

void main()
{
	vec3 adjusted_normal = normalize(vertex_normal) * (float(gl_FrontFacing) * 2.0 - 1.0);

	float LambertTerm = max(dot(adjusted_normal, norm_light_dir), 0.0);
	vec3 Id = diff_color.rgb * diff_color.a * LambertTerm;

	vec3 R = reflect(-norm_light_dir, adjusted_normal);
	vec3 Is = spec_color.rgb * spec_color.a * pow(max(dot(R, E), 0.0), shininess);

	vec3 base = mat_color.rgb;

	vec4 final_color = vec4((Ia + Id) * base + Is, mat_color.a);

	const ivec2 pos = ivec2(gl_FragCoord.xy);

	const uint base_index = imageLoad(base_indices, pos).r;
	const uint offset = imageAtomicAdd(layer_counts, pos, 1u);

	//fragments[base_index + offset] = final_color;
	// HAX, using .a as depth
	fragments[base_index + offset] = vec4(final_color.rgb, gl_FragCoord.z);
})"
	);

	auto fshader = fshad.create_shader_program(glc);

	fshader.set_uniform(layer_counts_image_uni, layer_counts_image_unit);
	fshader.set_uniform(base_index_image_uni, base_index_image_unit);

	auto const frag_buf_binding = storage_bindings.get<gl::vec4[]>();

	fshader.set_binding(frag_buf_loc, frag_buf_binding);

	gl::fragment_shader_builder sort_fshad(glc);

	sort_fshad.create_uniform(base_index_image_desc);
	sort_fshad.create_uniform(layer_counts_image_desc);

	sort_fshad.create_storage_array(fragment_buf_desc);

	auto fragdata = sort_fshad.create_output(gl::variable<gl::vec4>("fragdata", fragdatas));

	sort_fshad.set_source(
R"(

vec4 blend(in vec4 dst, in vec4 src)
{
	return vec4(mix(dst.rgb, src.rgb, src.a), src.a);
}

// Sorts based on alpha currently..
#define sort_two(v1, v2) { const float swap = float(v1.a < v2.a); const vec4 tmp = mix(v1, v2, swap); v2 = mix(v2, v1, swap); v1 = tmp; }

void main()
{
	const ivec2 pos = ivec2(gl_FragCoord.xy);

	const uint base_index = imageLoad(base_indices, pos).r;
	const uint count = imageAtomicExchange(layer_counts, pos, 0u);

	vec4 final_color = vec4(0.0);

	if (0 == count)
	{
		discard;
		return;
	}

	// This makes things not slow...
	const int c = int(min(count, 50));

	for (int i = c - 1; i > 0; --i)
	{
		for (int j = 0; j < i; ++j)
		{
			sort_two(fragments[base_index + j], fragments[base_index + j + 1]);
		}
	}

	for (uint i = 0; i != c; ++i)
	{
		//const vec4 col = fragments[base_index + i];
		// HAX, using .a as depth
		vec4 col = fragments[base_index + i];
		col.a = 0.5;

		final_color = blend(final_color, col);
	}

	fragdata = final_color;
})"
	);

	auto sort_fshader = sort_fshad.create_shader_program(glc);

	sort_fshader.set_uniform(layer_counts_image_uni, layer_counts_image_unit);
	sort_fshader.set_uniform(base_index_image_uni, base_index_image_unit);

	sort_fshader.set_binding(frag_buf_loc, frag_buf_binding);

	gl::vertex_shader_builder basic_vshad(glc);

	auto basic_vert_pos_attrib = basic_vshad.create_input(gl::variable<gl::vec2>("pos", attribs));

	basic_vshad.set_source(
R"(
void main()
{
	gl_Position = vec4(pos, 0.0, 1.0);
})"
	);

	auto basic_vshader = basic_vshad.create_shader_program(glc);

	gl::utexture_2d layer_counts_tex(glc), base_indices_tex(glc);

	// load vertex data
	auto verbuf = gl::make_buffer(glc, vertices, gl::buffer_access::none);

	// load index data
	auto indbuf = gl::make_buffer(glc, indices, gl::buffer_access::none);

	// automatically set data types, sizes and strides to components of custom vertex type
	gl::vertex_array arr(glc);

	{
	gl::vertex_buffer_binding_enumerator vbufs(glc);
	auto input_loc = vbufs.get<FooVertex>();

	arr.set_attribute_format(pos_attrib, input_loc | &FooVertex::pos);
	arr.set_attribute_format(norm_attrib, input_loc | &FooVertex::norm);

	arr.set_buffer(input_loc, verbuf.begin());
	arr.set_element_buffer(indbuf);
	}

	gl::buffer<gl::vec2> quad_verbuf(glc);
	quad_verbuf.storage((gl::vec2[])
	{
		{-1, -1}, {-1, 1}, {1, 1}, {1, -1}
	}, gl::buffer_access::none);

	// automatically set data types, sizes, strides
	gl::vertex_array quad_arr(glc);

	{
	gl::vertex_buffer_binding_enumerator vbufs(glc);
	auto input_loc = vbufs.get<gl::vec2>();

	quad_arr.set_attribute_format(basic_vert_pos_attrib, input_loc);

	quad_arr.set_buffer(input_loc, quad_verbuf.begin());
	}

	vshader.set_uniform(ambient_uni, {1, 1, 1, 0.2f});
	vshader.set_uniform(light_dir_uni, {-1, 1, 1});

	fshader.set_uniform(diff_color_uni, {1, 1, 0.75f, 1});
	fshader.set_uniform(spec_color_uni, {1, 1, 1, 0.1f});
	fshader.set_uniform(shininess_uni, 5);

	fshader.set_uniform(mat_color_uni, {0.125f, 0.75f, 0.125f, 0.5f});

	glc.enable(gl::capability::blend);
	//glc.enable(gl::capability::depth_test);
	//glc.enable(gl::capability::cull_face);
	//glc.front_face(gl::orientation::cw);

	glc.blend_equation(gl::blend_mode::add);
	glc.blend_func(gl::blend_factor::src_alpha, gl::blend_factor::inverse_src_alpha);

	{
	gl::framebuffer_builder fbb(glc);
	fbb.bind_draw_buffer(fragdata, gl::default_color_buffer::back_left);

	gl::framebuffer def_fb(glc, nullptr);
	fbb.modify_framebuffer(def_fb);
	}

	// point the model down towards the camera
	auto const pre_rotate = gl::rotate(0.2f, 1.f, 0.f, 0.f);
	// shift the model out from it's rotation point a bit and scale up
	auto const post_rotate = gl::translate(0.1f, -0.8f, 0.f) * gl::scale(8.f, 8.f, 8.f);

	glc.set_clear_depth(1.0f);
	glc.set_clear_color({0.2f, 0.2f, 0.2f, 1});

	gl::mat4 rotate = {};

	gl::query sample_query(glc, gl::query_type::samples_passed);

	gl::program_pipeline pline(glc);
	glc.use_program_pipeline(pline);
	//pline.use_stages(gl::shader_stage::compute, base_index_cshader);

	auto set_up_textures = [&]
	{
		gl::utexture_2d ltex(glc), btex(glc);

		ltex.define_storage(1, counter_format, window_size);
		layer_counts_tex = std::move(ltex);

		const gl::uint_t val = {};
		//layer_counts_tex.clear_sub_image(0, {}, window_size, gl::pixel_format::r, &val);
		layer_counts_tex.clear_image(0, gl::pixel_format::r, &val);

		btex.define_storage(1, counter_format, window_size);
		base_indices_tex = std::move(btex);

		glc.bind_image_texture(layer_counts_image_unit, layer_counts_tex, 0, gl::image_access::read_write, counter_format);
		glc.bind_image_texture(base_index_image_unit, base_indices_tex, 0, gl::image_access::read_write, counter_format);

		glc.bind_image_texture(layer_counts_image_unit, layer_counts_tex, 0, gl::image_access::read_write, counter_format);
		glc.bind_image_texture(base_index_image_unit, base_indices_tex, 0, gl::image_access::read_write, counter_format);
	};

	set_up_textures();

	typedef std::chrono::steady_clock clock;
	auto prev_frame_tp = clock::now();
	auto last_caption_update_tp = prev_frame_tp;

	dsp.set_display_func([&]
	{
		auto const proj =
			gl::perspective(glm::radians(45.f), (float_t)window_size.x / window_size.y, 1.f, 100.f) *
			glm::lookAt(glm::vec3{0.f, 1.f, 2.5f}, glm::vec3{}, glm::vec3{0.f, 1.f, 0.f});

		vshader.set_uniform(projection_uni, proj);

		// rotating model
		gl::mat4 model =
			pre_rotate *
			rotate *
			post_rotate;

		vshader.set_uniform(model_uni, model);

		rotate *= gl::rotate(3.14f / 360, 0.f, 1.f, 0.f);

		glc.depth_mask(true);
		glc.color_mask({true, true, true, true});
		glc.clear(gl::buffer_mask::color | gl::buffer_mask::depth);

		// Count layers
		glc.depth_mask({});
		glc.color_mask({});

		pline.use_stages(gl::shader_stage::vertex, vshader);
		pline.use_stages(gl::shader_stage::fragment, count_layers_fshader);

		sample_query.start();
		glc.draw_elements(arr, gl::primitive::triangles, 0, indices.size());
		sample_query.stop();

		// Compute base index
		glc.memory_barrier(gl::memory_barrier::shader_image_access);
		glc.use_program(base_index_cshader);
		glc.dispatch_compute({1, 1, 1});
		glc.use_program(nullptr);

		//auto const result_available = sample_query.result_available();
		auto const samples_passed = sample_query.result();

		if (samples_passed > 0)
		{
			// Create fragment buffer of proper size
			gl::buffer<gl::vec4, gl::detail::shader_storage_buffer_alignment> dnf(glc);
			dnf.storage(samples_passed, gl::buffer_access::none);

			glc.bind_buffer(frag_buf_binding, dnf.begin(), samples_passed);

			// Render to fragment buffer
			glc.memory_barrier(gl::memory_barrier::shader_image_access);
			pline.use_stages(gl::shader_stage::fragment, fshader);
			glc.draw_elements(arr, gl::primitive::triangles, 0, indices.size());

			// Sort fragments, blend, output
			glc.memory_barrier(gl::memory_barrier::shader_storage);
			glc.depth_mask(true);
			glc.color_mask({true, true, true, true});
			pline.use_stages(gl::shader_stage::vertex, basic_vshader);
			pline.use_stages(gl::shader_stage::fragment, sort_fshader);
			glc.draw_arrays(quad_arr, gl::primitive::triangle_fan, 0, 4);
		}

		//std::cout << "samples_passed: " << samples_passed << std::endl;
		//std::cout << "was_available: " << result_available << " avg layers: " << ((gl::float_t)samples_passed / window_size.x / window_size.y) << std::endl;

		auto const now_tp = clock::now();

		auto const fps = std::chrono::seconds(1) / (now_tp - prev_frame_tp);

		if (now_tp - last_caption_update_tp > std::chrono::seconds(1))
		{
			dsp.set_caption("glwrap-dragon fps:" + std::to_string(fps));
			last_caption_update_tp = now_tp;
		}

		prev_frame_tp = now_tp;
	});

	dsp.set_resize_func([&](gl::ivec2 _size)
	{
		glc.viewport({0, 0}, window_size = _size);

		set_up_textures();
	});

	dsp.run_loop();
}
