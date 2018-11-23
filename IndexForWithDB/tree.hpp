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
			if (i < 0) { // ԭ�ڵ�
				a->key[index] = key;
				a->value[index] = value;
			}
			else { // �½ڵ�
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

		// ����k���������е�λ��
		int search_index(nodes* node, int key) {
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
		int search_left(nonleaf* node) {
			nonleaf* r = node;
			while (!r->if_plf)
				r = (nonleaf*)r->pointer[0];
			int key = ((leaf*)r->pointer[0])->key[0];
			return key;
		}

		// �ڵ����㹻�Ŀռ������Ԫ��
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

		// �ڵ����ݳ���N�������½ڵ�ĵ�ַ
		leaf* split_leaf(leaf* ori, int key, int value) {
			int r = search_index(ori, key);

			leaf* nlf = (leaf*)malloc(sizeof(leaf));
			nlf->init();
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

		nonleaf* split_nonleaf(nonleaf* ori, int key, void* pointer) {
			int r = search_index(ori, key);

			nonleaf* nnlf = (nonleaf*)malloc(sizeof(nonleaf));
			nnlf->init();
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

		void print_space(int level) {
			for (int i = 1;i < 6 * level;i++) {
				if (i % 6 == 0) printf("|");
				else printf(" ");
			}
			if (level != 0) printf("+");
		}

		// �������kv������
		void print_leaf(leaf* node) {
			if (node != NULL) {
				for (int i = 0;i < node->num;i++)
					printf("%-3d[%-3d],", node->key[i], node->value[i]);
			}
			printf("\n");
		}

		// ����������ȱ���
		void print_nonleaf(nonleaf* node, int level) {
			for (int i = 0;i < node->num + 1;i++) {
				if (i != 0) print_space(level);
				// ��һ��Ԫ�ز�������

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

		void insert(int key, int value) {
			if (root->num == 0) { // root�ڵ����Ϊ��
				leaf* nlf = (leaf*)malloc(sizeof(leaf));
				nlf->init();
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
				leaf* nlf = (leaf*)malloc(sizeof(leaf));
				nlf->init();
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