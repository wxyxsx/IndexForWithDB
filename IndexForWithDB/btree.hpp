#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <vector>
#include <set>
#include <string>

constexpr auto N = 3; // 节点能放下key的数量
constexpr auto NULLADDR = 0; // NULL在数据库地址中的表示

namespace db {
	// TODO 继承 实现LOAD和SAVE
	class Node // TODO extend virtual page
	{
	public:
		int n; // 节点当前key的数量
		int k[N]; // key数组
		long long addr[N + 1]; // 数据库地址数组
		/*
		如果是非叶节点则0~N存放其它节点的数据库地址
		如果是叶节点则0~N-1存放数据节点的位置 N存放next
		*/
		int flag; // 0:叶子节点 1:非叶指向叶子 2:非叶指向非叶

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
		std::vector<Node*> objlst; // 存放所有节点的指针
		std::unordered_map<long long, int> stb; // 根据数据库地址检索在objlst中的偏移 
		std::set<int> ftb; // 存放所有objlst中空闲的偏移 set中数据自动排序 便于回收空间
		long long sid; // 模拟数据库地址
		long long root; // root节点数据库地址 因为root节点也会变动

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

		// 根据数据库地址获取节点指针
		Node* getnode(long long addr) {
			if (addr == NULLADDR) return NULL;
			return objlst[stb[addr]];
		}

		// 提供新节点的地址，返回数据库地址 
		// TODO 通过Node的hold方法获得地址？？
		long long setnode(Node* nd) {
			long long addr = sid;
			sid++;

			int key = 0;
			if (ftb.size != 0) { // 如果有空闲空间
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

		// 跨节点插入
		void span_insert(Node* a, Node* b, int k, long long v, int o, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int i = o - a->n;
			if (i < 0) { // 原节点
				a->k[o] = k;
				a->addr[o + s] = v;
			}
			else { // 新节点
				b->k[i] = k;
				b->addr[i + s] = v;
			}
		}

		// 查找k在节点原数组中的位置
		int search_index(Node* nd, int k) {
			int r = nd->n; // 默认指向最右端
			for (int i = 0;i < nd->n;i++) {
				if (k < nd->k[i]) {
					r = i;
					break;
				}
			}
			return r;
		}

		// 查找非叶节点下最左端叶节点的第一个值
		long long search_left(Node* nd) {
			Node* r = nd;
			while (r->flag != 0) // 直到到达叶节点
				r = getnode(r->addr[0]);
			return r->k[0];
		}

		// 节点有足够的空间插入新元素
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

		// 节点数据超过N，返回新节点的地址
		long long split_insert(Node* nd, int k, long long v, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = search_index(nd, k);

			Node* nnd = new Node();
			long long addr = setnode(nnd);

			nnd->n = (int)((N + 1) / 2) - s;
			nd->n = N + 1 - nnd->n;
			nnd->flag = nd->flag; // 分裂节点位于同一层

			for (int i = N;i > r;i--) // 比key大的右移一位
				span_insert(nd, nnd, nd->k[i - 1], nd->addr[i + s - 1], i, if_leaf);
			span_insert(nd, nnd, k, v, r, if_leaf);
			for (int i = r - 1;i > nd->n - 1;i--) // 比key小的k可能要迁移到新的节点
				span_insert(nd, nnd, nd->k[i], nd->addr[i + s], i, if_leaf);

			if (if_leaf) { // 链表节点插入
				nnd->addr[N + 1] = nd->addr[N + 1];
				nd->addr[N] = addr;
			}
			else {
				/*
				原来有3个键分裂之后为2+1，因为新的节点不能有NULL，需要删除一个key，把它的右端作为新节点的左端
				把原节点最右端的key删除，p放到新节点最左端
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

		// 依次输出kv并换行
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

		// 采用深度优先遍历
		void print_nonleaf(Node* nd, int level) {
			for (int i = 0;i < nd->n + 1;i++) {
				if (i != 0) print_space(level);
				// 第一个元素不用缩进

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
			// 即使索引没有元素也要有数据块
			Node* nd = new Node();
			nd->flag = 1;
			root = setnode(nd);
		}

		// 临时函数
		void create(int* arr, int len) {
			for (int i = 0;i < len;i++) {
				insert(arr[i], i);
				//print_tree();
			}
		}

		// TODO 要改成long long
		int search(int key) {
			int res = -1;

			Node* p = getnode(root);
			if (p->n = 0) return res;
			// root节点可能为空

			while (p->flag != 0) { // 如果不指向叶节点，继续向下找
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p != NULL) { // 考虑root节点左节点可能为NUL
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

			if (ndroot->n == 0) { // root节点可能为空
				Node* nnd = new Node();
				int addr = setnode(nnd);
				nnd->flag = 0;
				direct_insert(nnd, key, value, true);
				direct_insert(ndroot, key, addr, false);
				return;
			}

			std::stack<Node*> path; // 存放查询路径

			while (p->flag != 0) {
				path.push(p); // 当前节点入栈
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p == NULL) { // 如果p为空只可能是在root节点最左端
				Node* nnd = new Node();
				int addr = setnode(nnd);
				nnd->flag = 0;
				direct_insert(nnd, key, value, true);
				ndroot->addr[0] = addr;
				nnd->addr[N + 1] = ndroot->addr[1];
				return;
			}

			if (p->n < N) { // 最简单的情况，数据节点能放下
				direct_insert(p, key, value, true);
				return;
			}
			// 之后的条件下需要分裂

			long long v = split_insert(p, key, value, true);
			int k = getnode(v)->k[0]; // 叶节点提供给上一层的key,value

			p = path.top();
			path.pop();

			if (p == ndroot && ndroot->addr[0] == NULLADDR) {
				/*
				 例如插入的值一直增大 1,2,3,4
				 导致 NULL<-1->1,2,3
				 所以 1,2<-3->3,4
				*/
				ndroot->k[0] = k;
				ndroot->addr[0] = ndroot->addr[1];
				ndroot->addr[1] = v;
				return;
			}

			// 进入通用的循环 将k,v插入p中
			do {
				if (p->n < N) { // 如果非叶节点放的下
					direct_insert(p, k, v, false);
					break;
				}
				// 以下情况还需继续分裂

				v = split_insert(p, k, v, false);
				k = search_left(getnode(v));

				if (p == ndroot) { // 如果要分裂的是root节点
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