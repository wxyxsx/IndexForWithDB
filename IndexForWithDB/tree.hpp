#pragma once

#include"node.hpp"
#include<stack>

namespace db {
	// 根据k在第i位，将其放入a或b节点
	void bplustree::span_ins_lf(leaf* a, leaf* b, int key, int value, int index) {
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

	void bplustree::span_ins_nlf(nonleaf* a, nonleaf* b, int key, void* pointer, int index) {
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
	int bplustree::search_index(nodes* node, int key) {
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
	int bplustree::search_left(nonleaf* node) {
		nonleaf* r = node;
		while (!r->if_plf)
			r = (nonleaf*)r->pointer[0];
		int key = ((leaf*)r->pointer[0])->key[0];
		return key;
	}

	// 节点有足够的空间插入新元素
	void bplustree::ins_leaf(leaf* ori, int key, int value) {
		int r = search_index(ori, key);

		for (int i = ori->num;i > r;i--) {
			ori->key[i] = ori->key[i - 1];
			ori->value[i] = ori->value[i - 1];
		}
		ori->key[r] = key;
		ori->value[r] = value;
		ori->num++;
	}

	void bplustree::ins_nonleaf(nonleaf* ori, int key, void* pointer) {
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
	leaf* bplustree::split_leaf(leaf* ori, int key, int value) {
		int r = search_index(ori, key);

		leaf newleaf;
		leaf* nlf = &newleaf;
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

	nonleaf* bplustree::split_nonleaf(nonleaf* ori, int key, void* pointer) {
		int r = search_index(ori, key);

		nonleaf newnonleaf;
		nonleaf* nnlf = &newnonleaf;
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

	void bplustree::print_space(int level) {
		for (int i = 1;i < 6 * level;i++) {
			if (i % 6 == 0) printf("|");
			else printf(" ");
		}
		if (level != 0) printf("+");
	}

	// 依次输出kv并换行
	void bplustree::print_leaf(leaf* node) {
		if (node != NULL) {
			for (int i = 0;i < node->num;i++)
				printf("%-3d[%-3d],", node->key[i], node->value[i]);
		}
		printf("\n");
	}

	// 采用深度优先遍历
	void bplustree::print_nonleaf(nonleaf* node, int level) {
		for (int i = 0;i < node->num + 1;i++) {
			if (i != 0) print_space(level);
			// 第一个元素不用缩进

			if (i != node->num) printf("%-3d--+", node->key[i]);
			else printf("   --+");

			if (node->if_plf) print_leaf((leaf*)node->pointer[i]);
			else print_nonleaf((nonleaf*)node->pointer[i], level + 1);
		}
	}

	bplustree::bplustree(void) {
		root = NULL;
		int a[100] = { 112, 255, 223, 103, 457, 477, 277, 212, 245, 334, 13, 134, 389,
		123, 434, 312, 343, 133, 413, 357, 257, 253, 113, 33, 157, 345, 456, 12,
		143, 34, 403, 69, 155, 43, 287, 89, 444, 412, 45, 356, 77, 135, 256, 56,
		303, 87, 369, 121, 335, 289, 156, 445, 455, 189, 313, 21, 377, 487, 233,
		177, 213, 423, 465, 421, 433, 3, 145, 387, 333, 355, 453, 57, 187, 269,
		321, 265, 435, 243, 235, 44, 469, 23, 169, 244, 234, 344, 53, 353, 55,
		144, 203, 365, 153, 323, 443, 221, 35, 65, 165, 489 };
		printf("fh");
		create(a, 100);
		print_tree();
		print_leaf();
		
	}

	void bplustree::create(int* arr, int len) {
		nonleaf newnonleaf;
		root = &newnonleaf;
		root->if_plf = true;

		for (int i = 0;i < len;i++)
			insert(arr[i], i);
	}

	int bplustree::search(int key) {
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

	void bplustree::insert(int key, int value) {
		if (root->num == 0) { // root节点可能为空
			leaf newleaf;
			leaf* nlf = &newleaf;
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
			leaf newleaf;
			leaf* nlf = &newleaf;
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
				nonleaf newnonleaf;
				nonleaf* nroot = &newnonleaf;
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

	void bplustree::print_tree() {
		if (root->num != 0) print_nonleaf(root, 0);
		printf("-------------------------------------\n");
	}

	void bplustree::print_leaf() {
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

}