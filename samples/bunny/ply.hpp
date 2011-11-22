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

template <typename T>
class vertex_format
{
public:
	// function callback is lameish
	typedef std::function<void(std::istream&,T&)> callback_type;

	void bind(std::string const& _name, callback_type const& _func)
	{
		m_callbacks[_name] = _func;
	}

	callback_type get_callback(std::string const& _name) const
	{
		auto const f = m_callbacks.find(_name);

		if (f != m_callbacks.end())
			return f->second;
		else
			return callback_type{};
	}

private:
	std::map<std::string, callback_type> m_callbacks;
};

template <typename T, typename I>
void load(std::string const& _filename, vertex_format<T> const& _fmt,
	std::vector<T>& _vertex_data, std::vector<I>& _index_data)
{
	std::ifstream file(_filename);

	std::function<void()> propery_handler;

	// TODO: not generic
	std::size_t vert_count{}, face_count{};

	std::vector<typename vertex_format<T>::callback_type> vert_handlers;

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
				_vertex_data.reserve(vert_count);

				propery_handler = [&]
				{
					std::string name;
					line_stream >> name >> name;	// skip type :/
					vert_handlers.push_back(_fmt.get_callback(name));
				};
			}
			else if ("face" == cmd)
			{
				line_stream >> face_count;
				_index_data.reserve(face_count);

				propery_handler = []{};
			}
		}
		else if ("property" == cmd)
		{
			propery_handler();
		}
		else if ("end_header" == cmd)
		{
			break;
		}
	}

	while (vert_count--)
	{
		std::getline(file, line);
		std::istringstream line_stream(std::move(line));

		T vert{};

		std::string dummy;
		for (auto& handler : vert_handlers)
			if (handler)
				handler(line_stream, vert);
			else
				line_stream >> dummy;

		_vertex_data.push_back(vert);
	}

	while (face_count--)
	{
		std::getline(file, line);
		std::istringstream line_stream(std::move(line));

		// TODO: not reading faces properly, just assuming triangles..
		int face_vert_count = 0;
		line_stream >> face_vert_count;

		// TODO: hax for 3 and 4 vert faces
		if (3 == face_vert_count)
			std::copy_n(std::istream_iterator<I>(line_stream), 3, std::back_inserter(_index_data));
		else if (4 == face_vert_count)
		{
			std::vector<I> tmp;
			std::copy_n(std::istream_iterator<I>(line_stream), 4, std::back_inserter(tmp));

			_index_data.insert(_index_data.end(), tmp.begin(), tmp.end());
			_index_data.push_back(tmp[0]);
			_index_data.push_back(tmp[2]);
		}
	}
}

}
