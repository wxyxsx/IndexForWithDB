#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <vector>
#include <set>
#include <string>

constexpr auto N = 3; // �ڵ��ܷ���key������
constexpr auto NULLADDR = 0; // NULL�����ݿ��ַ�еı�ʾ

namespace db {
	// TODO �̳� ʵ��LOAD��SAVE
	class Node // TODO extend virtual page
	{
	public:
		int n; // �ڵ㵱ǰkey������
		int k[N]; // key����
		long long addr[N + 1]; // ���ݿ��ַ����
		/*
		����Ƿ�Ҷ�ڵ���0~N��������ڵ�����ݿ��ַ
		�����Ҷ�ڵ���0~N-1������ݽڵ��λ�� N���next
		*/
		int flag; // 0:Ҷ�ӽڵ� 1:��Ҷָ��Ҷ�� 2:��Ҷָ���Ҷ

		Node(void) {
			n = 0;
			for (int i = 0;i < N;i++) {
				k[i] = 0;
				addr[i] = NULLADDR;
			}
			addr[N + 1] = NULLADDR;
			flag = 0;
		}
	};

	class btree
	{
	private:
		std::vector<Node*> objlst; // ������нڵ��ָ��
		std::unordered_map<long long, int> stb; // �������ݿ��ַ������objlst�е�ƫ�� 
		std::set<int> ftb; // �������objlst�п��е�ƫ�� set�������Զ����� ���ڻ��տռ�
		long long sid; // ģ�����ݿ��ַ
		long long root; // root�ڵ����ݿ��ַ ��Ϊroot�ڵ�Ҳ��䶯

		/*
		Node* getnode(long long addr);
		long long setnode(Node* nd);
		void span_insert(Node* a, Node* b, int k, long long v, int o, bool if_leaf);
		search_index(Node* nd, int k);
		long long search_left(Node* nd);
		void direct_insert(Node* nd, int k, long long v, bool if_leaf);
		long long split_insert(Node* nd, int k, long long v, bool if_leaf);
		void print_space(int level);
		void print_space(int level);
		void print_nonleaf(Node* nd, int level)
		*/

		// �������ݿ��ַ��ȡ�ڵ�ָ��
		Node* getnode(long long addr) {
			if (addr == NULLADDR) return NULL;
			return objlst[stb[addr]];
		}

		// �ṩ�½ڵ�ĵ�ַ���������ݿ��ַ 
		// TODO ͨ��Node��hold������õ�ַ����
		long long setnode(Node* nd) {
			long long addr = sid;
			sid++;

			int key = 0;
			if (ftb.size != 0) { // ����п��пռ�
				key = *(ftb.begin());
				ftb.erase(key);
				objlst[key] = nd;
			}
			else {
				objlst.push_back(nd);
				key = objlst.size;
			}
			stb[addr] = key;
			return addr;
		}

		// ��ڵ����
		void span_insert(Node* a, Node* b, int k, long long v, int o, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int i = o - a->n;
			if (i < 0) { // ԭ�ڵ�
				a->k[o] = k;
				a->addr[o + s] = v;
			}
			else { // �½ڵ�
				b->k[i] = k;
				b->addr[i + s] = v;
			}
		}

		// ����k�ڽڵ�ԭ�����е�λ��
		int search_index(Node* nd, int k) {
			int r = nd->n; // Ĭ��ָ�����Ҷ�
			for (int i = 0;i < nd->n;i++) {
				if (k < nd->k[i]) {
					r = i;
					break;
				}
			}
			return r;
		}

		// ���ҷ�Ҷ�ڵ��������Ҷ�ڵ�ĵ�һ��ֵ
		long long search_left(Node* nd) {
			Node* r = nd;
			while (r->flag != 0) // ֱ������Ҷ�ڵ�
				r = getnode(r->addr[0]);
			return r->k[0];
		}

		// �ڵ����㹻�Ŀռ������Ԫ��
		void direct_insert(Node* nd, int k, long long v, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = search_index(nd, k);
			for (int i = nd->n;i > r;i--) {
				nd->k[i] = nd->k[i - 1];
				nd->addr[i + s] = nd->addr[i + s - 1];
			}
			nd->k[r] = k;
			nd->addr[r + s] = v;
			nd->n++;
		}

		// �ڵ����ݳ���N�������½ڵ�ĵ�ַ
		long long split_insert(Node* nd, int k, long long v, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = search_index(nd, k);

			Node* nnd = new Node();
			long long addr = setnode(nnd);

			nnd->n = (int)((N + 1) / 2) - s;
			nd->n = N + 1 - nnd->n;
			nnd->flag = nd->flag; // ���ѽڵ�λ��ͬһ��

			for (int i = N;i > r;i--) // ��key�������һλ
				span_insert(nd, nnd, nd->k[i - 1], nd->addr[i + s - 1], i, if_leaf);
			span_insert(nd, nnd, k, v, r, if_leaf);
			for (int i = r - 1;i > nd->n - 1;i--) // ��keyС��k����ҪǨ�Ƶ��µĽڵ�
				span_insert(nd, nnd, nd->k[i], nd->addr[i + s], i, if_leaf);

			if (if_leaf) { // ����ڵ����
				nnd->addr[N + 1] = nd->addr[N + 1];
				nd->addr[N] = addr;
			}
			else {
				/*
				ԭ����3��������֮��Ϊ2+1����Ϊ�µĽڵ㲻����NULL����Ҫɾ��һ��key���������Ҷ���Ϊ�½ڵ�����
				��ԭ�ڵ����Ҷ˵�keyɾ����p�ŵ��½ڵ������
				*/
				nnd->addr[0] = nd->addr[nd->n];
				nd->n--;
			}

			return addr;
		}

		void print_space(int level) {
			std::string str = "";
			for (int i = 1;i < 6 * level;i++) {
				if (i % 6 == 0) str.append("|");
				else str.append(" ");
			}
			if (level != 0) str.append("+");
			std::cout << str;
		}

		// �������kv������
		void print_leaf(Node* nd) {
			std::string str = "";
			if (nd != NULL) {
				for (int i = 0;i < nd->n;i++) {
					std::cout << std::setw(3) << std::left << nd->k[i] << "[";
					std::cout << std::setw(3) << std::left << nd->addr[i] << "],";
				}
			}
			std::cout << std::endl;
		}

		// ����������ȱ���
		void print_nonleaf(Node* nd, int level) {
			for (int i = 0;i < nd->n + 1;i++) {
				if (i != 0) print_space(level);
				// ��һ��Ԫ�ز�������

				if (i != nd->n) std::cout << std::setw(3) << std::left << nd->k[i] << "--+";
				else std::cout << "   --+";

				if (nd->flag == 1) print_leaf(getnode(nd->addr[i]));
				else print_nonleaf(getnode(nd->addr[i]), level + 1);
			}
		}

	public:

		/*
		btree(void);
		void create(int* arr, int len);
		int search(int key);
		void insert(int key, long long value);
		void print_tree();
		void print_leaf();
		*/

		btree(void) {
			// ��ʹ����û��Ԫ��ҲҪ�����ݿ�
			Node* nd = new Node();
			nd->flag = 1;
			root = setnode(nd);
		}

		// ��ʱ����
		void create(int* arr, int len) {
			for (int i = 0;i < len;i++) {
				insert(arr[i], i);
				//print_tree();
			}
		}

		// TODO Ҫ�ĳ�long long
		int search(int key) {
			int res = -1;

			Node* p = getnode(root);
			if (p->n = 0) return res;
			// root�ڵ����Ϊ��

			while (p->flag != 0) { // �����ָ��Ҷ�ڵ㣬����������
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p != NULL) { // ����root�ڵ���ڵ����ΪNUL
				for (int i = 0;i < p->n;i++) {
					if (key == p->k[i]) {
						res = (int)(p->addr[i]);
						break;
					}
				}
			}
			return res;
		}

		void insert(int key, long long value) {
			Node* ndroot = getnode(root);
			Node* p = ndroot;

			if (ndroot->n == 0) { // root�ڵ����Ϊ��
				Node* nnd = new Node();
				int addr = setnode(nnd);
				nnd->flag = 0;
				direct_insert(nnd, key, value, true);
				direct_insert(ndroot, key, addr, false);
				return;
			}

			std::stack<Node*> path; // ��Ų�ѯ·��

			while (p->flag != 0) {
				path.push(p); // ��ǰ�ڵ���ջ
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p == NULL) { // ���pΪ��ֻ��������root�ڵ������
				Node* nnd = new Node();
				int addr = setnode(nnd);
				nnd->flag = 0;
				direct_insert(nnd, key, value, true);
				ndroot->addr[0] = addr;
				nnd->addr[N + 1] = ndroot->addr[1];
				return;
			}

			if (p->n < N) { // ��򵥵���������ݽڵ��ܷ���
				direct_insert(p, key, value, true);
				return;
			}
			// ֮�����������Ҫ����

			long long v = split_insert(p, key, value, true);
			int k = getnode(v)->k[0]; // Ҷ�ڵ��ṩ����һ���key,value

			p = path.top();
			path.pop();

			if (p == ndroot && ndroot->addr[0] == NULLADDR) {
				/*
				 ��������ֵһֱ���� 1,2,3,4
				 ���� NULL<-1->1,2,3
				 ���� 1,2<-3->3,4
				*/
				ndroot->k[0] = k;
				ndroot->addr[0] = ndroot->addr[1];
				ndroot->addr[1] = v;
				return;
			}

			// ����ͨ�õ�ѭ�� ��k,v����p��
			do {
				if (p->n < N) { // �����Ҷ�ڵ�ŵ���
					direct_insert(p, k, v, false);
					break;
				}
				// ������������������

				v = split_insert(p, k, v, false);
				k = search_left(getnode(v));

				if (p == ndroot) { // ���Ҫ���ѵ���root�ڵ�
					Node* nnd = new Node();
					long long addr = setnode(nnd);
					nnd->n = 1;
					nnd->k[0] = k;
					nnd->addr[0] = root;
					nnd->addr[1] = v;
					nnd->flag = 2;

					root = addr;
				}

				if (path.empty()) break;

				p = path.top();
				path.pop();
			} while (true);
		}

		void print_tree() {
			Node* ndroot = getnode(root);
			if (ndroot->n != 0) print_nonleaf(ndroot, 0);
			std::cout << "-------------------------------------" << std::endl;
		}

		void print_leaf() {
			Node* p = getnode(root);

			while (p->flag != 0) p = getnode(p->addr[0]);

			while (p->addr[N + 1] != NULLADDR) {
				print_leaf(p);
				p = getnode(p->addr[N + 1]);
			}
		}
	};
}