
#include <iostream>
#include <algorithm>

#include "glwrap/gl.hpp"

struct MyUniformBlockData
{
	int val1;
	int val2;
};

void get_struct_layout(gl::detail::struct_layout<MyUniformBlockData>& sl)
{
	//sl.set_name("MyUniformBlockData");
	
	sl.add_member(&MyUniformBlockData::val1, "val1");
	sl.add_member(&MyUniformBlockData::val2, "val2");
	
	//sl.validate_layout(glc);
};

int main()
{
	//glewExperimental = true;

	gl::context glc;
	gl::display dsp(glc, {120, 120});
	dsp.set_caption("glwrap-compute");

	glc.enable_debugging();

	std::cout << "Vendor: " << glc.get_vendor_name() << std::endl;
	std::cout << "Renderer: " << glc.get_renderer_name() << std::endl;
	std::cout << "Version: " << glc.get_version() << std::endl;
	std::cout << "GLSL Version: " << glc.get_shading_language_version() << std::endl;

	/*template <typename T>
	struct struct_layout<MyFoo>
	{
		static const int i = 0;
	};*/

	struct MyCounterType
	{
		gl::uint_t counter1;
		gl::uint_t counter2;
	};

#if 0
	struct MyInput
	{
		gl::layout::std140<gl::float_t> input1;
		gl::layout::std140<gl::vec2> input2;
		gl::layout::std140<gl::vec3> input3;

		//gl::layout::std140<gl::mat4> input4;
	};
#endif

	gl::compute_shader_builder cshad(glc);

	gl::uniform_location_enumerator uniforms(glc);
	auto operand1_uni = cshad.create_uniform(gl::variable<gl::float_t>("operand1", uniforms));

	gl::atomic_counter_binding_enumerator counters(glc);
	auto counter_loc = counters.get<MyCounterType>();

	cshad.create_uniform(gl::variable("counter1", counter_loc | &MyCounterType::counter1));
	cshad.create_uniform(gl::variable("counter2", counter_loc | &MyCounterType::counter2));

	gl::shader_storage_location_enumerator storages(glc);
	//auto storage_loc = storages.get<gl::int_t>();

	auto storage_loc = cshad.create_storage(gl::variable<gl::int_t[]>("data1", storages));

	gl::uniform_block_location_enumerator uniform_blocks(glc);

	auto uniblock_loc = cshad.create_uniform_block(gl::variable<MyUniformBlockData>("", uniform_blocks));

	cshad.set_source(
R"(

//buffer int foo[2];

layout(local_size_x = 100, local_size_y = 10) in;

void main(void)
{
	if (0 == gl_LocalInvocationIndex)
	{
		//uint i = data1.length();
		atomicExchange(data1[0], 5);

		data1[1] = data1.length();

		data1[2] = val1;
		data1[3] = val2;
	}

	if (gl_LocalInvocationIndex < operand1)
	{
		atomicCounterIncrement(counter1);

		// Just testing stuff
		memoryBarrier();
		
		atomicCounterDecrement(counter2);
	}
})"
	);

	auto comp_shader = cshad.create_shader(glc);
	std::cout << "cshad log:\n" << comp_shader.get_log() << std::endl;
	if (!comp_shader.compile_status())
		std::cout << "cshad src:\n" << comp_shader.get_source() << std::endl;
	
	gl::program prog(glc);
	prog.attach(comp_shader);
	prog.link();
	std::cout << "program log:\n" << prog.get_log() << std::endl;

	if (!prog.is_good())
		return 1;

	std::vector<gl::float_t> operands =
	{
		0, 250, 112, 723, 300, 100
	};

	gl::buffer<MyCounterType> counter_buffer(glc);
	counter_buffer.assign(std::vector<MyCounterType>(operands.size()), gl::buffer_usage::static_read);

	// Create shader storage
	gl::buffer<gl::int_t> storage_buffer(glc);
	storage_buffer.assign(std::vector<gl::int_t>(32), gl::buffer_usage::static_read);
	glc.bind_buffer(storage_loc, storage_buffer.begin(), 32);

	// Create uniform block storage
	gl::buffer<MyUniformBlockData, gl::detail::uniform_buffer_alignment> uniform_buffer(glc);
	uniform_buffer.assign(std::vector<MyUniformBlockData>{{72,31}}, gl::buffer_usage::static_draw);
	glc.bind_buffer(uniblock_loc, uniform_buffer.begin());

	gl::buffer<gl::uvec3> cmdbuf(glc);
	cmdbuf.storage(1, gl::buffer_usage::static_draw);
	cmdbuf.assign_range((gl::uvec3[]){ {5, 1, 1} }, 0);

	// Compute
	glc.use_program(prog);

	auto counter_buf_iter = counter_buffer.begin();

	for (auto operand : operands)
	{
		prog.set_uniform(operand1_uni, operand);
		glc.bind_buffer(counter_loc, counter_buf_iter);
		
		//glc.dispatch_compute({5, 1, 1});
		glc.dispatch_compute_indirect(cmdbuf.begin());

		counter_buf_iter += 1;
	}

	std::cout << "DONE" << std::endl << std::endl;

	// print results
	for (auto& counters : gl::map_buffer(counter_buffer))
		std::cout << counters.counter1 << "\t" << counters.counter2 << std::endl;

	std::cout << std::endl;

	glc.memory_barrier(gl::memory_barrier::buffer_update);

	for (auto& datum : gl::map_buffer(storage_buffer))
		std::cout << datum << ' ';

	std::cout << std::endl;
}
