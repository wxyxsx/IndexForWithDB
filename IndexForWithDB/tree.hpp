#pragma once

#include"node.hpp"
#include<stack>

namespace db {
	// ����k�ڵ�iλ���������a��b�ڵ�
	void bplustree::span_ins_lf(leaf* a, leaf* b, int key, int value, int index) {
		int i = index - a->num;
		if (i < 0) { // ԭ�ڵ�
			a->key[index] = key;
			a->value[index] = value;
		}
		else { // �½ڵ�
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

	// ����k���������е�λ��
	int bplustree::search_index(nodes* node, int key) {
		int r = node->num; // Ĭ��ָ�����Ҷ�
		for (int i = 0;i < node->num;i++) {
			if (key < node->key[i]) {
				r = i;
				break;
			}
		}
		return r;
	}

	// ���ҽڵ�n�����Ҷ�ڵ�ĵ�һ��ֵ
	int bplustree::search_left(nonleaf* node) {
		nonleaf* r = node;
		while (!r->if_plf)
			r = (nonleaf*)r->pointer[0];
		int key = ((leaf*)r->pointer[0])->key[0];
		return key;
	}

	// �ڵ����㹻�Ŀռ������Ԫ��
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

	// �ڵ����ݳ���N�������½ڵ�ĵ�ַ
	leaf* bplustree::split_leaf(leaf* ori, int key, int value) {
		int r = search_index(ori, key);

		leaf newleaf;
		leaf* nlf = &newleaf;
		nlf->num = (int)((N + 1) / 2); // �µ�Ҷ�ڵ�������[(n+1)/2]��key
		ori->num = N + 1 - nlf->num; // N+1��key������
		nlf->next = ori->next;
		ori->next = nlf;

		for (int i = N;i > r;i--) // ��key�������һλ
			span_ins_lf(ori, nlf, ori->key[i - 1], ori->value[i - 1], i);
		span_ins_lf(ori, nlf, key, value, r);
		for (int i = r - 1;i > ori->num - 1;i--) // ��keyС��k����ҪǨ�Ƶ��µĽڵ�
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
		ԭ����3��������֮��Ϊ2+1����Ϊ�µĽڵ㲻����NULL����Ҫɾ��һ��key���������Ҷ���Ϊ�½ڵ�����
		��Ϊ��span_ins_nlf��׼ȷ�ԣ����ƶ���ɾ��
		*/
		nnlf->if_plf = ori->if_plf;

		for (int i = N;i > r;i--)
			span_ins_nlf(ori, nnlf, ori->key[i - 1], ori->pointer[i], i);
		span_ins_nlf(ori, nnlf, key, pointer, r);
		for (int i = r - 1;i > ori->num - 1;i--)
			span_ins_nlf(ori, nnlf, ori->key[i], ori->pointer[i + 1], i);

		// ��ԭ�ڵ����Ҷ˵�keyɾ����p�ŵ��½ڵ������
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

	// �������kv������
	void bplustree::print_leaf(leaf* node) {
		if (node != NULL) {
			for (int i = 0;i < node->num;i++)
				printf("%-3d[%-3d],", node->key[i], node->value[i]);
		}
		printf("\n");
	}

	// ����������ȱ���
	void bplustree::print_nonleaf(nonleaf* node, int level) {
		for (int i = 0;i < node->num + 1;i++) {
			if (i != 0) print_space(level);
			// ��һ��Ԫ�ز�������

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
		// root�ڵ����Ϊ��

		nonleaf* p = root;
		leaf* lf = NULL;

		while (true) {
			int t = search_index(p, key);
			// �����ָ��Ҷ�ڵ㣬����������
			if (!p->if_plf) p = (nonleaf*)p->pointer[t];
			else {
				lf = (leaf*)p->pointer[t];
				break;
			}
		}

		if (lf != NULL) { // ����root�ڵ���ڵ����ΪNUL
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
		if (root->num == 0) { // root�ڵ����Ϊ��
			leaf newleaf;
			leaf* nlf = &newleaf;
			ins_leaf(nlf, key, value);
			ins_nonleaf(root, key, nlf);
			return;
		}

		std::stack<nonleaf*> path; // ��Ų�ѯ·��
		nonleaf* p = root;
		leaf* lf = NULL;

		while (true) {
			path.push(p); // ��ǰ�ڵ���ջ
			int r = search_index(p, key);
			if (!p->if_plf) p = (nonleaf*)p->pointer[r];
			else {
				lf = (leaf*)p->pointer[r];
				break;
			}
		}

		if (lf == NULL) { // ���lfΪ��ֻ��������root�ڵ������
			leaf newleaf;
			leaf* nlf = &newleaf;
			ins_leaf(nlf, key, value);
			nlf->next = (leaf*)(root->pointer[1]);
			root->pointer[0] = nlf;
			return;
		}

		if (lf->num < N) { // ��򵥵���������ݽڵ��ܷ���
			ins_leaf(lf, key, value);
			return;
		}
		// ֮�����������Ҫ����

		leaf *nf = split_leaf(lf, key, value);
		int k = nf->key[0]; // �ṩ����һ���key,pointer

		p = path.top();
		path.pop();

		if (p == root && root->pointer[0] == NULL) {
			/*
			 ��������ֵһֱ���� 1,2,3,4
			 ���� NULL<-1->1,2,3
			 ���� 1,2<-3->3,4
			*/
			root->key[0] = nf->key[0];
			root->pointer[0] = root->pointer[1];
			root->pointer[1] = nf;
			return;
		}

		// ����ͨ�õ�ѭ�� ��k,c����p��
		void* c = nf;
		do {
			if (p->num < N) { // �����Ҷ�ڵ�ŵ���
				ins_nonleaf(p, k, c);
				break;
			}
			// ������������������

			c = split_nonleaf(p, k, c);
			k = search_left((nonleaf*)c);

			if (p == root) { // ���Ҫ���ѵ���root�ڵ�
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