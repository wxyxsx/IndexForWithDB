#pragma once

#define N 3

namespace db
{
	class nodes
	{
	public:
		int num;
		int key[N];

	public:
		void init();
	};

	class nonleaf : public nodes
	{
	public:
		void* pointer[N + 1];
		bool if_plf;  //  «∑Ò÷∏œÚleaf
		//	bool is_root;

	public:
		void init();
	};

	class leaf : public nodes
	{
	public:
		int value[N];
		leaf* next;

	public:
		void init();
	};

}