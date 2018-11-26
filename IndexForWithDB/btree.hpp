#pragma once

#include <iostream>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

constexpr auto N = 5;		  // �ڵ��ܷ���key������
constexpr auto NULLADDR = 0;  // NULL�����ݿ��ַ�еı�ʾ

namespace db {
	/*
	TODO �̳� virtual page
	ʵ��load dump close node_page
	�ο� tuple.hpp
	*/
	class Node
	{
	public:
		int n;					// �ڵ㵱ǰkey������
		int k[N];				// key����
		long long addr[N + 1];  // ���ݿ��ַ����
		/*
		����Ƿ�Ҷ�ڵ���0~N��������ڵ�����ݿ��ַ
		�����Ҷ�ڵ���0~N-1������ݽڵ��λ�� N���next
		*/
		int flag;				// 0:Ҷ�ӽڵ� 1:��Ҷָ��Ҷ�� 2:��Ҷָ���Ҷ

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
		std::vector<Node*> objlst;				// ������нڵ��ָ��
		std::unordered_map<long long, int> stb; // �������ݿ��ַ������objlst�е�ƫ�� 
		std::unordered_set<int> ftb;			// �������objlst�п��е�ƫ�� set�������Զ����� ���ڻ��տռ�

		/*
		keeper hold() loosen()
		hold(addr)���virtualpage Ȼ���virtualpage����load���ж��Ƿ�λ��
		��testnode����load ���жϱ�־λ Ȼ��loosen����
		���������ܵõ����ݿ��ַ Ӧ���п��п����ṹ bitmapҲ�� �������Ҳ��������
		����Ҫʵ��load��exit
		load���Ǹ���root�ڵ�����ݿ��ַ ����node�������������µ�node ����ȫ�ֹ�������ݽṹ�� ��Ŀǰgetset������һ��
		exit���Ǳ������нڵ㣬����exit()ָ�������д��
		*/

		long long sid;	// ģ�����ݿ��ַ
		long long root; // root�ڵ����ݿ��ַ ��Ϊroot�ڵ�Ҳ��䶯

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

		// �������ݿ��ַ��ȡ�ڵ�ָ�� ��ַΪ���򷵻�NULL
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
			if (ftb.size() != 0) {			// ����п��пռ� ���set�л�ȡƫ��
				key = *(ftb.begin());		// ***������������Ϊû���Թ�***
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
		void span_insert(Node* a, Node* b, int k, long long v, int o, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int i = o - a->n;
			if (i < 0) { // ԭ�ڵ�
				a->k[o] = k;
				a->addr[o + s] = v;
			}
			else {		 // �½ڵ�
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

		// ���ҷ�Ҷ�ڵ��������Ҷ�ڵ�ĵ�һ��key
		int search_left(Node* nd) {
			Node* r = nd;
			while (r->flag != 0) // ֱ������Ҷ�ڵ�
				r = getnode(r->addr[0]);
			return r->k[0];		 // Ӧ�ò������ �����ѷ�Ҷ�ڵ�������˱�Ȼ��ֵ
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

			nnd->n = lfmin - s;
			nd->n = N + 1 - nnd->n;
			nnd->flag = nd->flag;	// ���ѽڵ�λ��ͬһ��

			for (int i = N;i > r;i--)				// ��key�������һλ
				span_insert(nd, nnd, nd->k[i - 1], nd->addr[i + s - 1], i, if_leaf);
			span_insert(nd, nnd, k, v, r, if_leaf);
			for (int i = r - 1;i > nd->n - 1;i--)	// ��keyС��k����ҪǨ�Ƶ��µĽڵ�
				span_insert(nd, nnd, nd->k[i], nd->addr[i + s], i, if_leaf);

			if (if_leaf) {			// ����ڵ����
				nnd->addr[N] = nd->addr[N];
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
				if (i != 0) print_space(level); // ��һ��Ԫ�ز�������

				if (i != nd->n) std::cout << std::setw(3) << std::left << nd->k[i] << "--+";
				else std::cout << "   --+";

				if (nd->flag == 1) print_leaf(getnode(nd->addr[i]));
				else print_nonleaf(getnode(nd->addr[i]), level + 1);
			}
		}

		// ֱ��ɾ������򵥵����
		void direct_delete(Node* nd, int k, bool if_leaf) {
			int s = if_leaf ? 0 : 1;
			int r = 0;
			for (int i = 0;i < nd->n;i++) {
				if (k == nd->k[i]) {
					r = i;
					break;
				}
			} // ����һ�����ҵ���ӦԪ��
			int len = nd->n;
			// r ~ len-2 <- r+1 ~ len-1 
			for (int i = r + 1;i < len;i++) {
				nd->k[i - 1] = nd->k[i];
				nd->addr[i - 1 + s] = nd->addr[i + s];
			}
			nd->n--;
		}

		// b����Ҫɾ���Ľڵ� �����ұ߽ڵ���Сֵ(�ṩ���ϲ��޸�)
		int resize_delete_leaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			direct_delete(b, k, true);
			int la = a->n;
			int lb = b->n;
			int o = lfmin - lb;// ����b��Ҫ�����Ԫ����
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
				for (int i = 0;i < o;i++) { // a��Ԫ����ӵ�b�� lb~lb+o-1 <- 0~o-1 ok
					b->k[lb + i] = a->k[i];
					b->addr[lb + i] = a->addr[i];
				}
				for (int i = 0;i < la - o;i++) { // a�ڲ����λ 0~la-o-1 <- o~la-1 ok
					a->k[i] = a->k[i + o];
					a->addr[i] = a->addr[i + o];
				}
				a->n -= o;
				b->n += o;
				return a->k[0];
			}
		}

		// ��Delete���ͷŽڵ� delete���� ָ����NULL���޸�ftb��
		void merge_delete_leaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// bʼ�շ�Ҫɾ���� direct=true�� a<-b ������b<-a
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
			for (int i = 0;i < ly;i++) { // yԭ��ֵ���� lx~lx+ly-1 ~ 0~ly-1 
				x->k[lx + i] = y->k[i];
				x->addr[lx + i] = y->addr[i];
			}
			x->n += ly;
			y->n = 0;
			x->addr[N] = y->addr[N];
		}

		// �ؼ�������ȷ���������ļ�ֵ �Լ����Ϸ��صļ�ֵ
		int resize_delete_nonleaf(Node* a, Node* b, int k) {
			bool direct = a->k[0] < b->k[0] ? true : false;
			// bʼ�շ�Ҫɾ���� direct=true�� a->b ������b<-a
			direct_delete(b, k, false);
			int la = a->n;
			int lb = b->n;
			int o = nlfmin - lb; // ����b��Ҫ�����Ԫ����/aʧȥ��Ԫ����
			if (direct) { // a->b
				// 10 20 30 -> 40 50 == 10 20 30 -> 50
				// 10 20 30 -> (NULL) 35 50 == 10 -> 30 35 50 (20��ֱ�Ӷ���)
				for (int i = 0;i < lb;i++) { // bԭ��ֵ����  o~lb+o-1 <- 0~lb-1
					b->k[lb + o - 1 - i] = b->k[lb - 1 - i];
					b->addr[lb + o - i] = b->addr[lb - i];
				}
				b->k[o - 1] = search_left(getnode(b->addr[0])); // ��Ϊ��ߴ����֧����С��ֵ
				b->addr[o] = b->addr[0];
				for (int i = 0;i < o - 1;i++) { // �ƶ�key      0~o-2 <- la-o+1~la-1
					b->k[o - 2 - i] = a->k[la - 1 - i];
					//b->k[i] = a->k[la-o+1+i];
				}
				int res = a->k[la - o];
				for (int i = 0;i < o;i++) { // �ƶ���ַ 
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
				// 10 25 (NULL) -> 30 40 50 == 10 25 30 ->  50 ����(40)  
				b->k[lb] = search_left(getnode(a->addr[0]));
				for (int i = 0;i < o - 1;i++) { // �ƶ�key lb+1~lb+o-1 <- 0~o-2 o-2
					b->k[lb + 1 + i] = a->k[i];
				}
				int res = a->k[o - 1];
				for (int i = 0;i < o;i++) { // �ƶ���ַ  lb+1~lb+o <- 0~o-1 0-1
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
			// bʼ�շ�Ҫɾ���� direct=true�� a<-b ������b<-a
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
			// 1 2 -> 4 5 == 1 2 -> 5 ��resize������ͬ ֻ��Ҫȫ���ƶ���
			// 1 2 3 (NULL)->5 ==  1235
			int lx = x->n;
			int ly = y->n;
			x->k[lx] = search_left(getnode(y->addr[0]));
			for (int i = 0;i < ly;i++) { // �ƶ�key
				x->k[lx + 1 + i] = y->k[i];
			}
			for (int i = 0;i < ly + 1;i++) { // �ƶ���ַ
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

		btree(void) { // ��ʹ����û��Ԫ��ҲҪ�����ݿ�
			sid = 1;
			Node* nd = new Node();
			nd->flag = 1;
			root = setnode(nd);
			lfmin = (int)((N + 1) / 2);
			/*
			����ȡ�� ����N=4 ����ǽ�1��Ҷ�ڵ�������3��key ��ô��������3�Ľڵ�ɾ��һ��ֵ��� 2 3 ��û��merge��resize
			��ȡ�� ����Ϊ2��key�Ϳ��Դ���
			*/
			nlfmin = lfmin - 1;
		}

		// ��ʱ����
		void create(int* arr, int len) {
			for (int i = 0;i < len;i++) {
				insert(arr[i], i + 1);
				//print_tree();
			}
		}

		long long search(int key) {
			long long res = NULLADDR;

			Node* p = getnode(root);
			if (p->n == 0) return res;  // root�ڵ����Ϊ��

			while (p->flag != 0) {		// ���p����Ҷ�ڵ㣬����������
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p != NULL) {			// ����root�ڵ���ڵ����ΪNUL
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

			if (ndroot->n == 0) {	// root�ڵ����Ϊ�� �½�Ҷ�ڵ� Ԫ�ز���Ҷ�ڵ�͸��ڵ�
				Node* nnd = new Node();
				nnd->flag = 0;
				long long addr = setnode(nnd);
				direct_insert(nnd, key, value, true);
				direct_insert(ndroot, key, addr, false);
				return;
			}

			std::stack<Node*> path; // ��Ų�ѯ·��

			while (p != NULL && p->flag != 0) {
				path.push(p);		// ��ǰ�ڵ���ջ
				int r = search_index(p, key);
				p = getnode(p->addr[r]);
			}

			if (p == NULL) {		// ���pΪ��ֻ��������root�ڵ������ �½�Ҷ�ڵ� ��ַ������ڵ���
				Node* nnd = new Node();
				long long addr = setnode(nnd);
				nnd->flag = 0;
				direct_insert(nnd, key, value, true);
				ndroot->addr[0] = addr;
				nnd->addr[N] = ndroot->addr[1];
				return;
			}

			if (p->n < N) {				// ��򵥵���������ݽڵ��ܷ���
				direct_insert(p, key, value, true);
				return;
			}

			// ֮�����������Ҫ����Ҷ�ڵ�

			long long v = split_insert(p, key, value, true);
			int k = getnode(v)->k[0];   // Ҷ�ڵ��ṩ����һ���key,value

			p = path.top();
			path.pop();

			if (p == ndroot && ndroot->addr[0] == NULLADDR) {
				/*
				 ��������ֵһֱ���� 1,2,3,4
				 ���� NULL [1] 1,2 [3] 3,4
				 ���� 1,2 [3] 3,4
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

				// �����������������ѷ�Ҷ�ڵ�

				v = split_insert(p, k, v, false);
				k = search_left(getnode(v));

				if (p == ndroot) { // ���Ҫ���ѵ���root�ڵ� ��Ҫ�½�root�ڵ�
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
			if (search(key) == NULLADDR) return false; // ���û���ҵ�key�򷵻�û���ҵ�

			Node* ndroot = getnode(root);  // �ٴ����� ����Ҫ��¼����
			Node* p = ndroot; // ��ǰ�ڵ�

			std::stack<Node*> path;
			std::stack<int> poffset; // ��Ҫ��¼�ߵ����ĸ��ӽڵ�

			do {
				int r = search_index(p, key);
				path.push(p);		// ��ǰ�ڵ���ջ
				poffset.push(r);    // ƫ����ջ
				p = getnode(p->addr[r]);
			} while (p->flag != 0);

			// ����Ҷ�ڵ�

			if (p->n - 1 >= lfmin) { // ���Ҷ�ڵ��㹻����ֱ��ɾ��
				direct_delete(p, key, true);
				return true;
			}

			Node* pv = path.top(); //��ǰ�ڵ㸸�ڵ�
			int pov = poffset.top(); //getnode(pv->addr[pov])����ǰ�ڵ�

			if (pv == ndroot) {
				if (pv->addr[0] == NULLADDR) {
					// ���ֻ��һ��Ҷ�ڵ㣬���ӽڵ�����������
					direct_delete(p, key, true);
					if (p->n == 0) { // ����Ҷ�ڵ�
						erase_node(p);
						pv->n = 0;
					}
					return true;
				}
				else if (pv->n == 1) {
					int o = lfmin - p->n + 1; // leaf��Ҫ�����key��
					int sign = 1 - pov; // pov = 1 sign = 0 || pov = 0 sign = 1 || sign < 0
					Node* other = getnode(pv->addr[sign]);
					if (other->n - o < lfmin) {
						// �����һ���ڵ�key���� ��ϲ� �������ұ� ����ֻ��root����ڵ����ΪNULL
						merge_delete_leaf(other, p, key); // ʼ�պϲ�����߽ڵ�
						erase_node(getnode(pv->addr[1]));
						pv->addr[1] = pv->addr[0];
						pv->addr[0] = NULLADDR;
						pv->k[0] = getnode(pv->addr[1])->k[0];
						return true;
					}
				}
			}

			// �ȴ���Ҷ�ڵ� ����Ҫ���ظ������� 

			int o = lfmin - p->n + 1;  // ��δɾ���ڵ�p->n����û����
			int sign = pov == 0 ? 1 : pov - 1; //��¼���ڽڵ��λ��
			int tp = pov == 0 ? 0 : pov - 1;  // ��¼��һ��Ҫɾ��key��λ��
			Node* other = getnode(pv->addr[sign]);
			if (other->n - o >= lfmin) { // resize���ɣ�����ɾ���ڵ㣬�Ƚϰ�ȫ
				int ch = resize_delete_leaf(other, p, key);
				pv->k[tp] = ch;
				return true;
			}
			else { // merge
				merge_delete_leaf(other, p, key);
				erase_node(p);
				if (pv == ndroot || pv->n - 1 >= nlfmin) { // ��һ����root��һ����ֱ��ɾ�� ��Ϊroot����������Ѿ�����
					direct_delete(pv, pv->k[tp], false);
					return true;
				}
			}
			// ���򽻸�ѭ������ɾ��
			do {
				int curk = tp; // curk�����һ����Ҫɾ��key��λ��
				p = path.top(); // ��ǰ�ڵ�����
				path.pop();
				poffset.pop();

				o = nlfmin + 1 - p->n; // ��ʱ��û��ɾ���ڵ� p->n���������� Ҫ����-1
				pv = path.top();
				pov = poffset.top();

				sign = pov == 0 ? 1 : pov - 1; // ���ڽڵ�λ��
				tp = pov == 0 ? 0 : pov - 1; // ��һ�����Ҫɾ����λ��
				other = getnode(pv->addr[sign]);
				if (other->n - o >= nlfmin) { // resize���ɣ�����ɾ���ڵ㣬�Ƚϰ�ȫ
					int ch = resize_delete_nonleaf(other, p, p->k[curk]);
					pv->k[tp] = ch;
					return true;
				}
				else { // merge
					merge_delete_nonleaf(other, p, p->k[curk]);
					erase_node(p);
					if ((pv == ndroot && pv->n > 1) || pv->n - 1 >= nlfmin) { // ��һ����root����Ϊ1 ��������Ϊnlfmin
						direct_delete(pv, pv->k[tp], false);
						return true;
					}
					else if (pv == ndroot && pv->n == 1) { // ���root�ڵ�Ҫɾ�� ��������root
						root = ndroot->addr[0];
						erase_node(ndroot);
						return true;
					}

					// ����ѭ������ɾ��
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