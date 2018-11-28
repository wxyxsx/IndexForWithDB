#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>

constexpr int BUFFSIZE = 100; // 假设节点大小确定
constexpr int N = (BUFFSIZE-16)/12;		  // 节点能放下key的数量 这里为7
constexpr auto NULLADDR = 0;  // NULL在数据库地址中的表示
constexpr int MAXLEN = 20; // 字符串最长不超过20
constexpr int HALFLEN = (int)(BUFFSIZE - 16) / 2;
/*
4[flag]+4[num]+4*n[key]+8*(n+1)[addr]=BUFFSIZE
n = [(BUFFSIZE-16)/12] = 4
*/

/*
对于字符串：
初始占用 4+4+8=16
每增加一个字符串占用8+size(str) 48
这里期望装下6个字符串
*/


namespace db {
	class Node
	{
	public:
		int n;
		std::vector<std::string> k;
		std::vector<long long> a;
		long long next;
		int flag;
		// 0 unused 1 leaf 2 nonleaf point to leaf 3 nonleaf point to nonleaf

		Node(void) {
			n = 0;
			next = NULLADDR;
			flag = 0;
			a.resize(1, NULLADDR);
		}

		int size() {
			int cur = 16 + 9 * n;
			for (int i = 0;i < n;i++) {
				cur += k[i].length();
			}
			return cur;
		}


		// true 不能插入
		bool full(std::string key) {
			int cur = size();
			cur += key.length() + 9;
			return cur > BUFFSIZE;
		}

		/*
		分离出的新节点key的数目 也不对，非叶节点要删除最左边的条目
		除去一定要占用的16bytes 剩下的字符串占用的大小一定要超过一般
		*/
		int split(std::string key) {
			int r = search(key);
			int half = (int)(BUFFSIZE - 16) / 2;
			int cur = 0;
			for (int i = 0;i <r;i++) {
				cur += 9 + k[i].length();
				if(cur>half) return n - i; // n + 1 - (i + 1); 总数 - 保留的数目
			}
			cur += 9 + key.length();
			if (cur > half) return n - r;
			for (int i = r;i < n;i++) {
				cur += 9 + k[i].length();
				if (cur > half) return n-1 - i; // 还要多给一个key 
			}
			return 0;
		}

		int search(std::string key) {
			int r = 0;
			for (int i = n;i > 0;i--) {
				if (key >= k[i - 1]) {
					r = i;
					break;
				}
			}
			return r;
		}

		// 大于一半返回true
		bool half() {
			int hf = (int)(BUFFSIZE - 16) / 2;
			int cur = size() - 16;
			return cur >= hf;
		}

		int resize(bool direct) {
			int hf = (int)(BUFFSIZE - 16) / 2;
			int cur = size() - 16;
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (direct) cur -= k[n - 1 - i].length() + 9; // 从大到小
				else cur -= k[i].length() + 9;

				if (cur < hf) {
					o = i;
					break;
				}
			}
			return o;
		}

		int resize(bool direct,int len) {
			int hf = (int)(BUFFSIZE - 16) / 2; // half
			int remain = size()-16; // 节点剩余大小
			int cur = len+9; // 新节点加入的大小
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (cur >= hf || remain < hf) { // 新节点不能加入超过一半 旧节点不能减少超过一半
					o = i;
					break;
				}
				if (direct) {
					int t = k[n - 1 - i].length() + 9; // 从大到小
					cur += t;
					remain -= t;
				}
				else {
					int t = k[i].length() + 9;
					cur += t;
					remain -= t;
				}
			}
			return o+1;
		}

	};

	class btree
	{
	private:
		std::vector<Node*> objlst;				// 存放所有节点的指针
		std::unordered_map<long long, int> stb; // 根据数据库地址检索在objlst中的偏移 
		std::set<int> ftb;						// 存放所有objlst中空闲的偏移 set中数据自动排序 便于回收空间

		long long root; // root节点的数据库地址 因为root节点也会变动

		long long sid;	// 初始化为1 ==NULLADDR+1
		long long getfreeaddr() { // 模拟对空闲地址的查找
			long long r = sid;
			sid++;
			return r;
		}

		// 数据库地址 -> 节点指针 可能返回NULL
		Node* getnode(long long addr) {
			if (addr == NULLADDR) return NULL; 
			return objlst[stb[addr]];
		}

		// 节点指针 存放在objlst中 -> 分配的数据库地址
		long long setnode(Node* nd) {
			long long addr = getfreeaddr();

			int key = 0;
			if (ftb.size() != 0) {			// 如果有空闲空间 则从set中获取偏移
				key = *(ftb.begin());		
				ftb.erase(key);
				objlst[key] = nd;
			}
			else {
				key = (int)objlst.size();	// size_t->int 应该不会超
				objlst.push_back(nd);
			}
			stb[addr] = key;
			return addr;
		}

		// 跨节点插入
		void span_insert(Node* a, Node* b, std::string k, long long v, int o, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
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

		// 查找k在节点原数组中的位置 从大往小查找
		int search_index(Node* nd, std::string k) {
			int r = 0; 
			int len = nd->n;
			for (int i = len;i > 0;i--) {
				if (k >= nd->k[i - 1]) {
					r = i;
					break;
				}
			}
			return r;
		}

		// 查找非叶节点下最左端叶节点的第一个key
		std::string search_left(Node* nd) {
			Node* r = nd;
			while (r->flag != 1) // 直到到达叶节点
				r = getnode(r->a[0]);
			return r->k[0];		 // 应该不会出错 都分裂非叶节点了最左端必然有值
		}

		// 节点有足够的空间插入新元素
		void direct_insert(Node* nd, std::string k, long long v, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = search_index(nd, k);
			int len = nd->n;

			nd->k.resize(len + 1);
			if(if_leaf) nd->a.resize(len + 1);
			else nd->a.resize(len + 2);

			for (int i = nd->n;i > r;i--) {
				nd->k[i] = nd->k[i - 1];
				nd->a[i + s] = nd->a[i + s - 1];
			}
			nd->k[r] = k;
			nd->a[r + s] = v;
			nd->n++;
		}

		long long split_insert_leaf(Node* nd, std::string k, long long v) {
			Node* nnd = new Node();
			long long addr = setnode(nnd);


		}

		// 节点数据超过N，返回新节点的地址
		long long split_insert(Node* nd, std::string k, long long v, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = search_index(nd, k);

			Node* nnd = new Node();
			long long addr = setnode(nnd);

			nnd->n = nd->split(k);
			nnd->k.resize(nnd->n);
			nnd->a.resize(nnd->n + s);
			int ln = nd->n;
			nd->n = ln +1 - nnd->n;
			nnd->flag = nd->flag;	// 分裂节点位于同一层

			for (int i = ln;i > r;i--)				// 比key大的右移一位
				span_insert(nd, nnd, nd->k[i - 1], nd->a[i + s - 1], i, if_leaf);
			span_insert(nd, nnd, k, v, r, if_leaf);
			for (int i = r - 1;i > nd->n - 1;i--)	// 比key小的k可能要迁移到新的节点
				span_insert(nd, nnd, nd->k[i], nd->a[i + s], i, if_leaf);

			if (if_leaf) {			// 链表节点插入
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
				nd->k.resize(nd->n);
				nd->a.resize(nd->n + s);
			}

			return addr;
		}

		void print_space(int level) {
			std::string str = "";
			for (int i = 1;i < 18 * level;i++) {
				if (i % 18 == 0) str.append("|");
				else str.append(" ");
			}
			if (level != 0) str.append("+");
			std::cout << str;
		}

		// 
		void print_leaf(Node* nd) {
			std::string str = "";
			if (nd != NULL) {
				for (int i = 0;i < nd->n;i++) {
					std::cout << nd->k[i] << "[";
					std::cout << nd->a[i] << "],";
				}
			}
			std::cout << std::endl;
		}

		// 采用深度优先遍历
		void print_nonleaf(Node* nd, int level) {
			for (int i = 0;i < nd->n + 1;i++) {
				if (i != 0) print_space(level); // 第一个元素不用缩进

				if (i != nd->n) std::cout << std::setw(15) << std::left << nd->k[i] << "--+";
				else std::cout << "               --+";

				if (nd->flag == 2) print_leaf(getnode(nd->a[i]));
				else print_nonleaf(getnode(nd->a[i]), level + 1);
			}
		}

		// 直接删除，最简单的情况
		void direct_delete(Node* nd, std::string k, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = nd->search(k); // 假设一定能找到对应元素
			int len = nd->n;
			// r ~ len-2 <- r+1 ~ len-1 
			for (int i = r + 1;i < len;i++) {
				nd->k[i - 1] = nd->k[i];
				nd->a[i - 1 + s] = nd->a[i + s];
			}
			nd->n--;
			nd->k.resize(nd->n);
			nd->a.resize(nd->n + s);
		}

		// b传入要删除的节点 返回右边节点最小值(提供给上层修改)
		std::string resize_delete_leaf(Node* a, Node* b) { // o是b需要插入的元素数
			bool direct = a->k[0] < b->k[0] ? true : false;
			int la = a->n;
			int lb = b->n;

			int o = a->resize(direct);
			b->k.resize(lb + o);
			b->a.resize(lb + o);
			if (direct) { // a -> b 
				// 123->45 == -4 == 123->5 == 12->35
				for (int i = 0;i < lb;i++) {  // o ~ lb+o-1(lfmin-1) <- 0 ~ lb-1 ok
					b->k[lb - 1 + o - i] = b->k[lb - 1 - i];
					b->a[lb - 1 + o - i] = b->a[lb - 1 - i];
				}
				for (int i = 0;i < o;i++) { // 0 ~ o-1 <- la-1-o ~ la-1  ok
					b->k[o - 1 - i] = a->k[la - 1 - i];
					b->a[o - 1 - i] = a->a[la - 1 - i];
				}
			}
			else { //  b<-a
				// 12->345 == -2 == 1->345 == 13->45
				for (int i = 0;i < o;i++) { // a的元素添加到b上 lb~lb+o-1 <- 0~o-1 ok
					b->k[lb + i] = a->k[i];
					b->a[lb + i] = a->a[i];
				}
				for (int i = 0;i < la - o;i++) { // a内部填补空位 0~la-o-1 <- o~la-1 ok
					a->k[i] = a->k[i + o];
					a->a[i] = a->a[i + o];
				}
				a->n -= o;
				b->n += o;
			}
			a->n -= o;
			b->n += o;
			a->k.resize(la - o);
			a->a.resize(la - o);
			if (direct) return b->k[0];
			else return a->k[0];
		}

		// 由Delete来释放节点 delete对象 指针置NULL，修改ftb等
		bool merge_delete_leaf(Node* a, Node* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// b始终放要删除的 direct=true则 a<-b 否则是b<-a
			if (a->size() + b->size() - 16 > BUFFSIZE) return false;
			Node *x, *y;
			if (direct) {
				x = a;
				y = b;
			}
			else {
				x = b;
				y = a;
			}
			// x<-y
			//  12<-3 123
			int lx = x->n;
			int ly = y->n;
			x->k.resize(lx + ly);
			x->a.resize(lx + ly);
			for (int i = 0;i < ly;i++) { // y原有值后移 lx~lx+ly-1 ~ 0~ly-1 
				x->k[lx + i] = y->k[i];
				x->a[lx + i] = y->a[i];
			}
			x->n += ly;
			y->n = 0;
			x->next = y->next;
			return true;
		}

		// 关键在于正确插入新增的键值 以及向上返回的键值
		std::string resize_delete_nonleaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// b始终放要删除的 direct=true则 a->b 否则是b<-a
			int la = a->n;
			int lb = b->n;

			int hf = (int)(BUFFSIZE - 16) / 2;
			int cur = b->size();
			int o = 0;
			std::string tp;

			if (direct) { // a->b
				// 10 20 30 -> 40 50 == 10 20 30 -> 50
				// 10 20 30 -> (NULL) 35 50 == 10 -> 30 35 50 (20是直接丢弃)
				tp = search_left(getnode(b->a[0]));
				o = a->resize(true, tp.length());

				b->k.resize(lb + o);
				b->a.resize(lb + o + 1);

				for (int i = 0;i < lb;i++) { // b原有值后移  o~lb+o-1 <- 0~lb-1
					b->k[lb + o - 1 - i] = b->k[lb - 1 - i];
					b->a[lb + o - i] = b->a[lb - i];
				}
				b->k[o - 1] = tp; // 因为左边代表分支下最小的值
				b->a[o] = b->a[0];
				for (int i = 0;i < o - 1;i++) { // 移动key      0~o-2 <- la-o+1~la-1
					b->k[o - 2 - i] = a->k[la - 1 - i];
					//b->k[i] = a->k[la-o+1+i];
				}
				std::string res = a->k[la - o];
				for (int i = 0;i < o;i++) { // 移动地址 
					b->a[o - 1 - i] = a->a[la - i];
					//b->addr[i] = a->addr[la-o+1+i];
				}
				a->n -= o;
				b->n += o;

				a->k.resize(la - o);
				a->a.resize(la - o + 1);

				//return search_left(getnode(b->addr[0]));
				return res;
			}
			else { //  b<-a
				tp = search_left(getnode(a->a[0]));
				o = a->resize(false, tp.length());

				b->k.resize(lb + o);
				b->a.resize(lb + o + 1);
				// 10 20 -> 30 40 50 == 10 ->30 40 50
				// 10 25 (NULL) -> 30 40 50 == 10 25 30 ->  50 丢弃(40)  
				b->k[lb] = tp;
				for (int i = 0;i < o - 1;i++) { // 移动key lb+1~lb+o-1 <- 0~o-2 o-2
					b->k[lb + 1 + i] = a->k[i];
				}
				std::string res = a->k[o - 1];
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
				b->n += o;

				a->k.resize(la - o);
				a->a.resize(la - o + 1);

				//return search_left(getnode(a->addr[0]));
				return res;
			}
		}

		bool merge_delete_nonleaf(Node* a, Node* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// b始终放要删除的 direct=true则 a<-b 否则是b<-a
			Node *x, *y;
			if (direct) {
				x = a;
				y = b;
			}
			else {
				x = b;
				y = a;
			}
			std::string st = search_left(getnode(y->a[0]));
			if (a->size() + b->size() - 16 + st.length() + 9 > BUFFSIZE) return false;
			//x<-y
			// 1 2 -> 4 5 == 1 2 -> 5 和resize代码雷同 只是要全部移动走
			// 1 2 3 (NULL)->5 ==  1235
			int lx = x->n;
			int ly = y->n;
			x->k.resize(lx + ly + 1);
			x->a.resize(lx + ly + 2);
			x->k[lx] = st;
			for (int i = 0;i < ly;i++) { // 移动key
				x->k[lx + 1 + i] = y->k[i];
			}
			for (int i = 0;i < ly + 1;i++) { // 移动地址
				x->a[lx + 1 + i] = y->a[i];
			}
			x->n += ly + 1;
			y->n = 0;
			return true;
		}				

		void erase_node(long long addr) { // 清楚addr对应的指针
			Node* nd = getnode(addr);
			delete nd;
			int i = stb[addr];
			stb.erase(addr);
			objlst[i] = NULL;
			ftb.insert(i);
		}

	public:

		btree(void) { // 即使索引没有元素也要有数据块
			sid = 1;
			Node* nd = new Node();
			nd->flag = 2;
			root = setnode(nd);
		}

		long long search(std::string key) {
			long long res = NULLADDR;

			Node* p = getnode(root);
			if (p->n == 0) return res;  // root节点可能为空

			while (p->flag != 1) {		// 如果p不是叶节点，继续向下找
				int r = p->search(key);
				p = getnode(p->a[r]);
			}

			if (p != NULL) {			// 考虑root节点左节点可能为NUL
				for (int i = 0;i < p->n;i++) {
					if (key == p->k[i]) {
						res = (p->a[i]);
						break;
					}
				}
			}
			return res;
		}

		void insert(std::string key, long long value) {
			Node* ndroot = getnode(root);

			if (ndroot->n == 0) {	// root节点为空就需要新建叶节点
				Node* nnd = new Node();
				nnd->flag = 1;
				direct_insert(nnd, key, value, true);

				long long addr = setnode(nnd); // 目前地址从btree中获取，以后应该从node中获取它绑定的地址
				direct_insert(ndroot, key, addr, false);
				return;
			}

			Node* p = ndroot;
			std::stack<Node*> path; // 存放查询路径

			do {
				path.push(p);
				int r = search_index(p, key);
				p = getnode(p->a[r]);
			} while (p != NULL && p->flag != 1);

			if (p == NULL) {		// 如果p为空只可能是在root节点最左端 新建叶节点 地址放入根节点中
				Node* nnd = new Node();
				long long addr = setnode(nnd);
				nnd->flag = 1;
				direct_insert(nnd, key, value, true);
				ndroot->a[0] = addr;
				nnd->next = ndroot->a[1];
				return;
			}

			if (!p->full(key)) {	// 数据节点能放下
				direct_insert(p, key, value, true);
				return;
			}

			// 之后的条件下需要分裂叶节点

			long long v = split_insert(p, key, value, true);
			std::string k = getnode(v)->k[0];   // 叶节点提供给上一层的key,value

			

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
					return;
				}

				if (!p->full(k)) { // 如果非叶节点放的下
					direct_insert(p, k, v, false);
					break;
				}

				// 以下情况还需继续分裂非叶节点

				v = split_insert(p, k, v, false);
				k = search_left(getnode(v));
				

				if (p == ndroot) { // 如果要分裂的是root节点 需要新建root节点
					Node* nnd = new Node();
					long long addr = setnode(nnd);
					nnd->n = 1;
					nnd->k.resize(1);
					nnd->a.resize(2);

					nnd->k[0] = k;
					nnd->a[0] = root;
					nnd->a[1] = v;
					nnd->flag = 3;

					root = addr;
				}

			} while (!path.empty());
		}

		bool delkey(std::string key) {
			if (search(key) == NULLADDR) return false; // 如果没有找到key则返回没有找到

			Node* ndroot = getnode(root);  // 再次搜索 不过要记录过程
			Node* p = ndroot; // 当前节点

			std::stack<Node*> path;
			std::stack<int> poffset; // 还要记录走的是哪个子节点

			do {
				int r = p->search(key);
				path.push(p);		// 当前节点入栈
				poffset.push(r);    // 偏移入栈
				p = getnode(p->a[r]);
			} while (p->flag != 1);

			// 到达叶节点
			direct_delete(p, key, true);
			if (p->half()) return true; // 如果叶节点足够大则直接删除

			Node* pv = path.top(); //当前节点父节点
			int pov = poffset.top(); //getnode(pv->addr[pov])即当前节点

			if (pv == ndroot) {
				if (pv->a[0] == NULLADDR) {
					// 如果只有一个叶节点，无视节点数量的限制
					if (p->n == 0) { // 擦除叶节点
						erase_node(pv->a[1]);
						pv->n = 0;
					}
					return true;
				}
				else if (pv->n == 1) {
					int sign = 1 - pov; // pov = 1 sign = 0 || pov = 0 sign = 1 || sign < 0
					Node* other = getnode(pv->a[sign]);
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

			// 先处理叶节点 估计要有重复代码了 

			int sign = pov == 0 ? 1 : pov - 1; //记录相邻节点的位置
			int tp = pov == 0 ? 0 : pov - 1;  // 记录上一层要删除key的位置
			Node* other = getnode(pv->a[sign]);
			if (merge_delete_leaf(other, p)) {
				long long eaddr = pv->a[tp + 1];
				erase_node(eaddr);
				if (pv == ndroot || pv->size() - pv->k[tp].length() - 9 >= HALFLEN) { // 上一层是root则一定能直接删除 因为root的特殊情况已经处理
					direct_delete(pv, pv->k[tp], false);
					return true;
				}
			} else { // resize即可，不会删除节点，比较安全
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
				direct_delete(p, p->k[curk], false);
				if (merge_delete_nonleaf(other, p)) {
					long long eaddr = pv->a[tp + 1];
					erase_node(eaddr);
					if ((pv == ndroot && pv->n > 1) || pv->size() - pv->k[tp].length() - 9 >= HALFLEN) { // 上一层是root至少为1 其它至少为nlfmin
						direct_delete(pv, pv->k[tp], false);
						return true;
					}
					else if (pv == ndroot && pv->n == 1) { // 如果root节点要删除 重新设置root
						long long eaddr = root;
						root = ndroot->a[0];
						erase_node(eaddr);
						return true;
					}
				} else { // resize即可，不会删除节点，比较安全
					pv->k[tp] = resize_delete_nonleaf(other, p);
					return true;
				}
		

			} while (true);

			return true;
		}

		void print_tree() {
			Node* ndroot = getnode(root);
			if (ndroot->n != 0) print_nonleaf(ndroot, 0);
			std::cout << "-------------------------------------" << std::endl;
		}

		void print_leaf() {
			Node* p = getnode(root);

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
	};
}