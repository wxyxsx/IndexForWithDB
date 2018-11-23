#pragma once

#include<vector>

#define N 3

namespace db
{
	class nodes
	{
	public:
		int num;
		std::vector<int> key;

	public:
		nodes(void) {
			num = 0;
		}
	};

	class nonleaf : public nodes
	{
	public:
		std::vector<void*> pointer;
		bool if_plf;  //  «∑Ò÷∏œÚleaf
		//	bool is_root;

	public:
		nonleaf(void) {
			if_plf = false;
		}
	};

	class leaf : public nodes
	{
	public:
		std::vector<int> value;
		leaf* next;

	public:
		leaf(void) {
			next = NULL;
		}
	};

}