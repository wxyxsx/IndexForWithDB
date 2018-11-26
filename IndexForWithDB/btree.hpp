#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

constexpr auto N = 5;		  // 节点能放下key的数量
constexpr auto NULLADDR = 0;  // NULL在数据库地址中的表示

namespace db {
	/*
	TODO 继承 virtual page
	实现load dump close node_page
	参考 tuple.hpp
	*/
	class Node
	{
	public:
		int n;					// 节点当前key的数量
		int k[N];				// key数组
		long long addr[N + 1];  // 数据库地址数组
		/*
		如果是非叶节点则0~N存放其它节点的数据库地址
		如果是叶节点则0~N-1存放数据节点的位置 N存放next
		*/
		int flag;				// 0:叶子节点 1:非叶指向叶子 2:非叶指向非叶

		Node(void) {
			n = 0;
			for (int i = 0;i < N;i++) {
				k[i] = 0;
				addr[i] = NULLADDR;
			}
			addr[N] = NULLADDR;
			flag = 0;
		}

		// Node(virtual_page &&p) {}
	};

	class btree
	{
	private:
		std::vector<Node*> objlst;				// 存放所有节点的指针
		std::unordered_map<long long, int> stb; // 根据数据库地址检索在objlst中的偏移 
		std::unordered_set<int> ftb;			// 存放所有objlst中空闲的偏移 set中数据自动排序 便于回收空间

		/*
		keeper hold() loosen()
		hold(addr)获得virtualpage 然后对virtualpage尝试load，判断是否位空
		用testnode尝试load 在判断标志位 然后loosen？？
		反正就是能得到数据库地址 应该有空闲块管理结构 bitmap也好 逐个载入也过于慢了
		还需要实现load和exit
		load就是根据root节点的数据库地址 建立node，在依次拉出新的node 放入全局管理的数据结构中 和目前getset方法不一样
		exit就是遍历所有节点，发送exit()指令，把数据写入
		*/

		long long sid;	// 模拟数据库地址
		long long root; // root节点数据库地址 因为root节点也会变动

		int lfmin, nlfmin;
		/*
		Node* getnode(long long addr);
		long long setnode(Node* nd);
		void span_insert(Node* a, Node* b, int k, long long v, int o, bool if_leaf);
		int search_index(Node* nd, int k);
		long long search_left(Node* nd);
		void direct_insert(Node* nd, int k, long long v, bool if_leaf);
		long long split_insert(Node* nd, int k, long long v, bool if_leaf);
		void print_space(int level);
		void print_space(int level);
		void print_nonleaf(Node* nd, int level)
		*/

		// 根据数据库地址获取节点指针 地址为空则返回NULL
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
			if (ftb.size() != 0) {			// 如果有空闲空间 则从set中获取偏移
				key = *(ftb.begin());		// ***可能有问题因为没尝试过***
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
		void span_insert(Node* a, Node* b, int k, long long v, int o, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int i = o - a->n;
			if (i < 0) { // 原节点
				a->k[o] = k;
				a->addr[o + s] = v;
			}
			else {		 // 新节点
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

		// 查找非叶节点下最左端叶节点的第一个key
		int search_left(Node* nd) {
			Node* r = nd;
			while (r->flag != 0) // 直到到达叶节点
				r = getnode(r->addr[0]);
			return r->k[0];		 // 应该不会出错 都分裂非叶节点了最左端必然有值
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

			nnd->n = lfmin - s;
			nd->n = N + 1 - nnd->n;
			nnd->flag = nd->flag;	// 分裂节点位于同一层

			for (int i = N;i > r;i--)				// 比key大的右移一位
				span_insert(nd, nnd, nd->k[i - 1], nd->addr[i + s - 1], i, if_leaf);
			span_insert(nd, nnd, k, v, r, if_leaf);
			for (int i = r - 1;i > nd->n - 1;i--)	// 比key小的k可能要迁移到新的节点
				span_insert(nd, nnd, nd->k[i], nd->addr[i + s], i, if_leaf);

			if (if_leaf) {			// 链表节点插入
				nnd->addr[N] = nd->addr[N];
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
				if (i != 0) print_space(level); // 第一个元素不用缩进

				if (i != nd->n) std::cout << std::setw(3) << std::left << nd->k[i] << "--+";
				else std::cout << "   --+";

				if (nd->flag == 1) print_leaf(getnode(nd->addr[i]));
				else print_nonleaf(getnode(nd->addr[i]), level + 1);
			}
		}

		// 直接删除，最简单的情况
		void direct_delete(Node* nd, int k, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = 0;
			for (int i = 0;i < nd->n;i++) {
				if (k == nd->k[i]) {
					r = i;
					break;
				}
			} // 假设一定能找到对应元素
			int len = nd->n;
			// r ~ len-2 <- r+1 ~ len-1 
			for (int i = r + 1;i < len;i++) {
				nd->k[i - 1] = nd->k[i];
				nd->addr[i - 1 + s] = nd->addr[i + s];
			}
			nd->n--;
		}

		// b传入要删除的节点 返回右边节点最小值(提供给上层修改)
		int resize_delete_leaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			direct_delete(b, k, true);
			int la = a->n;
			int lb = b->n;
			int o = lfmin - lb;// 计算b需要插入的元素数
			if (direct) { // a -> b 
				// 123->45 == -4 == 123->5 == 12->35
				for (int i = 0;i < lb;i++) {  // o ~ lb+o-1(lfmin-1) <- 0 ~ lb-1 ok
					b->k[lb - 1 + o - i] = b->k[lb - 1 - i];
					b->addr[lb - 1 + o - i] = b->addr[lb - 1 - i];
				}
				for (int i = 0;i < o;i++) { // 0 ~ o-1 <- la-1-o ~ la-1  ok
					b->k[o - 1 - i] = a->k[la - 1 - i];
					b->addr[o - 1 - i] = a->addr[la - 1 - i];
				}
				a->n -= o;
				b->n += o;
				return b->k[0];
			}
			else { //  b<-a
				// 12->345 == -2 == 1->345 == 13->45
				for (int i = 0;i < o;i++) { // a的元素添加到b上 lb~lb+o-1 <- 0~o-1 ok
					b->k[lb + i] = a->k[i];
					b->addr[lb + i] = a->addr[i];
				}
				for (int i = 0;i < la - o;i++) { // a内部填补空位 0~la-o-1 <- o~la-1 ok
					a->k[i] = a->k[i + o];
					a->addr[i] = a->addr[i + o];
				}
				a->n -= o;
				b->n += o;
				return a->k[0];
			}
		}

		// 由Delete来释放节点 delete对象 指针置NULL，修改ftb等
		void merge_delete_leaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// b始终放要删除的 direct=true则 a<-b 否则是b<-a
			direct_delete(b, k, true);
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
			for (int i = 0;i < ly;i++) { // y原有值后移 lx~lx+ly-1 ~ 0~ly-1 
				x->k[lx + i] = y->k[i];
				x->addr[lx + i] = y->addr[i];
			}
			x->n += ly;
			y->n = 0;
			x->addr[N] = y->addr[N];
		}

		// 关键在于正确插入新增的键值 以及向上返回的键值
		int resize_delete_nonleaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// b始终放要删除的 direct=true则 a->b 否则是b<-a
			direct_delete(b, k, false);
			int la = a->n;
			int lb = b->n;
			int o = nlfmin - lb; // 计算b需要插入的元素数/a失去的元素数
			if (direct) { // a->b
				// 10 20 30 -> 40 50 == 10 20 30 -> 50
				// 10 20 30 -> (NULL) 35 50 == 10 -> 30 35 50 (20是直接丢弃)
				for (int i = 0;i < lb;i++) { // b原有值后移  o~lb+o-1 <- 0~lb-1
					b->k[lb + o - 1 - i] = b->k[lb - 1 - i];
					b->addr[lb + o - i] = b->addr[lb - i];
				}
				b->k[o - 1] = search_left(getnode(b->addr[0])); // 因为左边代表分支下最小的值
				b->addr[o] = b->addr[0];
				for (int i = 0;i < o - 1;i++) { // 移动key      0~o-2 <- la-o+1~la-1
					b->k[o - 2 - i] = a->k[la - 1 - i];
					//b->k[i] = a->k[la-o+1+i];
				}
				int res = a->k[la - o];
				for (int i = 0;i < o;i++) { // 移动地址 
					b->addr[o - 1 - i] = a->addr[la - i];
					//b->addr[i] = a->addr[la-o+1+i];
				}
				a->n -= o;
				b->n += o;
				//return search_left(getnode(b->addr[0]));
				return res;
			}
			else { //  b<-a
				// 10 20 -> 30 40 50 == 10 ->30 40 50
				// 10 25 (NULL) -> 30 40 50 == 10 25 30 ->  50 丢弃(40)  
				b->k[lb] = search_left(getnode(a->addr[0]));
				for (int i = 0;i < o - 1;i++) { // 移动key lb+1~lb+o-1 <- 0~o-2 o-2
					b->k[lb + 1 + i] = a->k[i];
				}
				int res = a->k[o - 1];
				for (int i = 0;i < o;i++) { // 移动地址  lb+1~lb+o <- 0~o-1 0-1
					b->addr[lb + 1 + i] = a->addr[i];
				}

				for (int i = 0;i < la - o;i++) { //  0~la-o-1 <-  o~la-1
					a->k[i] = a->k[o + i];
				}
				for (int i = 0;i < la - o + 1;i++) { // 0~la-o < o~la
					a->addr[i] = a->addr[o + i];
				}
				a->n -= o;
				b->n += o;
				//return search_left(getnode(a->addr[0]));
				return res;
			}
		}

		void merge_delete_nonleaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// b始终放要删除的 direct=true则 a<-b 否则是b<-a
			direct_delete(b, k, false);
			Node *x, *y;
			if (direct) {
				x = a;
				y = b;
			}
			else {
				x = b;
				y = a;
			}
			//x<-y
			// 1 2 -> 4 5 == 1 2 -> 5 和resize代码雷同 只是要全部移动走
			// 1 2 3 (NULL)->5 ==  1235
			int lx = x->n;
			int ly = y->n;
			x->k[lx] = search_left(getnode(y->addr[0]));
			for (int i = 0;i < ly;i++) { // 移动key
				x->k[lx + 1 + i] = y->k[i];
			}
			for (int i = 0;i < ly + 1;i++) { // 移动地址
				x->addr[lx + 1 + i] = y->addr[i];
			}
			x->n += ly + 1;
			y->n = 0;
		}

		void erase_node(Node* nd) {

		}

	public:

		/*
		btree(void);
		void create(int* arr, int len);
		long long search(int key);
		void insert(int key, long long value);
		void print_tree();
		void print_leaf();
		*/

		btree(void) { // 即使索引没有元素也要有数据块
			sid = 1;
			Node* nd = new Node();
			nd->flag = 1;
			root = setnode(nd);
			lfmin = (int)((N + 1) / 2);
			/*
			必须取整 例如N=4 如果是进1则叶节点至少有3个key 那么两个都是3的节点删除一个值变成 2 3 就没法merge或resize
			而取整 两个为2的key就可以处理
			*/
			nlfmin = lfmin - 1;
		}

		// 临时函数
		void create(int* arr, int len) {
			for (int i = 0;i < len;i++) {
				insert(arr[i], i + 1);
				//print_tree();
			}
		}

		long long search(int key) {
			long long res = NULLADDR;

			Node* p = getnode(root);
			if (p->n == 0) return res;  // root节点可能为空

			while (p->flag != 0) {		// 如果p不是叶节点，继续向下找
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p != NULL) {			// 考虑root节点左节点可能为NUL
				for (int i = 0;i < p->n;i++) {
					if (key == p->k[i]) {
						res = (p->addr[i]);
						break;
					}
				}
			}
			return res;
		}

		void insert(int key, long long value) {
			Node* ndroot = getnode(root);
			Node* p = ndroot;

			if (ndroot->n == 0) {	// root节点可能为空 新建叶节点 元素插入叶节点和根节点
				Node* nnd = new Node();
				nnd->flag = 0;
				long long addr = setnode(nnd);
				direct_insert(nnd, key, value, true);
				direct_insert(ndroot, key, addr, false);
				return;
			}

			std::stack<Node*> path; // 存放查询路径

			while (p != NULL && p->flag != 0) {
				path.push(p);		// 当前节点入栈
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p == NULL) {		// 如果p为空只可能是在root节点最左端 新建叶节点 地址放入根节点中
				Node* nnd = new Node();
				long long addr = setnode(nnd);
				nnd->flag = 0;
				direct_insert(nnd, key, value, true);
				ndroot->addr[0] = addr;
				nnd->addr[N] = ndroot->addr[1];
				return;
			}

			if (p->n < N) {				// 最简单的情况，数据节点能放下
				direct_insert(p, key, value, true);
				return;
			}

			// 之后的条件下需要分裂叶节点

			long long v = split_insert(p, key, value, true);
			int k = getnode(v)->k[0];   // 叶节点提供给上一层的key,value

			p = path.top();
			path.pop();

			if (p == ndroot && ndroot->addr[0] == NULLADDR) {
				/*
				 例如插入的值一直增大 1,2,3,4
				 导致 NULL [1] 1,2 [3] 3,4
				 所以 1,2 [3] 3,4
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

				// 以下情况还需继续分裂非叶节点

				v = split_insert(p, k, v, false);
				k = search_left(getnode(v));

				if (p == ndroot) { // 如果要分裂的是root节点 需要新建root节点
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

		bool delkey(int key) {
			if (search(key) == NULLADDR) return false; // 如果没有找到key则返回没有找到

			Node* ndroot = getnode(root);  // 再次搜索 不过要记录过程
			Node* p = ndroot; // 当前节点

			std::stack<Node*> path;
			std::stack<int> poffset; // 还要记录走的是哪个子节点

			do {
				int r = search_index(p, key);
				path.push(p);		// 当前节点入栈
				poffset.push(r);    // 偏移入栈
				p = getnode(p->addr[r]);
			} while (p->flag != 0);

			// 到达叶节点

			if (p->n - 1 >= lfmin) { // 如果叶节点足够大则直接删除
				direct_delete(p, key, true);
				return true;
			}

			Node* pv = path.top(); //当前节点父节点
			int pov = poffset.top(); //getnode(pv->addr[pov])即当前节点

			if (pv == ndroot) {
				if (pv->addr[0] == NULLADDR) {
					// 如果只有一个叶节点，无视节点数量的限制
					direct_delete(p, key, true);
					if (p->n == 0) { // 擦除叶节点
						erase_node(p);
						pv->n = 0;
					}
					return true;
				}
				else if (pv->n == 1) {
					int o = lfmin - p->n + 1; // leaf需要插入的key数
					int sign = 1 - pov; // pov = 1 sign = 0 || pov = 0 sign = 1 || sign < 0
					Node* other = getnode(pv->addr[sign]);
					if (other->n - o < lfmin) {
						// 如果另一个节点key不够 则合并 并挂在右边 保持只有root的左节点才能为NULL
						merge_delete_leaf(other, p, key); // 始终合并到左边节点
						erase_node(getnode(pv->addr[1]));
						pv->addr[1] = pv->addr[0];
						pv->addr[0] = NULLADDR;
						pv->k[0] = getnode(pv->addr[1])->k[0];
						return true;
					}
				}
			}

			// 先处理叶节点 估计要有重复代码了 

			int o = lfmin - p->n + 1;  // 还未删除节点p->n数量没减少
			int sign = pov == 0 ? 1 : pov - 1; //记录相邻节点的位置
			int tp = pov == 0 ? 0 : pov - 1;  // 记录上一层要删除key的位置
			Node* other = getnode(pv->addr[sign]);
			if (other->n - o >= lfmin) { // resize即可，不会删除节点，比较安全
				int ch = resize_delete_leaf(other, p, key);
				pv->k[tp] = ch;
				return true;
			}
			else { // merge
				merge_delete_leaf(other, p, key);
				erase_node(p);
				if (pv == ndroot || pv->n - 1 >= nlfmin) { // 上一层是root则一定能直接删除 因为root的特殊情况已经处理
					direct_delete(pv, pv->k[tp], false);
					return true;
				}
			}
			// 否则交给循环继续删除
			do {
				int curk = tp; // curk变成这一层需要删除key的位置
				p = path.top(); // 当前节点上移
				path.pop();
				poffset.pop();

				o = nlfmin + 1 - p->n; // 这时候还没有删除节点 p->n还是正常的 要额外-1
				pv = path.top();
				pov = poffset.top();

				sign = pov == 0 ? 1 : pov - 1; // 相邻节点位置
				tp = pov == 0 ? 0 : pov - 1; // 上一层可能要删除的位置
				other = getnode(pv->addr[sign]);
				if (other->n - o >= nlfmin) { // resize即可，不会删除节点，比较安全
					int ch = resize_delete_nonleaf(other, p, p->k[curk]);
					pv->k[tp] = ch;
					return true;
				}
				else { // merge
					merge_delete_nonleaf(other, p, p->k[curk]);
					erase_node(p);
					if ((pv == ndroot && pv->n > 1) || pv->n - 1 >= nlfmin) { // 上一层是root至少为1 其它至少为nlfmin
						direct_delete(pv, pv->k[tp], false);
						return true;
					}
					else if (pv == ndroot && pv->n == 1) { // 如果root节点要删除 重新设置root
						root = ndroot->addr[0];
						erase_node(ndroot);
						return true;
					}

					// 否则循环继续删除
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

			while (p->flag != 0) p = getnode(p->addr[0]);

			while (p != NULL) {
				print_leaf(p);
				p = getnode(p->addr[N]);
			}
		}
	};
}