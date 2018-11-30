#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>

namespace db {

	// todo 处理字符串(删除，查询，删除)要限制最大长度

	using str = std::string;

	constexpr int BUFFSIZE = 100; // 节点buffer大小
	constexpr int MAXSTRSIZE = 20; // 字符串最大长度
	constexpr long long NULLADDR = 0; // NULL在数据库地址中的表示
	constexpr int N = (BUFFSIZE - 16) / 12; // 节点能放下的整型key数
	constexpr int MINLF = (N + 1) / 2; // 叶节点最小整型key数
	constexpr int MINNLF = MINLF - 1; // 非叶节点最小整型key数
	constexpr int HALFSTR = (BUFFSIZE - 16) / 2; // 节点最小字符串占用

	/*
	4[flag]+4[num]+4*n[key]+8*(n+1)[addr]=BUFFSIZE
	*/

	template <typename T>
	class Node
	{
	public:
		int flag;
		// 0 未使用 1 叶节点 2 指向叶节点 3 指向非叶节点
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
				cur += static_cast<int>(k[i].length());
			return cur;
		}

		int size() {
			int cur = strsize();
			return cur + 16;
		}

		bool full(str key) {
			int cur = size();
			cur += static_cast<int>(key.length()) + 9;
			return cur > BUFFSIZE;
		}

		// 留下的字符串占用要超过一半
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
				cur += 9 + static_cast<int>(k[i].length());
				if (cur > HALFSTR) return n - i; // n + 1 - (i + 1); 总数 - 保留的数目
			}
			cur += 9 + static_cast<int>(key.length());
			if (cur > HALFSTR) return n - r;
			for (int i = r;i < n;i++) {
				cur += 9 + static_cast<int>(k[i].length());
				if (cur > HALFSTR) return n - 1 - i;
			}
			return 0;
		}

		bool half() {
			int cur = strsize();
			return cur >= HALFSTR;
		}

		bool half(str key) {
			int cur = strsize();
			cur -= static_cast<int>(key.length()) - 9;
			return cur >= HALFSTR;
		}

		bool merge(int len) {
			int cur = strsize();
			return cur + len > BUFFSIZE;
		}

		bool merge(int len, str key) {
			int cur = strsize();
			cur += static_cast<int>(key.length()) + 9;
			return cur + len > BUFFSIZE;
		}

		int move(bool direct) {
			int cur = strsize();
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (direct) cur -= static_cast<int>(k[n - 1 - i].length()) + 9; // 从大到小
				else cur -= static_cast<int>(k[i].length()) + 9;
				if (cur < HALFSTR) {
					o = i; // break的时候 循环了i+1次数 但此时已经不符合条件 所以用上一次i
					break;
				}
			}
			return o;
		}

		int move(bool direct, str key) {
			int remain = strsize(); // 节点剩余大小
			int cur = static_cast<int>(key.length()) + 9; // 新节点加入的大小
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (cur >= HALFSTR || remain < HALFSTR) { // 新节点不能加入超过一半 旧节点不能减少超过一半
					o = i;
					break;
				}
				int t;
				if (direct) t = static_cast<int>(k[n - 1 - i].length()) + 9;  // 从大到小
				else  t = static_cast<int>(k[i].length()) + 9;
				cur += t;
				remain -= t;
			}
			return o;
			// i放在开头是循环的次数
			// 循环1次后break，则表示只能移入一个节点
			// 循环2次后break，证明只能循环1次是符合要求的，节点损失2个key
		}
	};

	template <class T>
	class bptree
	{
	public:

		std::vector<Node<T>*> objlst; // 存放所有节点的指针
		std::unordered_map<long long, int> stb; // 根据数据库地址检索在objlst中的偏移 
		std::set<int> ftb; // 存放所有objlst中空闲的偏移 set中数据自动排序 便于回收空间

		long long root; // root节点的数据库地址 因为root节点也会变动

		long long sid;
		long long getfreeaddr() { // 模拟对空闲地址的查找
			long long r = sid;
			sid++;
			return r;
		}

		// 数据库地址 -> 节点指针 可能返回NULL
		Node<T>* getnode(long long addr) {
			if (addr == NULLADDR) return NULL;
			return objlst[stb[addr]];
		}

		// 节点指针 存放在objlst中 -> 分配的数据库地址
		long long setnode(Node<T>* nd) {
			long long addr = getfreeaddr();

			int key = 0;
			if (ftb.size() != 0) { // 如果有空闲空间 则从set中获取偏移
				key = *(ftb.begin());
				ftb.erase(key);
				objlst[key] = nd;
			}
			else {
				key = static_cast<int>(objlst.size());
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

		// 根据顺序跨节点插入
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

		// 查找k在节点原数组中的位置
		int search_index(Node<T>* nd, T k) {
			int r = nd->n; // 默认指向最右端
			for (int i = 0;i < nd->n;i++) {
				if (k < nd->k[i]) {
					r = i;
					break;
				}
			}
			return r;
		}

		// 查找非叶节点下最左端叶节点的第一个key
		T search_left(Node<T>* nd) {
			Node<T>* r = nd;
			while (r->flag != 1) // 直到到达叶节点
				r = getnode(r->a[0]);
			return r->k[0];  // 都分裂非叶节点了最左端必然有值
		}

		// 节点有足够的空间插入新元素
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

		// 节点数据超过N，返回新节点的地址
		long long split_insert(Node<T>* nd, T k, long long v) {
			int s = nd->isleaf();
			int r = search_index(nd, k);

			Node<T>* nnd = new Node<T>();
			long long addr = setnode(nnd);

			nnd->n = nd->split(k);
			nnd->resize();

			int ln = nd->n;
			nd->n = ln + 1 - nnd->n;
			nnd->flag = nd->flag; // 分裂节点位于同一层

			for (int i = ln;i > r;i--) // 比key大的右移一位
				span_insert(nd, nnd, nd->k[i - 1], nd->a[i + s - 1], i);
			span_insert(nd, nnd, k, v, r);
			for (int i = r - 1;i > nd->n - 1;i--) // 比key小的k可能要迁移到新的节点
				span_insert(nd, nnd, nd->k[i], nd->a[i + s], i);

			if (s == 0) { // 链表节点插入
				nnd->next = nd->next;
				nd->next = addr;
			}
			else {
				/*
				原来有3个键分裂之后为2+1，因为新的节点不能有NULL，需要删除一个key，把它的右端作为新节点的左端
				把原节点最右端的key删除，p放到新节点最左端
				*/
				nnd->a[0] = nd->a[nd->n];
				nd->n--;
				nd->resize();
			}

			return addr;
		}

		// 直接删除，最简单的情况
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

		// b传入要删除的节点 返回右边节点最小值(提供给上层修改)
		T resize_delete_leaf(Node<T>* a, Node<T>* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			int la = a->n;
			int lb = b->n;

			int o = a->move(direct); // b需要插入的元素数目
			b->n += o;
			b->resize();

			if (direct) { // a -> b 
				// 123->45 == -4 == 123->5 == 12->35
				for (int i = 0;i < lb;i++) { // o ~ lb+o-1(lfmin-1) <- 0 ~ lb-1
					b->k[lb - 1 + o - i] = b->k[lb - 1 - i];
					b->a[lb - 1 + o - i] = b->a[lb - 1 - i];
				}
				for (int i = 0;i < o;i++) { // 0 ~ o-1 <- la-1-o ~ la-1
					b->k[o - 1 - i] = a->k[la - 1 - i];
					b->a[o - 1 - i] = a->a[la - 1 - i];
				}
			}
			else { //  b<-a
				// 12->345 == -2 == 1->345 == 13->45
				for (int i = 0;i < o;i++) { // a的元素添加到b上 lb~lb+o-1 <- 0~o-1
					b->k[lb + i] = a->k[i];
					b->a[lb + i] = a->a[i];
				}
				for (int i = 0;i < la - o;i++) { // a内部填补空位 0~la-o-1 <- o~la-1
					a->k[i] = a->k[i + o];
					a->a[i] = a->a[i + o];
				}
			}

			a->n -= o;
			a->resize();

			if (direct) return b->k[0];
			else return a->k[0];
		}

		// 关键在于正确插入新增的键值 以及向上返回的键值
		T resize_delete_nonleaf(Node<T>* a, Node<T>* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;

			int la = a->n;
			int lb = b->n;

			int o = 0;
			T tp;

			if (direct) { // a->b
				// 10 20 30 -> 40 50 == 10 20 30 -> 50
				// 10 20 30 -> (NULL) 35 50 == 10 -> 30 35 50 (20是直接丢弃)
				tp = search_left(getnode(b->a[0]));
				o = a->move(direct, tp);

				b->n += o;
				b->resize();

				for (int i = 0;i < lb;i++) { // b原有值后移  o~lb+o-1 <- 0~lb-1
					b->k[lb + o - 1 - i] = b->k[lb - 1 - i];
					b->a[lb + o - i] = b->a[lb - i];
				}
				b->k[o - 1] = tp; // 因为左边代表分支下最小的值
				b->a[o] = b->a[0];
				for (int i = 0;i < o - 1;i++) { // 移动key 0~o-2 <- la-o+1~la-1
					b->k[o - 2 - i] = a->k[la - 1 - i];
				}
				T res = a->k[la - o];
				for (int i = 0;i < o;i++) {  // 移动地址 
					b->a[o - 1 - i] = a->a[la - i];
				}

				a->n -= o;
				a->resize();

				return res;
			}
			else { //  b<-a
				// 10 20 -> 30 40 50 == 10 ->30 40 50
				// 10 25 (NULL) -> 30 40 50 == 10 25 30 ->  50 丢弃(40)  
				tp = search_left(getnode(a->a[0]));
				o = a->move(direct, tp);

				b->n += o;
				b->resize();

				b->k[lb] = tp;
				for (int i = 0;i < o - 1;i++) { // 移动key lb+1~lb+o-1 <- 0~o-2 o-2
					b->k[lb + 1 + i] = a->k[i];
				}
				T res = a->k[o - 1];
				for (int i = 0;i < o;i++) { // 移动地址  lb+1~lb+o <- 0~o-1 0-1
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

		// 由Delete来释放节点 delete对象 指针置NULL，修改ftb等
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

			for (int i = 0;i < ly;i++) { // y原有值后移 lx~lx+ly-1 ~ 0~ly-1 
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
			if (a->merge(b->size(), st)) return false;
			// 1 2 -> 4 5 == 1 2 -> 5 和resize代码雷同 只是要全部移动走
			// 1 2 3 (NULL)->5 ==  1235
			int lx = x->n;
			int ly = y->n;
			x->n += ly + 1;
			x->resize();

			x->k[lx] = st;
			for (int i = 0;i < ly;i++) { // 移动key
				x->k[lx + 1 + i] = y->k[i];
			}
			for (int i = 0;i < ly + 1;i++) { // 移动地址
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


		void print_space(int level, int pd) {
			str st = "";
			for (int i = 1;i < (pd + 3) * level;i++) {
				if (i % (pd + 3) == 0) st.append("|");
				else st.append(" ");
			}
			if (level != 0) st.append("+");
			std::cout << st;
		}

		// 采用深度优先遍历
		void print_nonleaf(Node<T>* nd, int level, int pd) {
			for (int i = 0;i < nd->n + 1;i++) {
				if (i != 0) print_space(level, pd); // 第一个元素不用缩进

				if (i != nd->n) std::cout << std::setw(pd) << std::left << nd->k[i] << "--+";
				else {
					str st = "";
					for (int i = 0;i < pd;i++) st.append(" ");
					st.append("--+");
					std::cout << st;
				}

				if (nd->flag == 2) print_leaf(getnode(nd->a[i]));
				else print_nonleaf(getnode(nd->a[i]), level + 1, pd);
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
			if (p->n == 0) return res; // root节点可能为空

			while (p->flag != 1) { // 如果p不是叶节点，继续向下找
				int r = search_index(p, key);
				p = getnode(p->a[r]);
				if (p == NULL) break; // 考虑root节点左节点可能为NUL
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
			std::stack<Node<T>*> path; // 存放查询路径

			do {
				path.push(p);
				int r = search_index(p, key);
				p = getnode(p->a[r]);
			} while (p != NULL && p->flag != 1);

			if (p == NULL) { // 如果p为空只可能是在root节点最左端 新建叶节点 地址放入根节点中
				Node<T>* nnd = new Node<T>();
				long long addr = setnode(nnd);
				nnd->flag = 1;
				direct_insert(nnd, key, value);
				ndroot->a[0] = addr;
				nnd->next = ndroot->a[1];
				return true;
			}

			if (!p->full(key)) { // 数据节点能放下
				direct_insert(p, key, value);
				return true;
			}

			long long v = split_insert(p, key, value);
			T k = getnode(v)->k[0]; // 叶节点提供给上一层的key,value

			// 进入通用的循环 将k,v插入p中
			do {
				p = path.top();
				path.pop();

				if (p == ndroot && ndroot->a[0] == NULLADDR) {
					/*
					 例如插入的值一直增大 1,2,3,4
					 导致 NULL [1] 1,2 [3] 3,4
					 所以 1,2 [3] 3,4
					*/
					ndroot->k[0] = k;
					ndroot->a[0] = ndroot->a[1];
					ndroot->a[1] = v;
					return true;
				}

				if (!p->full(k)) { // 如果非叶节点放的下
					direct_insert(p, k, v);
					break;
				}

				// 以下情况还需继续分裂非叶节点

				v = split_insert(p, k, v);
				k = search_left(getnode(v));


				if (p == ndroot) { // 如果要分裂的是root节点 需要新建root节点
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
			if (search(key) == NULLADDR) return false; // 如果没有找到key则返回没有找到

			Node<T>* ndroot = getnode(root);
			Node<T>* p = ndroot;

			std::stack<Node<T>*> path;
			std::stack<int> poffset; // 还要记录走的是哪个子节点

			do {
				int r = search_index(p, key);
				path.push(p); // 当前节点入栈
				poffset.push(r); // 偏移入栈
				p = getnode(p->a[r]);
			} while (p->flag != 1);

			// 到达叶节点
			direct_delete(p, key);
			if (p->half()) return true;

			Node<T>* pv = path.top(); //当前节点父节点
			int pov = poffset.top(); //getnode(pv->addr[pov])即当前节点

			if (pv == ndroot) {
				if (pv->a[0] == NULLADDR) { // 如果只有一个叶节点，无视节点数量的限制
					if (p->n == 0) { // 擦除叶节点
						erase_node(pv->a[1]);
						pv->n = 0;
					}
					return true;
				}
				else if (pv->n == 1) {
					int sign = 1 - pov;
					Node<T>* other = getnode(pv->a[sign]);
					if (merge_delete_leaf(other, p)) {
						// 如果另一个节点key不够 则合并 并挂在右边 保持只有root的左节点才能为NULL
						// 始终合并到左边节点
						erase_node(pv->a[1]);
						pv->a[1] = pv->a[0];
						pv->a[0] = NULLADDR;
						pv->k[0] = getnode(pv->a[1])->k[0];
						return true;
					}
				}
			}


			int sign = pov == 0 ? 1 : pov - 1; //记录相邻节点的位置
			int tp = pov == 0 ? 0 : pov - 1; // 记录上一层要删除key的位置
			Node<T>* other = getnode(pv->a[sign]);
			if (merge_delete_leaf(other, p)) {
				long long eaddr = pv->a[tp + 1];
				erase_node(eaddr);
				if (pv == ndroot || pv->half(pv->k[tp])) {  // 上一层是root则一定能直接删除 因为root的特殊情况已经处理
					direct_delete(pv, pv->k[tp]);
					return true;
				}
			}
			else { // resize即可，不会删除节点，比较安全
				pv->k[tp] = resize_delete_leaf(other, p);
				return true;
			}

			// 否则交给循环继续删除
			do {
				int curk = tp; // curk变成这一层需要删除key的位置
				p = path.top(); // 当前节点上移
				path.pop();
				poffset.pop();

				pv = path.top();
				pov = poffset.top();

				sign = pov == 0 ? 1 : pov - 1; // 相邻节点位置
				tp = pov == 0 ? 0 : pov - 1; // 上一层可能要删除的位置
				other = getnode(pv->a[sign]);
				direct_delete(p, p->k[curk]);
				if (merge_delete_nonleaf(other, p)) {
					long long eaddr = pv->a[tp + 1];
					erase_node(eaddr);
					if ((pv == ndroot && pv->n > 1) || pv->half(pv->k[tp])) { // 上一层是root至少为1 其它至少为half
						direct_delete(pv, pv->k[tp]);
						return true;
					}
					else if (pv == ndroot && pv->n == 1) { // 如果root节点要删除 重新设置root
						long long eaddr = root;
						root = ndroot->a[0];
						erase_node(eaddr);
						return true;
					}
				}
				else { // resize即可，不会删除节点，比较安全
					pv->k[tp] = resize_delete_nonleaf(other, p);
					return true;
				}


			} while (true);

			return true;
		}
	};
}