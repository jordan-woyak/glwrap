
#include <algorithm>
#include <iostream>

#include "glwrap/gl.hpp"

struct MyUniformBlockData {
  int val1;
  int val2;
};

struct UnboundedMember {
  int p1;
  int p2;
};

struct MyShaderStorageData {
  int data1[12];
  UnboundedMember unbounded_data[];
};

static_assert(sizeof(MyShaderStorageData) == sizeof(int) * 12);

void get_struct_layout(gl::detail::struct_layout<MyUniformBlockData> &sl) {
  sl.set_name("MyUniformBlockData");

  sl.add_member(&MyUniformBlockData::val1, "val1");
  sl.add_member(&MyUniformBlockData::val2, "val2");

  // sl.validate_layout(glc);
};

void get_struct_layout(gl::detail::struct_layout<MyShaderStorageData> &sl) {
  sl.set_name("MyShaderStorageData");

  sl.add_member(&MyShaderStorageData::data1, "data1");
  sl.add_member(&MyShaderStorageData::unbounded_data, "unbounded_data");

  // sl.validate_layout(glc);
};

void get_struct_layout(gl::detail::struct_layout<UnboundedMember> &sl) {
  sl.set_name("UnboundedMember");

  sl.add_member(&UnboundedMember::p1, "p1");
  sl.add_member(&UnboundedMember::p2, "p2");

  // sl.validate_layout(glc);
};

int main() {
  gl::context glc;
  gl::display dsp(glc, {120, 120});
  dsp.set_caption("glwrap-compute");

  glc.enable_profiling();
  glc.enable_debugging();

  std::cout << "Vendor: " << glc.get_vendor_name() << std::endl;
  std::cout << "Renderer: " << glc.get_renderer_name() << std::endl;
  std::cout << "Version: " << glc.get_version() << std::endl;
  std::cout << "GLSL Version: " << glc.get_shading_language_version()
            << std::endl;

  struct MyCounterType {
    gl::uint_t counter1;
    gl::uint_t counter2;
  };

  gl::compute_shader_builder cshad(glc);

  cshad.include_udt_definition<UnboundedMember>();

  gl::uniform_location_enumerator uniforms(glc);
  auto operand1_uni =
      cshad.create_uniform(gl::variable<gl::float_t>("operand1", uniforms));

  gl::atomic_counter_binding_enumerator counters(glc);
  auto counter_loc = counters.get<MyCounterType>();

  cshad.create_uniform(
      gl::variable("counter1", counter_loc | &MyCounterType::counter1));
  cshad.create_uniform(
      gl::variable("counter2", counter_loc | &MyCounterType::counter2));

  gl::shader_storage_location_enumerator storages(glc);
  auto storage_loc = cshad.create_storage_block(
      gl::variable<MyShaderStorageData>("", storages));

  gl::uniform_block_location_enumerator uniform_blocks(glc);
  auto uniblock_loc = cshad.create_uniform_block(
      gl::variable<MyUniformBlockData>("ublock", uniform_blocks));

  cshad.set_source(
      R"(

layout(local_size_x = 100, local_size_y = 10) in;

void main(void)
{
	if (0 == gl_LocalInvocationIndex)
	{
		//uint i = data1.length();
		atomicExchange(data1[0], 5);

		data1[1] = data1.length();

		//data1[0] = ublock.val1;
		//data1[1] = ublock.val2;

		data1[2] = ublock.val1;
		data1[3] = ublock.val2;

    data1[4] = unbounded_data.length();
	}

	if (gl_LocalInvocationIndex < operand1)
	{
		atomicCounterIncrement(counter1);

		// Just testing stuff
		memoryBarrier();
		
		atomicCounterDecrement(counter2);
	}
})");

  auto comp_shader = cshad.create_shader(glc);
  std::cout << "cshad log:\n" << comp_shader.get_log() << std::endl;
  if (!comp_shader.compile_status())
    std::cout << "cshad src:\n" << comp_shader.get_source() << std::endl;

  gl::program prog(glc);
  prog.attach(comp_shader);

  gl::shader_storage_binding_enumerator ssbindings(glc);
  auto ssb = ssbindings.get<MyShaderStorageData>();

  gl::uniform_block_binding_enumerator ubbindings(glc);
  auto ubb = ubbindings.get<MyUniformBlockData>();

  prog.link();

  prog.set_binding(storage_loc, ssb);
  prog.set_binding(uniblock_loc, ubb);

  std::cout << "program log:\n" << prog.get_log() << std::endl;

  // if (!prog.is_good())
  //	return 1;

  gl::buffer_pool bpool(glc, gl::buffer_access::dynamic_storage |
                                 gl::buffer_access::map_read);

  // bpool.get<gl::int_t>(45);
  // bpool.get<gl::int_t>(45);
  // bpool.get<gl::int_t>(45);
  // bpool.get<gl::int_t>(1024*1024);
  // bpool.get<gl::int_t>(1024);

  std::vector<gl::float_t> operands = {0, 250, 112, 723, 300, 100};

  constexpr int unbounded_member_count = 30;
  constexpr int shader_storage_byte_count =
      sizeof(MyShaderStorageData) +
      sizeof(UnboundedMember) * unbounded_member_count;

  // Create shader storage
  auto storage_buffer =
      bpool.get_exact_size<MyShaderStorageData,
                           gl::detail::shader_storage_buffer_alignment>(
          shader_storage_byte_count);
  storage_buffer.assign_range({MyShaderStorageData{}}, 0);
  glc.bind_buffer_exact_size(ssb, storage_buffer.begin(),
                             shader_storage_byte_count);

  // Create counter buffer
  auto counter_buffer = bpool.get<MyCounterType>(operands.size());
  counter_buffer.assign_range(std::vector<MyCounterType>(operands.size()), 0);

  // Create uniform block storage
  auto uniform_buffer =
      bpool.get<MyUniformBlockData, gl::detail::uniform_buffer_alignment>(1);
  uniform_buffer.assign_range({{72, 31}}, 0);
  glc.bind_buffer(ubb, uniform_buffer.begin());

  auto cmdbuf = bpool.get<gl::uvec3>(1);
  cmdbuf.assign_range({{5, 1, 1}}, 0);

  bpool.debug_stats();

  // Compute
  glc.use_program(prog);

  auto counter_buf_iter = counter_buffer.begin();

  for (auto operand : operands) {
    prog.set_uniform(operand1_uni, operand);
    glc.bind_buffer(counter_loc, counter_buf_iter);

    glc.dispatch_compute({5, 1, 1});
    glc.dispatch_compute_indirect(cmdbuf.begin());

    counter_buf_iter += 1;
  }

  std::cout << "DONE" << std::endl << std::endl;

  // print results
  for (auto &counters : counter_buffer.get_range(0, operands.size()))
    std::cout << counters.counter1 << "\t" << counters.counter2 << std::endl;

  std::cout << std::endl;

  glc.memory_barrier(gl::memory_barrier::buffer_update);

  auto rng = storage_buffer.get_range(0, 1);
  for (auto &datum : rng.front().data1)
    std::cout << datum << ' ';

  std::cout << std::endl;
}
