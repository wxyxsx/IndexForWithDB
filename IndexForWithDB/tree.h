#pragma once

#include"node.h"

namespace db {
	class bplustree
	{
	private:
		nonleaf* root;

	private:
		leaf* split_leaf(leaf* ori, int key, int value);
		nonleaf* split_nonleaf(nonleaf* ori, int key, void* pointer);

		void ins_leaf(leaf* ori, int key, int value);
		void ins_nonleaf(nonleaf* ori, int key, void* pointer);

		void span_ins_lf(leaf* a, leaf* b, int key, int value, int index);
		void span_ins_nlf(nonleaf* a, nonleaf* b, int key, void* pointer, int index);

		int search_index(nodes* node, int key);
		int search_left(nonleaf* node);

		void print_space(int level);
		void print_leaf(leaf* node);
		void print_nonleaf(nonleaf* node, int level);

	public:
		bplustree(void);
		void create(int* arr, int len);
		int search(int key);
		void insert(int key, int value);
		void print_tree();
		void print_leaf();
	};
}