
#pragma once

namespace gl
{

template <int D>
class sampler : public globject
{
public:
	sampler(context& _context)
		: globject(gen_return(glGenSamplers))
	{}

	~sampler()
	{
		auto const nh = native_handle();
		glDeleteSamplers(1, &nh);
	}
};

typedef sampler<1> sampler_1d;
typedef sampler<2> sampler_2d;
typedef sampler<3> sampler_3d;

}

