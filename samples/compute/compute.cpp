
#include <iostream>
#include <algorithm>

#include "glwrap/gl.hpp"

int main()
{
	glewExperimental = true;

	gl::context glc;
	gl::display dsp(glc, {120, 120});
	dsp.set_caption("glwrap-compute");

	std::cout << "Vendor: " << glc.get_vendor_name() << std::endl;
	std::cout << "Renderer: " << glc.get_renderer_name() << std::endl;
	std::cout << "Version: " << glc.get_version() << std::endl;
	std::cout << "GLSL Version: " << glc.get_shading_language_version() << std::endl;

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

	cshad.set_source(
R"(

//buffer int foo[2];

layout(local_size_x = 100, local_size_y = 10) in;

void main(void)
{
	if (gl_LocalInvocationIndex < operand1)
	{
		atomicCounterIncrement(counter1);
		atomicCounterDecrement(counter2);
	}
})"
	);

	auto comp_shader = cshad.create_shader(glc);
	std::cout << "cshad log:\n" << comp_shader.get_log() << std::endl;
	//std::cout << "cshad src:\n" << comp_shader.get_source() << std::endl;
	
	gl::program prog(glc);
	prog.attach(comp_shader);
	prog.link();
	std::cout << "program log:\n" << prog.get_log() << std::endl;

	std::vector<gl::float_t> operands =
	{
		0, 250, 112, 723, 300, 100
	};

	gl::buffer<MyCounterType> counter_buffer(glc);
	counter_buffer.assign(std::vector<MyCounterType>(operands.size()), gl::buffer_usage::static_read);

	// Compute
	glc.use_program(prog);

	auto counter_buf_iter = counter_buffer.begin();

	for (auto operand : operands)
	{
		prog.set_uniform(operand1_uni, operand);
		glc.bind_buffer(counter_loc, counter_buf_iter);
		
		glc.dispatch_compute({5, 1, 1});

		counter_buf_iter += 1;
	}

	std::cout << "DONE" << std::endl << std::endl;

	// print results
	for (auto& counters : gl::map_buffer(counter_buffer))
		std::cout << counters.counter1 << "\t" << counters.counter2 << std::endl;
}
