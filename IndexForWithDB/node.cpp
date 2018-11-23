#include "pch.h"
#include<vector>

namespace db
{
	void nodes::init() {
		num = 0;
	}

	void nonleaf::init() {
		for (int i = 0;i < N;i++)
			key[i] = 0;
		for (int i = 0;i < N + 1;i++)
			pointer[i] = NULL;
		num = 0;
		if_plf = false;
	}

	void leaf::init() {
		for (int i = 0;i < N;i++) {
			key[i] = 0;
			value[i] = 0;
		}
		num = 0;
		next = NULL;
	}
}