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
		void init() {
			num = 0;
		}
	};

	class nonleaf : public nodes
	{
	public:
		void* pointer[N + 1];
		bool if_plf;  //  «∑Ò÷∏œÚleaf
		//	bool is_root;

	public:
		void init() {
			for (int i = 0;i < N;i++)
				key[i] = 0;
			for (int i = 0;i < N + 1;i++)
				pointer[i] = nullptr;
			num = 0;
			if_plf = false;
		}
	};

	class leaf : public nodes
	{
	public:
		int value[N];
		leaf* next;

	public:
		void init() {
			for (int i = 0;i < N;i++) {
				key[i] = 0;
				value[i] = 0;
			}
			num = 0;
			next = nullptr;
		}
	};

}