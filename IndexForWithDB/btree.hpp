#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>

constexpr int BUFFSIZE = 100; // ����ڵ��Сȷ��
constexpr int N = (BUFFSIZE-16)/12;		  // �ڵ��ܷ���key������ ����Ϊ7
constexpr auto NULLADDR = 0;  // NULL�����ݿ��ַ�еı�ʾ
constexpr int MAXLEN = 20; // �ַ����������20
constexpr int HALFLEN = (int)(BUFFSIZE - 16) / 2;
/*
4[flag]+4[num]+4*n[key]+8*(n+1)[addr]=BUFFSIZE
n = [(BUFFSIZE-16)/12] = 4
*/

/*
�����ַ�����
��ʼռ�� 4+4+8=16
ÿ����һ���ַ���ռ��8+size(str) 48
��������װ��6���ַ���
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


		// true ���ܲ���
		bool full(std::string key) {
			int cur = size();
			cur += key.length() + 9;
			return cur > BUFFSIZE;
		}

		/*
		��������½ڵ�key����Ŀ Ҳ���ԣ���Ҷ�ڵ�Ҫɾ������ߵ���Ŀ
		��ȥһ��Ҫռ�õ�16bytes ʣ�µ��ַ���ռ�õĴ�Сһ��Ҫ����һ��
		*/
		int split(std::string key) {
			int r = search(key);
			int half = (int)(BUFFSIZE - 16) / 2;
			int cur = 0;
			for (int i = 0;i <r;i++) {
				cur += 9 + k[i].length();
				if(cur>half) return n - i; // n + 1 - (i + 1); ���� - ��������Ŀ
			}
			cur += 9 + key.length();
			if (cur > half) return n - r;
			for (int i = r;i < n;i++) {
				cur += 9 + k[i].length();
				if (cur > half) return n-1 - i; // ��Ҫ���һ��key 
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

		// ����һ�뷵��true
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
				if (direct) cur -= k[n - 1 - i].length() + 9; // �Ӵ�С
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
			int remain = size()-16; // �ڵ�ʣ���С
			int cur = len+9; // �½ڵ����Ĵ�С
			int o = 0;
			for (int i = 0;i < n;i++) {
				if (cur >= hf || remain < hf) { // �½ڵ㲻�ܼ��볬��һ�� �ɽڵ㲻�ܼ��ٳ���һ��
					o = i;
					break;
				}
				if (direct) {
					int t = k[n - 1 - i].length() + 9; // �Ӵ�С
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
		std::vector<Node*> objlst;				// ������нڵ��ָ��
		std::unordered_map<long long, int> stb; // �������ݿ��ַ������objlst�е�ƫ�� 
		std::set<int> ftb;						// �������objlst�п��е�ƫ�� set�������Զ����� ���ڻ��տռ�

		long long root; // root�ڵ�����ݿ��ַ ��Ϊroot�ڵ�Ҳ��䶯

		long long sid;	// ��ʼ��Ϊ1 ==NULLADDR+1
		long long getfreeaddr() { // ģ��Կ��е�ַ�Ĳ���
			long long r = sid;
			sid++;
			return r;
		}

		// ���ݿ��ַ -> �ڵ�ָ�� ���ܷ���NULL
		Node* getnode(long long addr) {
			if (addr == NULLADDR) return NULL; 
			return objlst[stb[addr]];
		}

		// �ڵ�ָ�� �����objlst�� -> ��������ݿ��ַ
		long long setnode(Node* nd) {
			long long addr = getfreeaddr();

			int key = 0;
			if (ftb.size() != 0) {			// ����п��пռ� ���set�л�ȡƫ��
				key = *(ftb.begin());		
				ftb.erase(key);
				objlst[key] = nd;
			}
			else {
				key = (int)objlst.size();	// size_t->int Ӧ�ò��ᳬ
				objlst.push_back(nd);
			}
			stb[addr] = key;
			return addr;
		}

		// ��ڵ����
		void span_insert(Node* a, Node* b, std::string k, long long v, int o, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int i = o - a->n;
			if (i < 0) { // ԭ�ڵ�
				a->k[o] = k;
				a->a[o + s] = v;
			}
			else {		 // �½ڵ�
				b->k[i] = k;
				b->a[i + s] = v;
			}
		}

		// ����k�ڽڵ�ԭ�����е�λ�� �Ӵ���С����
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

		// ���ҷ�Ҷ�ڵ��������Ҷ�ڵ�ĵ�һ��key
		std::string search_left(Node* nd) {
			Node* r = nd;
			while (r->flag != 1) // ֱ������Ҷ�ڵ�
				r = getnode(r->a[0]);
			return r->k[0];		 // Ӧ�ò������ �����ѷ�Ҷ�ڵ�������˱�Ȼ��ֵ
		}

		// �ڵ����㹻�Ŀռ������Ԫ��
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

		// �ڵ����ݳ���N�������½ڵ�ĵ�ַ
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
			nnd->flag = nd->flag;	// ���ѽڵ�λ��ͬһ��

			for (int i = ln;i > r;i--)				// ��key�������һλ
				span_insert(nd, nnd, nd->k[i - 1], nd->a[i + s - 1], i, if_leaf);
			span_insert(nd, nnd, k, v, r, if_leaf);
			for (int i = r - 1;i > nd->n - 1;i--)	// ��keyС��k����ҪǨ�Ƶ��µĽڵ�
				span_insert(nd, nnd, nd->k[i], nd->a[i + s], i, if_leaf);

			if (if_leaf) {			// ����ڵ����
				nnd->next = nd->next;
				nd->next = addr;
			}
			else {
				/*
				ԭ����3��������֮��Ϊ2+1����Ϊ�µĽڵ㲻����NULL����Ҫɾ��һ��key���������Ҷ���Ϊ�½ڵ�����
				��ԭ�ڵ����Ҷ˵�keyɾ����p�ŵ��½ڵ������
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

		// ����������ȱ���
		void print_nonleaf(Node* nd, int level) {
			for (int i = 0;i < nd->n + 1;i++) {
				if (i != 0) print_space(level); // ��һ��Ԫ�ز�������

				if (i != nd->n) std::cout << std::setw(15) << std::left << nd->k[i] << "--+";
				else std::cout << "               --+";

				if (nd->flag == 2) print_leaf(getnode(nd->a[i]));
				else print_nonleaf(getnode(nd->a[i]), level + 1);
			}
		}

		// ֱ��ɾ������򵥵����
		void direct_delete(Node* nd, std::string k, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = nd->search(k); // ����һ�����ҵ���ӦԪ��
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

		// b����Ҫɾ���Ľڵ� �����ұ߽ڵ���Сֵ(�ṩ���ϲ��޸�)
		std::string resize_delete_leaf(Node* a, Node* b) { // o��b��Ҫ�����Ԫ����
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
				for (int i = 0;i < o;i++) { // a��Ԫ����ӵ�b�� lb~lb+o-1 <- 0~o-1 ok
					b->k[lb + i] = a->k[i];
					b->a[lb + i] = a->a[i];
				}
				for (int i = 0;i < la - o;i++) { // a�ڲ����λ 0~la-o-1 <- o~la-1 ok
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

		// ��Delete���ͷŽڵ� delete���� ָ����NULL���޸�ftb��
		bool merge_delete_leaf(Node* a, Node* b) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// bʼ�շ�Ҫɾ���� direct=true�� a<-b ������b<-a
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
			for (int i = 0;i < ly;i++) { // yԭ��ֵ���� lx~lx+ly-1 ~ 0~ly-1 
				x->k[lx + i] = y->k[i];
				x->a[lx + i] = y->a[i];
			}
			x->n += ly;
			y->n = 0;
			x->next = y->next;
			return true;
		}

		// �ؼ�������ȷ���������ļ�ֵ �Լ����Ϸ��صļ�ֵ
		std::string resize_delete_nonleaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// bʼ�շ�Ҫɾ���� direct=true�� a->b ������b<-a
			int la = a->n;
			int lb = b->n;

			int hf = (int)(BUFFSIZE - 16) / 2;
			int cur = b->size();
			int o = 0;
			std::string tp;

			if (direct) { // a->b
				// 10 20 30 -> 40 50 == 10 20 30 -> 50
				// 10 20 30 -> (NULL) 35 50 == 10 -> 30 35 50 (20��ֱ�Ӷ���)
				tp = search_left(getnode(b->a[0]));
				o = a->resize(true, tp.length());

				b->k.resize(lb + o);
				b->a.resize(lb + o + 1);

				for (int i = 0;i < lb;i++) { // bԭ��ֵ����  o~lb+o-1 <- 0~lb-1
					b->k[lb + o - 1 - i] = b->k[lb - 1 - i];
					b->a[lb + o - i] = b->a[lb - i];
				}
				b->k[o - 1] = tp; // ��Ϊ��ߴ����֧����С��ֵ
				b->a[o] = b->a[0];
				for (int i = 0;i < o - 1;i++) { // �ƶ�key      0~o-2 <- la-o+1~la-1
					b->k[o - 2 - i] = a->k[la - 1 - i];
					//b->k[i] = a->k[la-o+1+i];
				}
				std::string res = a->k[la - o];
				for (int i = 0;i < o;i++) { // �ƶ���ַ 
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
				// 10 25 (NULL) -> 30 40 50 == 10 25 30 ->  50 ����(40)  
				b->k[lb] = tp;
				for (int i = 0;i < o - 1;i++) { // �ƶ�key lb+1~lb+o-1 <- 0~o-2 o-2
					b->k[lb + 1 + i] = a->k[i];
				}
				std::string res = a->k[o - 1];
				for (int i = 0;i < o;i++) { // �ƶ���ַ  lb+1~lb+o <- 0~o-1 0-1
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
			// bʼ�շ�Ҫɾ���� direct=true�� a<-b ������b<-a
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
			// 1 2 -> 4 5 == 1 2 -> 5 ��resize������ͬ ֻ��Ҫȫ���ƶ���
			// 1 2 3 (NULL)->5 ==  1235
			int lx = x->n;
			int ly = y->n;
			x->k.resize(lx + ly + 1);
			x->a.resize(lx + ly + 2);
			x->k[lx] = st;
			for (int i = 0;i < ly;i++) { // �ƶ�key
				x->k[lx + 1 + i] = y->k[i];
			}
			for (int i = 0;i < ly + 1;i++) { // �ƶ���ַ
				x->a[lx + 1 + i] = y->a[i];
			}
			x->n += ly + 1;
			y->n = 0;
			return true;
		}				

		void erase_node(long long addr) { // ���addr��Ӧ��ָ��
			Node* nd = getnode(addr);
			delete nd;
			int i = stb[addr];
			stb.erase(addr);
			objlst[i] = NULL;
			ftb.insert(i);
		}

	public:

		btree(void) { // ��ʹ����û��Ԫ��ҲҪ�����ݿ�
			sid = 1;
			Node* nd = new Node();
			nd->flag = 2;
			root = setnode(nd);
		}

		long long search(std::string key) {
			long long res = NULLADDR;

			Node* p = getnode(root);
			if (p->n == 0) return res;  // root�ڵ����Ϊ��

			while (p->flag != 1) {		// ���p����Ҷ�ڵ㣬����������
				int r = p->search(key);
				p = getnode(p->a[r]);
			}

			if (p != NULL) {			// ����root�ڵ���ڵ����ΪNUL
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

			if (ndroot->n == 0) {	// root�ڵ�Ϊ�վ���Ҫ�½�Ҷ�ڵ�
				Node* nnd = new Node();
				nnd->flag = 1;
				direct_insert(nnd, key, value, true);

				long long addr = setnode(nnd); // Ŀǰ��ַ��btree�л�ȡ���Ժ�Ӧ�ô�node�л�ȡ���󶨵ĵ�ַ
				direct_insert(ndroot, key, addr, false);
				return;
			}

			Node* p = ndroot;
			std::stack<Node*> path; // ��Ų�ѯ·��

			do {
				path.push(p);
				int r = search_index(p, key);
				p = getnode(p->a[r]);
			} while (p != NULL && p->flag != 1);

			if (p == NULL) {		// ���pΪ��ֻ��������root�ڵ������ �½�Ҷ�ڵ� ��ַ������ڵ���
				Node* nnd = new Node();
				long long addr = setnode(nnd);
				nnd->flag = 1;
				direct_insert(nnd, key, value, true);
				ndroot->a[0] = addr;
				nnd->next = ndroot->a[1];
				return;
			}

			if (!p->full(key)) {	// ���ݽڵ��ܷ���
				direct_insert(p, key, value, true);
				return;
			}

			// ֮�����������Ҫ����Ҷ�ڵ�

			long long v = split_insert(p, key, value, true);
			std::string k = getnode(v)->k[0];   // Ҷ�ڵ��ṩ����һ���key,value

			

			// ����ͨ�õ�ѭ�� ��k,v����p��
			do {
				p = path.top();
				path.pop();

				if (p == ndroot && ndroot->a[0] == NULLADDR) {
					/*
					 ��������ֵһֱ���� 1,2,3,4
					 ���� NULL [1] 1,2 [3] 3,4
					 ���� 1,2 [3] 3,4
					*/
					ndroot->k[0] = k;
					ndroot->a[0] = ndroot->a[1];
					ndroot->a[1] = v;
					return;
				}

				if (!p->full(k)) { // �����Ҷ�ڵ�ŵ���
					direct_insert(p, k, v, false);
					break;
				}

				// �����������������ѷ�Ҷ�ڵ�

				v = split_insert(p, k, v, false);
				k = search_left(getnode(v));
				

				if (p == ndroot) { // ���Ҫ���ѵ���root�ڵ� ��Ҫ�½�root�ڵ�
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
			if (search(key) == NULLADDR) return false; // ���û���ҵ�key�򷵻�û���ҵ�

			Node* ndroot = getnode(root);  // �ٴ����� ����Ҫ��¼����
			Node* p = ndroot; // ��ǰ�ڵ�

			std::stack<Node*> path;
			std::stack<int> poffset; // ��Ҫ��¼�ߵ����ĸ��ӽڵ�

			do {
				int r = p->search(key);
				path.push(p);		// ��ǰ�ڵ���ջ
				poffset.push(r);    // ƫ����ջ
				p = getnode(p->a[r]);
			} while (p->flag != 1);

			// ����Ҷ�ڵ�
			direct_delete(p, key, true);
			if (p->half()) return true; // ���Ҷ�ڵ��㹻����ֱ��ɾ��

			Node* pv = path.top(); //��ǰ�ڵ㸸�ڵ�
			int pov = poffset.top(); //getnode(pv->addr[pov])����ǰ�ڵ�

			if (pv == ndroot) {
				if (pv->a[0] == NULLADDR) {
					// ���ֻ��һ��Ҷ�ڵ㣬���ӽڵ�����������
					if (p->n == 0) { // ����Ҷ�ڵ�
						erase_node(pv->a[1]);
						pv->n = 0;
					}
					return true;
				}
				else if (pv->n == 1) {
					int sign = 1 - pov; // pov = 1 sign = 0 || pov = 0 sign = 1 || sign < 0
					Node* other = getnode(pv->a[sign]);
					if (merge_delete_leaf(other, p)) {
						// �����һ���ڵ�key���� ��ϲ� �������ұ� ����ֻ��root����ڵ����ΪNULL
						// ʼ�պϲ�����߽ڵ�
						erase_node(pv->a[1]);
						pv->a[1] = pv->a[0];
						pv->a[0] = NULLADDR;
						pv->k[0] = getnode(pv->a[1])->k[0];
						return true;
					}
				}
			}

			// �ȴ���Ҷ�ڵ� ����Ҫ���ظ������� 

			int sign = pov == 0 ? 1 : pov - 1; //��¼���ڽڵ��λ��
			int tp = pov == 0 ? 0 : pov - 1;  // ��¼��һ��Ҫɾ��key��λ��
			Node* other = getnode(pv->a[sign]);
			if (merge_delete_leaf(other, p)) {
				long long eaddr = pv->a[tp + 1];
				erase_node(eaddr);
				if (pv == ndroot || pv->size() - pv->k[tp].length() - 9 >= HALFLEN) { // ��һ����root��һ����ֱ��ɾ�� ��Ϊroot����������Ѿ�����
					direct_delete(pv, pv->k[tp], false);
					return true;
				}
			} else { // resize���ɣ�����ɾ���ڵ㣬�Ƚϰ�ȫ
				pv->k[tp] = resize_delete_leaf(other, p);
				return true;
			}
		
			// ���򽻸�ѭ������ɾ��
			do {
				int curk = tp; // curk�����һ����Ҫɾ��key��λ��
				p = path.top(); // ��ǰ�ڵ�����
				path.pop();
				poffset.pop();

				pv = path.top();
				pov = poffset.top();

				sign = pov == 0 ? 1 : pov - 1; // ���ڽڵ�λ��
				tp = pov == 0 ? 0 : pov - 1; // ��һ�����Ҫɾ����λ��
				other = getnode(pv->a[sign]);
				direct_delete(p, p->k[curk], false);
				if (merge_delete_nonleaf(other, p)) {
					long long eaddr = pv->a[tp + 1];
					erase_node(eaddr);
					if ((pv == ndroot && pv->n > 1) || pv->size() - pv->k[tp].length() - 9 >= HALFLEN) { // ��һ����root����Ϊ1 ��������Ϊnlfmin
						direct_delete(pv, pv->k[tp], false);
						return true;
					}
					else if (pv == ndroot && pv->n == 1) { // ���root�ڵ�Ҫɾ�� ��������root
						long long eaddr = root;
						root = ndroot->a[0];
						erase_node(eaddr);
						return true;
					}
				} else { // resize���ɣ�����ɾ���ڵ㣬�Ƚϰ�ȫ
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