#pragma once

#include <string>
#include <initializer_list>

using namespace std;

class mesh_lib
{
public:
	void init(const initializer_list<string>& args) noexcept;
};
