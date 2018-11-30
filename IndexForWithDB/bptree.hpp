#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>

namespace db {

	using str = std::string;

	constexpr int BUFFSIZE = 100;
	constexpr int MAXSTRSIZE = 20;
	constexpr long long NULLADDR = 0;
	constexpr int N = (BUFFSIZE - 16) / 12;
	constexpr int MINLF = (N + 1) / 2;
	constexpr int MINNLF = MINLF - 1;
	constexpr int HALFSTR = (BUFFSIZE - 16) / 2;

	template <typename T>
	class Node
	{
	public:
		int flag;
		int n;
		std::vector<T> k;
		std::vector<long long> a;
		long long next;

		Node(void) {
			flag = 0;
			n = 0;
			a.resize(1, NULLADDR);
			next = NULLADDR;
		}

		int isleaf() {
			int s = flag == 1 ? 0 : 1;
			return s;
		}

		void resize() {
			int s = isleaf();
			k.resize(n);
			a.resize(n + s);
		}

		int size() {
			return n;
		}

		bool full(T key) {
			return n + 1 > N;
		}

		int split(T key) {
			int s = isleaf();
			return MINLF - s;
		}

		bool half() {
			int s = isleaf();
			return n >= MINLF - s;
		}

		bool half(T key) {
			int s = isleaf();
			return n - 1 >= MINLF - s;
		}

		bool merge(int len) {
			return n + len > N;
		}

		bool merge(int len, T key) {
			return n + len + 1 > N;
		}

		int move(bool direct) {
			int s = isleaf();
			return n - MINLF + s;
		}

		int move(bool direct, T key) {
			int s = isleaf();
			return n - MINLF + s;
		}
	};

	//template <typename T, std::enable_if<>>
	template<>
	class Node<str> {
	public:
		int flag;
		int n;
		std::vector<str> k;
		std::vector<long long> a;
		long long next;

		Node(void) {
			flag = 0;
			n = 0;
			a.resize(1, NULLADDR);
			next = NULLADDR;
		}

		// leaf-0 nonleaf-1
		int isleaf() {
			int s = flag == 1 ? 0 : 1;
			return s;
		}

		void resize() {
			int s = isleaf();
			k.resize(n);
			a.resize(n + s);
		}

		int strsize() {
			int cur = 9 * n;
			for (int i = 0;i < n;i++)
				cur += k[i].length();
			return cur;
		}

		int size() {
			int cur = strsize();
			return cur + 16;
		}

		bool full(str key) {
			int cur = size();
			cur += key.length() + 9;
			return cur > BUFFSIZE;
		}

		int split(str key) {
			int r = n;
			for (int i = 0;i < n;i++) {
				if (key <= k[i]) {
					r = i;
					break;
				}
			}
			int cur = 0;
			for (int i = 0;i < r;i++) {
				cur += 9 + k[i].length();
				if (cur > HALFSTR) return n - i;
			}
			cur += 9 + key.length();
			if (cur > HALFSTR) return n - r;
			for (int i = r;i < n;i++) {
				cur += 9 + k[i].length();
				if (cur > HALFSTR) return n - 1 - i;
			}
			return 0;
		}

		bool half(){
			int cur = strsize();
			return cur >= HALFSTR;
		}

		bool half(str key) {
			int cur = strsize();
			cur -= key.length() - 9;
			return cur >= HALFSTR;
		}

		bool merge(int len) {
			int cur = strsize();
			return cur + len > BUFFSIZE;
		}

		bool merge(int len, str key) {
			int cur = strsize();
			cur += key.length() + 9;
			return cur + len > BUFFSIZE;
		}

		int move(bool direct) {
			int cur = strsize();
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (direct) cur -= k[n - 1 - i].length() + 9;
				else cur -= k[i].length() + 9;
				if (cur < HALFSTR) {
					o = i;
					break;
				}
			}
			return o;
		}

		int move(bool direct, str key) {
			int remain = strsize();
			int cur = key.length() + 9;
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (cur >= HALFSTR || remain < HALFSTR) {
					o = i;
					break;
				}
				int t;
				if (direct) t = k[n - 1 - i].length() + 9;
				else  t = k[i].length() + 9;
				cur += t;
				remain -= t;
			}
			return o;
		}
	};

	template <class T>
	class bptree
	{
	public:

		std::vector<Node<T>*> objlst;
		std::unordered_map<long long, int> stb;
		std::set<int> ftb;

		long long root;

		long long sid;
		long long getfreeaddr() {
			long long r = sid;
			sid++;
			return r;
		}

		Node<T>* getnode(long long addr) {
			if (addr == NULLADDR) return NULL;
			return objlst[stb[addr]];
		}

		long long setnode(Node<T>* nd) {
			long long addr = getfreeaddr();

			int key = 0;
			if (ftb.size() != 0) {
				key = *(ftb.begin());
				ftb.erase(key);
				objlst[key] = nd;
			}
			else {
				key = (int)objlst.size();
				objlst.push_back(nd);
			}
			stb[addr] = key;
			return addr;
		}

		void erase_node(long long addr) {
			Node<T>* nd = getnode(addr);
			delete nd;
			int i = stb[addr];
			stb.erase(addr);
			objlst[i] = NULL;
			ftb.insert(i);
		}

		void span_insert(Node<T>* a, Node<T>* b, T k, long long v, int o) {
			int s = a->isleaf();
			int i = o - a->n;
			if (i < 0) { // 原节点
				a->k[o] = k;
				a->a[o + s] = v;
			}
			else {		 // 新节点
				b->k[i] = k;
				b->a[i + s] = v;
			}
		}

		int search_index(Node<T>* nd, T k) {
			int r = nd->n;
			for (int i = 0;i < nd->n;i++) {
				if (k < nd->k[i]) {
					r = i;
					break;
				}
			}
			return r;
		}

		T search_left(Node<T>* nd) {
			Node<T>* r = nd;
			while (r->flag != 1)
				r = getnode(r->a[0]);
			return r->k[0];
		}

		void direct_insert(Node<T>* nd, T k, long long v) {
			int s = nd->isleaf();
			int r = search_index(nd, k);
			int len = nd->n;

			nd->n++;
			nd->resize();

			for (int i = len;i > r;i--) {
				nd->k[i] = nd->k[i - 1];
				nd->a[i + s] = nd->a[i + s - 1];
			}
			nd->k[r] = k;
			nd->a[r + s] = v;
		}

		long long split_insert(Node<T>* nd, T k, long long v) {
			int s = nd->isleaf();
			int r = search_index(nd, k);

			Node<T>* nnd = new Node<T>();
			long long addr = setnode(nnd);

			nnd->n = nd->split(k);
			nnd->resize();

			int ln = nd->n;
			nd->n = ln + 1 - nnd->n;
			nnd->flag = nd->flag;

			for (int i = ln;i > r;i--)
				span_insert(nd, nnd, nd->k[i - 1], nd->a[i + s - 1], i);
			span_insert(nd, nnd, k, v, r);
			for (int i = r - 1;i > nd->n - 1;i--)
				span_insert(nd, nnd, nd->k[i], nd->a[i + s], i);

			if (s==0) {
				nnd->next = nd->next;
				nd->next = addr;
			}
			else {
				nnd->a[0] = nd->a[nd->n];
				nd->n--;
				nd->resize();
			}

			return addr;
		}

		void direct_delete(Node<T>* nd, T k) {
			int s = nd->isleaf();
			int r = 0;
			for (int i = 0;i < nd->n;i++) {
				if (k == nd->k[i]) {
					r = i;
					break;
				}
			}
			int len = nd->n;
			// r ~ len-2 <- r+1 ~ len-1 
			for (int i = r + 1;i < len;i++) {
				nd->k[i - 1] = nd->k[i];
				nd->a[i - 1 + s] = nd->a[i + s];
			}
			nd->n--;
			nd->resize();
		}

		T resize_delete_leaf(Node<T>* a, Node<T>* b) { 
			bool direct = a->k[0] < b->k[0] ? true : false;
			int la = a->n;
			int lb = b->n;

			int o = a->move(direct);
			b->n += o;
			b->resize();

			if (direct) { 
				for (int i = 0;i < lb;i++) { 
					b->k[lb - 1 + o - i] = b->k[lb - 1 - i];
					b->a[lb - 1 + o - i] = b->a[lb - 1 - i];
				}
				for (int i = 0;i < o;i++) { 
					b->k[o - 1 - i] = a->k[la - 1 - i];
					b->a[o - 1 - i] = a->a[la - 1 - i];
				}
			}
			else {
				for (int i = 0;i < o;i++) { 
					b->k[lb + i] = a->k[i];
					b->a[lb + i] = a->a[i];
				}
				for (int i = 0;i < la - o;i++) { 
					a->k[i] = a->k[i + o];
					a->a[i] = a->a[i + o];
				}
			}

			a->n -= o;
			a->resize();

			if (direct) return b->k[0];
			else return a->k[0];
		}

		T resize_delete_nonleaf(Node<T>* a, Node<T>* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			
			int la = a->n;
			int lb = b->n;

			int o = 0;
			T tp;

			if (direct) { 
				tp = search_left(getnode(b->a[0]));
				o = a->move(direct, tp);
				
				b->n += o;
				b->resize();

				for (int i = 0;i < lb;i++) { 
					b->k[lb + o - 1 - i] = b->k[lb - 1 - i];
					b->a[lb + o - i] = b->a[lb - i];
				}
				b->k[o - 1] = tp; 
				b->a[o] = b->a[0];
				for (int i = 0;i < o - 1;i++) { 
					b->k[o - 2 - i] = a->k[la - 1 - i];
				}
				T res = a->k[la - o];
				for (int i = 0;i < o;i++) { 
					b->a[o - 1 - i] = a->a[la - i];
				}

				a->n -= o;
				a->resize();

				return res;
			}
			else {
				tp = search_left(getnode(a->a[0]));
				o = a->move(direct, tp);

				b->n += o;
				b->resize();

				b->k[lb] = tp;
				for (int i = 0;i < o - 1;i++) { 
					b->k[lb + 1 + i] = a->k[i];
				}
				T res = a->k[o - 1];
				for (int i = 0;i < o;i++) { 
					b->a[lb + 1 + i] = a->a[i];
				}

				for (int i = 0;i < la - o;i++) { //  0~la-o-1 <-  o~la-1
					a->k[i] = a->k[o + i];
				}
				for (int i = 0;i < la - o + 1;i++) { // 0~la-o < o~la
					a->a[i] = a->a[o + i];
				}

				a->n -= o;
				a->resize();

				return res;
			}
		}

		bool merge_delete_leaf(Node<T>* a, Node<T>* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			if (a->merge(b->size())) return false;
			Node<T> *x, *y;
			if (direct) {
				x = a;
				y = b;
			}
			else {
				x = b;
				y = a;
			}

			int lx = x->n;
			int ly = y->n;
			x->n += ly;
			x->resize();

			for (int i = 0;i < ly;i++) { 
				x->k[lx + i] = y->k[i];
				x->a[lx + i] = y->a[i];
			}
			
			y->n = 0;
			x->next = y->next;
			return true;
		}

		bool merge_delete_nonleaf(Node<T>* a, Node<T>* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;

			Node<T> *x, *y;
			if (direct) {
				x = a;
				y = b;
			}
			else {
				x = b;
				y = a;
			}
			T st = search_left(getnode(y->a[0]));
			if (a->merge(b->size(),st)) return false;

			int lx = x->n;
			int ly = y->n;
			x->n += ly + 1;
			x->resize();

			x->k[lx] = st;
			for (int i = 0;i < ly;i++) { 
				x->k[lx + 1 + i] = y->k[i];
			}
			for (int i = 0;i < ly + 1;i++) { 
				x->a[lx + 1 + i] = y->a[i];
			}
			
			y->n = 0;
			return true;
		}

		void print_leaf() {
			Node<T>* p = getnode(root);

			if (p->a[0] == NULLADDR) {
				print_leaf(getnode(p->a[1]));
				return;
			}

			while (p->flag != 1) p = getnode(p->a[0]);

			while (p != NULL) {
				print_leaf(p);
				p = getnode(p->next);
			}
		}

		void print_leaf(Node<T>* nd) {
			if (nd != NULL) {
				for (int i = 0;i < nd->n;i++) {
					std::cout << nd->k[i] << "[";
					std::cout << nd->a[i] << "],";
				}
			}
			std::cout << std::endl;
		}

		void print_space(int level,int pd) {
			str st = "";
			for (int i = 1;i < (pd+3) * level;i++) {
				if (i % (pd+3) == 0) st.append("|");
				else st.append(" ");
			}
			if (level != 0) st.append("+");
			std::cout << st;
		}

		void print_nonleaf(Node<T>* nd, int level,int pd) {
			for (int i = 0;i < nd->n + 1;i++) {
				if (i != 0) print_space(level,pd); // 第一个元素不用缩进

				if (i != nd->n) std::cout << std::setw(pd) << std::left << nd->k[i] << "--+";
				else {
					str st = "";
					for (int i = 0;i < pd;i++) st.append(" ");
					st.append("--+");
					std::cout << st;
				}

				if (nd->flag == 2) print_leaf(getnode(nd->a[i]));
				else print_nonleaf(getnode(nd->a[i]), level + 1,pd);
			}
		}

		void print_tree(int padding) {
			Node<T>* ndroot = getnode(root);
			if (ndroot->n != 0) print_nonleaf(ndroot, 0, padding);
			std::cout << "-------------------------------------" << std::endl;
		}

		bptree(void) { // 即使索引没有元素也要有数据块
			sid = 1;
			Node<T>* nd = new Node<T>();
			nd->flag = 2;
			root = setnode(nd);
		}

		long long search(T key) {
			long long res = NULLADDR;

			Node<T>* p = getnode(root);
			if (p->n == 0) return res;

			while (p->flag != 1) {
				int r = search_index(p, key);
				p = getnode(p->a[r]);
				if (p == NULL) break;
			}

			if (p != NULL) {
				for (int i = 0;i < p->n;i++) {
					if (key == p->k[i]) {
						res = (p->a[i]);
						break;
					}
				}
			}
			return res;
		}

		bool insert(T key, long long value) {
			if (search(key) != NULLADDR) return false;

			Node<T>* ndroot = getnode(root);

			if (ndroot->n == 0) {	// root节点为空就需要新建叶节点
				Node<T>* nnd = new Node<T>();
				nnd->flag = 1;
				direct_insert(nnd, key, value);

				long long addr = setnode(nnd); // 目前地址从btree中获取，以后应该从node中获取它绑定的地址
				direct_insert(ndroot, key, addr);
				return true;
			}

			Node<T>* p = ndroot;
			std::stack<Node<T>*> path;

			do {
				path.push(p);
				int r = search_index(p, key);
				p = getnode(p->a[r]);
			} while (p != NULL && p->flag != 1);

			if (p == NULL) {
				Node<T>* nnd = new Node<T>();
				long long addr = setnode(nnd);
				nnd->flag = 1;
				direct_insert(nnd, key, value);
				ndroot->a[0] = addr;
				nnd->next = ndroot->a[1];
				return true;
			}

			if (!p->full(key)) {
				direct_insert(p, key, value);
				return true;
			}

			long long v = split_insert(p, key, value);
			T k = getnode(v)->k[0];

			do {
				p = path.top();
				path.pop();

				if (p == ndroot && ndroot->a[0] == NULLADDR) {
					ndroot->k[0] = k;
					ndroot->a[0] = ndroot->a[1];
					ndroot->a[1] = v;
					return true;
				}

				if (!p->full(k)) {
					direct_insert(p, k, v);
					break;
				}

				v = split_insert(p, k, v);
				k = search_left(getnode(v));


				if (p == ndroot) {
					Node<T>* nnd = new Node<T>();
					long long addr = setnode(nnd);
					nnd->n = 1;
					nnd->resize();

					nnd->k[0] = k;
					nnd->a[0] = root;
					nnd->a[1] = v;
					nnd->flag = 3;

					root = addr;
				}

			} while (!path.empty());
			return true;
		}

		bool delkey(T key) {
			if (search(key) == NULLADDR) return false; 

			Node<T>* ndroot = getnode(root);  
			Node<T>* p = ndroot; 

			std::stack<Node<T>*> path;
			std::stack<int> poffset; 

			do {
				int r = search_index(p, key);
				path.push(p);		
				poffset.push(r);   
				p = getnode(p->a[r]);
			} while (p->flag != 1);

			direct_delete(p, key);
			if (p->half()) return true; 

			Node<T>* pv = path.top(); 
			int pov = poffset.top(); 

			if (pv == ndroot) {
				if (pv->a[0] == NULLADDR) {
					if (p->n == 0) { 
						erase_node(pv->a[1]);
						pv->n = 0;
					}
					return true;
				}
				else if (pv->n == 1) {
					int sign = 1 - pov; 
					Node<T>* other = getnode(pv->a[sign]);
					if (merge_delete_leaf(other, p)) {
						erase_node(pv->a[1]);
						pv->a[1] = pv->a[0];
						pv->a[0] = NULLADDR;
						pv->k[0] = getnode(pv->a[1])->k[0];
						return true;
					}
				}
			}


			int sign = pov == 0 ? 1 : pov - 1;
			int tp = pov == 0 ? 0 : pov - 1;  
			Node<T>* other = getnode(pv->a[sign]);
			if (merge_delete_leaf(other, p)) {
				long long eaddr = pv->a[tp + 1];
				erase_node(eaddr);
				if (pv == ndroot || pv->half(pv->k[tp])) { 
					direct_delete(pv, pv->k[tp]);
					return true;
				}
			}
			else { 
				pv->k[tp] = resize_delete_leaf(other, p);
				return true;
			}

			do {
				int curk = tp; 
				p = path.top(); 
				path.pop();
				poffset.pop();

				pv = path.top();
				pov = poffset.top();

				sign = pov == 0 ? 1 : pov - 1;
				tp = pov == 0 ? 0 : pov - 1; 
				other = getnode(pv->a[sign]);
				direct_delete(p, p->k[curk]);
				if (merge_delete_nonleaf(other, p)) {
					long long eaddr = pv->a[tp + 1];
					erase_node(eaddr);
					if ((pv == ndroot && pv->n > 1) || pv->half(pv->k[tp])) { 
						direct_delete(pv, pv->k[tp]);
						return true;
					}
					else if (pv == ndroot && pv->n == 1) { 
						long long eaddr = root;
						root = ndroot->a[0];
						erase_node(eaddr);
						return true;
					}
				}
				else { 
					pv->k[tp] = resize_delete_nonleaf(other, p);
					return true;
				}


			} while (true);

			return true;
		}
	};
}