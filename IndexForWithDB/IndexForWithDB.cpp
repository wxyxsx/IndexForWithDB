#include "pch.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>

using namespace std;

int main()
{
	db::btree* tr = new db::btree();
	vector<int> arr;
	for (int i = 1;i < 501;i++) arr.push_back(i);
	random_shuffle(arr.begin(), arr.end());
	int v = 1;
	for (vector<int>::iterator it = arr.begin();it != arr.end();++it) {
		tr->insert(*it, v);
		v++;
	}
	tr->print_tree();
	random_shuffle(arr.begin(), arr.end());
	v = 1;
	for (vector<int>::iterator it = arr.begin();it != arr.end();++it) {
		tr->delkey(*it);
		v++;
		if (v % 100 == 0) {
			tr->print_tree();
			tr->print_leaf();
		}
	}
	random_shuffle(arr.begin(), arr.end());
	v = 1;
	for (vector<int>::iterator it = arr.begin();it != arr.end();++it) {
		tr->insert(*it, v);
		v++;
	}
	tr->print_tree();
	return 0;
}