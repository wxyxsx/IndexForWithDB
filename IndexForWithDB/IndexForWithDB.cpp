﻿#include "pch.h"

using namespace std;

int main()
{
	db::bplustree tr;
	int a[100] = { 89, 82, 74, 22, 27, 76, 17, 72, 98, 52,
		58, 86, 11, 65, 12, 91, 30, 54, 19, 77, 61, 92,
		67, 18, 100, 20, 9, 99, 51, 26, 97, 38, 25, 32,
		3, 75, 71, 80, 47, 88, 85, 83, 40, 84, 69, 31, 13,
		66, 14, 94, 28, 59, 96, 41, 55, 1, 63, 73, 4, 68, 7,
		62, 44, 49, 78, 15, 90, 45, 16, 93, 5, 56, 29, 48, 21,
		46, 43, 95, 36, 39, 87, 53, 50, 6, 81, 42, 79, 23, 37,
		10, 60, 64, 57, 33, 70, 34, 35, 8, 2, 24 };
	tr.create(a, 100);
	tr.print_tree();
	tr.print_leaf();
	//printf("%d,%d,%d\n",tr.search(56), tr.search(135), tr.search(123));
}