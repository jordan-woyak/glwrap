#pragma once

namespace gl
{

enum class capability : GLenum
{
	blend = GL_BLEND,
	//color_logic_op = GL_COLOR_LOGIC_OP,
	cull_face = GL_CULL_FACE,
	//depth_clamp = GL_DEPTH_CLAMP,
	depth_test = GL_DEPTH_TEST,
	dither = GL_DITHER,
	//line_smooth = GL_LINE_SMOOTH,
	//multisample = GL_MULTISAMPLE,

	polygon_offset_fill = GL_POLYGON_OFFSET_FILL,
	//polygon_offset_line = GL_POLYGON_OFFSET_LINE,
	//polygon_offset_point = GL_POLYGON_OFFSET_POINT,
	//polygon_smooth = GL_POLYGON_SMOOTH,

	primitive_restart_fixed_index = GL_PRIMITIVE_RESTART_FIXED_INDEX,

	rasterizer_discard = GL_RASTERIZER_DISCARD,

	sample_alpha_to_coverage = GL_SAMPLE_ALPHA_TO_COVERAGE,
	//sample_alpha_to_one = GL_SAMPLE_ALPHA_TO_ONE,
	sample_coverage = GL_SAMPLE_COVERAGE,
	sample_mask = GL_SAMPLE_MASK,

	scissor_test = GL_SCISSOR_TEST,
	stencil_test = GL_STENCIL_TEST,

	//texture_cube_map_seamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,
	//program_point_size = GL_PROGRAM_POINT_SIZE
};

enum class filter : GLenum
{
	nearest = GL_NEAREST,
	linear = GL_LINEAR
};

enum class provoke_mode : GLenum
{
	first = GL_FIRST_VERTEX_CONVENTION,
	last = GL_LAST_VERTEX_CONVENTION,
};

enum class stencil_action : GLenum
{
	keep = GL_KEEP,
	zero = GL_ZERO,
	replace = GL_REPLACE,
	increment = GL_INCR,
	increment_wrap = GL_INCR_WRAP,
	decrement = GL_DECR,
	decrement_wrap = GL_DECR_WRAP,
	invert = GL_INVERT
};

enum class comparison : GLenum
{
	never = GL_NEVER,
	less = GL_LESS,
	less_equal = GL_LEQUAL,
	greater = GL_GREATER,
	greater_equal = GL_GEQUAL,
	equal = GL_EQUAL,
	not_equal = GL_NOTEQUAL,
	always = GL_ALWAYS
};

enum class face : GLenum
{
	back = GL_BACK,
	front = GL_FRONT,
	both = GL_FRONT_AND_BACK
};

enum class orientation : GLenum
{
	cw = GL_CW,
	ccw = GL_CCW
};

enum class blend_mode : GLenum
{
	add = GL_FUNC_ADD,
	subtract = GL_FUNC_SUBTRACT,
	reverse_subtract = GL_FUNC_REVERSE_SUBTRACT,
	min = GL_MIN,
	max = GL_MAX
};

enum class blend_factor : GLenum
{
	zero = GL_ZERO,
	one = GL_ONE,
	src_color = GL_SRC_COLOR,
	inverse_src_color = GL_ONE_MINUS_SRC_COLOR,
	dst_color = GL_DST_COLOR,
	inverse_dst_color = GL_ONE_MINUS_DST_COLOR,
	src_alpha = GL_SRC_ALPHA,
	inverse_src_alpha = GL_ONE_MINUS_SRC_ALPHA,
	dst_alpha = GL_DST_ALPHA,
	inverse_dst_alpha = GL_ONE_MINUS_DST_ALPHA,
	constant_color = GL_CONSTANT_COLOR,
	inverse_constant_color = GL_ONE_MINUS_CONSTANT_COLOR,
	constant_alpha = GL_CONSTANT_ALPHA,
	inverse_constant_alpha = GL_ONE_MINUS_CONSTANT_ALPHA,
	src_alpha_saturate = GL_SRC_ALPHA_SATURATE,

	// dual source blending:
	src1_color = GL_SRC1_COLOR,
	inverse_src1_color = GL_ONE_MINUS_SRC1_COLOR,
	src1_alpha = GL_SRC1_ALPHA,
	inverse_src1_alpha = GL_ONE_MINUS_SRC1_ALPHA,
};

enum class color_buffer : GLenum
{
	none = GL_NONE,
	front_left = GL_FRONT_LEFT,
	front_right = GL_FRONT_RIGHT,
	back_left = GL_BACK_LEFT,
	back_right = GL_BACK_RIGHT
};

enum class primitive : GLenum
{
	points = GL_POINTS,
	line_strip = GL_LINE_STRIP,
	line_loop = GL_LINE_LOOP,
	lines = GL_LINES,
	//line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
	//lines_adjacency = GL_LINES_ADJACENCY,
	triangle_strip = GL_TRIANGLE_STRIP,
	triangle_fan = GL_TRIANGLE_FAN,
	triangles = GL_TRIANGLES,
	//triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
	//triangles_adjacency = GL_TRIANGLES_ADJACENCY
};

enum class hint_target : GLenum
{
	//line_smooth = GL_LINE_SMOOTH_HINT,
	//polygon_smooth = GL_POLYGON_SMOOTH_HINT,
	//texture_compression = GL_TEXTURE_COMPRESSION_HINT,
	fragment_shader_derivative = GL_FRAGMENT_SHADER_DERIVATIVE_HINT,
	//generate_mipmap = GL_GENERATE_MIPMAP_HINT, // ES only
};

enum class hint_value : GLenum
{
	fastest = GL_FASTEST,
	nicest = GL_NICEST,
	dont_care = GL_DONT_CARE
};

enum class sync_condition : GLenum
{
	gpu_commands_complete = GL_SYNC_GPU_COMMANDS_COMPLETE
};

// TODO: allow bitwise combinations:
enum class memory_barrier : GLenum
{
	vertex_attrib_array = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT,
	element_array = GL_ELEMENT_ARRAY_BARRIER_BIT,
	uniform = GL_UNIFORM_BARRIER_BIT,
	texture_fetch = GL_TEXTURE_FETCH_BARRIER_BIT,
	shader_image_access = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT,
	command = GL_COMMAND_BARRIER_BIT,
	pixel_buffer = GL_PIXEL_BUFFER_BARRIER_BIT,
	texture_update = GL_TEXTURE_UPDATE_BARRIER_BIT,
	buffer_update = GL_BUFFER_UPDATE_BARRIER_BIT,
	framebuffer = GL_FRAMEBUFFER_BARRIER_BIT,
	transoform_feedback = GL_TRANSFORM_FEEDBACK_BARRIER_BIT,
	atomic_counter = GL_ATOMIC_COUNTER_BARRIER_BIT,
	shader_storage = GL_SHADER_STORAGE_BARRIER_BIT,

	all = GL_ALL_BARRIER_BITS,
};

}
