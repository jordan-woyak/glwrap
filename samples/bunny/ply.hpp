#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <map>

namespace ply
{

template <typename T, typename M>
constexpr std::intptr_t get_member_offset(M T::*_member)
{
	// TODO: not portable
	return reinterpret_cast<std::intptr_t>(&(static_cast<const T*>(nullptr)->*_member));
}

template <typename T>
struct attrib_binder
{
	typedef std::size_t offset_type;

	offset_type& offset;
};

template <typename T, typename P, typename M>
attrib_binder<M> operator|(attrib_binder<T>&& _binder, M P::*_member)
{
	_binder.offset += get_member_offset(_member);
	return attrib_binder<M>{_binder.offset};
}

template <typename T>
class vertex_format
{
public:
	typedef typename attrib_binder<T>::offset_type offset_type;

	attrib_binder<T> bind(std::string const& _name)
	{
		return attrib_binder<T>{m_offsets[_name]};
	}

	offset_type get_offset(std::string const& _name) const
	{
		auto const f = m_offsets.find(_name);

		if (f != m_offsets.end())
			return f->second;
		else
			return sizeof(T);
	}

private:
	std::map<std::string, offset_type> m_offsets;
};

template <typename T, typename I>
void load(std::string const& _filename, vertex_format<T> const& _fmt,
	std::vector<T>& _vertex_data, std::vector<I>& _index_data)
{
	std::ifstream file(_filename);

	std::function<void()> property_handler;

	// TODO: not generic
	std::size_t vert_count{}, face_count{};

	std::vector<typename vertex_format<T>::offset_type> attrib_offsets;

	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream line_stream(std::move(line));
		std::string cmd;
		line_stream >> cmd;

		if ("element" == cmd)
		{
			line_stream >> cmd;

			if ("vertex" == cmd)
			{
				line_stream >> vert_count;

				property_handler = [&]
				{
					std::string name;
					line_stream >> name >> name;	// skip type :/
					attrib_offsets.push_back(_fmt.get_offset(name));
				};
			}
			else if ("face" == cmd)
			{
				line_stream >> face_count;

				property_handler = []{};
			}
		}
		else if ("property" == cmd)
		{
			property_handler();
		}
		else if ("end_header" == cmd)
		{
			break;
		}
	}

	_vertex_data.reserve(vert_count);
	while (vert_count--)
	{
		// Un-binded offsets are size to sizeof(T) and written to dummy value.
		struct
		{
			T val {};
			float dummy;
		} vert;

		for (auto offset : attrib_offsets)
		{
			file >> *reinterpret_cast<float*>(reinterpret_cast<char*>(&vert) + offset);
		}

		_vertex_data.push_back(vert.val);
	}

	_index_data.reserve(face_count);
	while (face_count--)
	{
		// TODO: not reading faces properly, just assuming triangles..
		int face_vert_count = 0;
		file >> face_vert_count;

		// TODO: hax for 3 and 4 vert faces
		if (3 == face_vert_count)
			std::copy_n(std::istream_iterator<I>(file), 3, std::back_inserter(_index_data));
		else if (4 == face_vert_count)
		{
			std::vector<I> tmp;
			std::copy_n(std::istream_iterator<I>(file), 4, std::back_inserter(tmp));

			_index_data.insert(_index_data.end(), tmp.begin(), tmp.end());
			_index_data.push_back(tmp[0]);
			_index_data.push_back(tmp[2]);
		}
	}
}

}
