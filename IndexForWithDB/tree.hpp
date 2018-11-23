#pragma once

#include"node.hpp"
#include<iostream>
#include<stack>

namespace db {
	class bplustree
	{
	private:
		nonleaf* root;

		//private:
		//	leaf* split_leaf(leaf* ori, int key, int value);
		//	nonleaf* split_nonleaf(nonleaf* ori, int key, void* pointer);

		//	void ins_leaf(leaf* ori, int key, int value);
		//	void ins_nonleaf(nonleaf* ori, int key, void* pointer);

		//	void span_ins_lf(leaf* a, leaf* b, int key, int value, int index);
		//	void span_ins_nlf(nonleaf* a, nonleaf* b, int key, void* pointer, int index);

		//	int search_index(nodes* node, int key);
		//	int search_left(nonleaf* node);

		//	void print_space(int level);
		//	void print_leaf(leaf* node);
		//	void print_nonleaf(nonleaf* node, int level);

	private:
		void span_ins_lf(leaf* a, leaf* b, int key, int value, int index) {
			int i = index - a->num;
			if (i < 0) { // 原节点
				a->key[index] = key;
				a->value[index] = value;
			}
			else { // 新节点
				b->key[i] = key;
				b->value[i] = value;
			}
		}

		void span_ins_nlf(nonleaf* a, nonleaf* b, int key, void* pointer, int index) {
			int i = index - a->num;
			if (i < 0) {
				a->key[index] = key;
				a->pointer[index + 1] = pointer;
			}
			else {
				b->key[i] = key;
				b->pointer[i + 1] = pointer;
			}
		}

		// 查找k在新数组中的位置
		int search_index(nodes* node, int key) {
			int r = node->num; // 默认指向最右端
			for (int i = 0;i < node->num;i++) {
				if (key < node->key[i]) {
					r = i;
					break;
				}
			}
			return r;
		}

		// 查找节点n最左端叶节点的第一个值
		int search_left(nonleaf* node) {
			nonleaf* r = node;
			while (!r->if_plf)
				r = (nonleaf*)r->pointer[0];
			int key = ((leaf*)r->pointer[0])->key[0];
			return key;
		}

		// 节点有足够的空间插入新元素
		void ins_leaf(leaf* ori, int key, int value) {
			int r = search_index(ori, key);

			for (int i = ori->num;i > r;i--) {
				ori->key[i] = ori->key[i - 1];
				ori->value[i] = ori->value[i - 1];
			}
			ori->key[r] = key;
			ori->value[r] = value;
			ori->num++;
		}

		void ins_nonleaf(nonleaf* ori, int key, void* pointer) {
			int r = search_index(ori, key);

			for (int i = ori->num;i > r;i--) {
				ori->key[i] = ori->key[i - 1];
				ori->pointer[i + 1] = ori->pointer[i];
			}
			ori->key[r] = key;
			ori->pointer[r + 1] = pointer;
			ori->num++;
		}

		// 节点数据超过N，返回新节点的地址
		leaf* split_leaf(leaf* ori, int key, int value) {
			int r = search_index(ori, key);

			leaf* nlf = (leaf*)malloc(sizeof(leaf));
			nlf->init();
			nlf->num = (int)((N + 1) / 2); // 新的叶节点至少有[(n+1)/2]个key
			ori->num = N + 1 - nlf->num; // N+1是key的总数
			nlf->next = ori->next;
			ori->next = nlf;

			for (int i = N;i > r;i--) // 比key大的右移一位
				span_ins_lf(ori, nlf, ori->key[i - 1], ori->value[i - 1], i);
			span_ins_lf(ori, nlf, key, value, r);
			for (int i = r - 1;i > ori->num - 1;i--) // 比key小的k可能要迁移到新的节点
				span_ins_lf(ori, nlf, ori->key[i], ori->value[i], i);

			return nlf;
		}

		nonleaf* split_nonleaf(nonleaf* ori, int key, void* pointer) {
			int r = search_index(ori, key);

			nonleaf* nnlf = (nonleaf*)malloc(sizeof(nonleaf));
			nnlf->init();
			nnlf->num = (int)((N + 1) / 2) - 1;
			ori->num = N + 1 - nnlf->num;
			/*
			原来有3个键分裂之后为2+1，因为新的节点不能有NULL，需要删除一个key，把它的右端作为新节点的左端
			但为了span_ins_nlf的准确性，先移动再删除
			*/
			nnlf->if_plf = ori->if_plf;

			for (int i = N;i > r;i--)
				span_ins_nlf(ori, nnlf, ori->key[i - 1], ori->pointer[i], i);
			span_ins_nlf(ori, nnlf, key, pointer, r);
			for (int i = r - 1;i > ori->num - 1;i--)
				span_ins_nlf(ori, nnlf, ori->key[i], ori->pointer[i + 1], i);

			// 把原节点最右端的key删除，p放到新节点最左端
			nnlf->pointer[0] = ori->pointer[ori->num];
			ori->num--;

			return nnlf;
		}

		void print_space(int level) {
			for (int i = 1;i < 6 * level;i++) {
				if (i % 6 == 0) printf("|");
				else printf(" ");
			}
			if (level != 0) printf("+");
		}

		// 依次输出kv并换行
		void print_leaf(leaf* node) {
			if (node != NULL) {
				for (int i = 0;i < node->num;i++)
					printf("%-3d[%-3d],", node->key[i], node->value[i]);
			}
			printf("\n");
		}

		// 采用深度优先遍历
		void print_nonleaf(nonleaf* node, int level) {
			for (int i = 0;i < node->num + 1;i++) {
				if (i != 0) print_space(level);
				// 第一个元素不用缩进

				if (i != node->num) printf("%-3d--+", node->key[i]);
				else printf("   --+");

				if (node->if_plf) print_leaf((leaf*)node->pointer[i]);
				else print_nonleaf((nonleaf*)node->pointer[i], level + 1);
			}
		}

		//public:
		//	bplustree(void);
		//	void create(int* arr, int len);
		//	int search(int key);
		//	void insert(int key, int value);
		//	void print_tree();
		//	void print_leaf();

	public:
		bplustree(void) {
			root = NULL;
		}

		void create(int* arr, int len) {
			root = (nonleaf*)malloc(sizeof(nonleaf));
			root->init();
			root->if_plf = true;

			for (int i = 0;i < len;i++) {
				insert(arr[i], i);
				//print_tree();
			}


		}

		int search(int key) {
			int res = -1;
			if (root == NULL) return res;
			// root节点可能为空

			nonleaf* p = root;
			leaf* lf = NULL;

			while (true) {
				int t = search_index(p, key);
				// 如果不指向叶节点，继续向下找
				if (!p->if_plf) p = (nonleaf*)p->pointer[t];
				else {
					lf = (leaf*)p->pointer[t];
					break;
				}
			}

			if (lf != NULL) { // 考虑root节点左节点可能为NUL
				for (int i = 0;i < lf->num;i++) {
					if (key == lf->key[i]) {
						res = lf->value[i];
						break;
					}
				}
			}

			return res;
		}

		void insert(int key, int value) {
			if (root->num == 0) { // root节点可能为空
				leaf* nlf = (leaf*)malloc(sizeof(leaf));
				nlf->init();
				ins_leaf(nlf, key, value);
				ins_nonleaf(root, key, nlf);
				return;
			}

			std::stack<nonleaf*> path; // 存放查询路径
			nonleaf* p = root;
			leaf* lf = NULL;

			while (true) {
				path.push(p); // 当前节点入栈
				int r = search_index(p, key);
				if (!p->if_plf) p = (nonleaf*)p->pointer[r];
				else {
					lf = (leaf*)p->pointer[r];
					break;
				}
			}

			if (lf == NULL) { // 如果lf为空只可能是在root节点最左端
				leaf* nlf = (leaf*)malloc(sizeof(leaf));
				nlf->init();
				ins_leaf(nlf, key, value);
				nlf->next = (leaf*)(root->pointer[1]);
				root->pointer[0] = nlf;
				return;
			}

			if (lf->num < N) { // 最简单的情况，数据节点能放下
				ins_leaf(lf, key, value);
				return;
			}
			// 之后的条件下需要分裂

			leaf *nf = split_leaf(lf, key, value);
			int k = nf->key[0]; // 提供给上一层的key,pointer

			p = path.top();
			path.pop();

			if (p == root && root->pointer[0] == NULL) {
				/*
				 例如插入的值一直增大 1,2,3,4
				 导致 NULL<-1->1,2,3
				 所以 1,2<-3->3,4
				*/
				root->key[0] = nf->key[0];
				root->pointer[0] = root->pointer[1];
				root->pointer[1] = nf;
				return;
			}

			// 进入通用的循环 将k,c插入p中
			void* c = nf;
			do {
				if (p->num < N) { // 如果非叶节点放的下
					ins_nonleaf(p, k, c);
					break;
				}
				// 以下情况还需继续分裂

				c = split_nonleaf(p, k, c);
				k = search_left((nonleaf*)c);

				if (p == root) { // 如果要分裂的是root节点
					nonleaf* nroot = (nonleaf*)malloc(sizeof(nonleaf));
					nroot->init();
					nroot->num = 1;
					nroot->key[0] = k;
					nroot->pointer[0] = p;
					nroot->pointer[1] = c;
					nroot->if_plf = false;

					root = nroot;
				}

				if (path.empty()) break;

				p = path.top();
				path.pop();
			} while (true);
		}

		void print_tree() {
			if (root->num != 0) print_nonleaf(root, 0);
			printf("-------------------------------------\n");
		}

		void print_leaf() {
			nonleaf* p = root;
			leaf* lf = NULL;

			while (true) {
				if (!p->if_plf) p = (nonleaf*)p->pointer[0];
				else {
					lf = (leaf*)p->pointer[0];
					break;
				}
			}

			do {
				print_leaf(lf);
				lf = lf->next;
			} while (lf != NULL);
		}
	};
}