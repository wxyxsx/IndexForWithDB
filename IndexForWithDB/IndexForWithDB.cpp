#include "pch.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <string>

using namespace std;

int main()
{
	//db::btree* tr = new db::btree();
	//vector<int> arr;
	//for (int i = 1;i < 501;i++) arr.push_back(i);
	//random_shuffle(arr.begin(), arr.end());
	//int v = 1;
	//for (vector<int>::iterator it = arr.begin();it != arr.end();++it) {
	//	tr->insert(*it, v);
	//	v++;
	//}
	//tr->print_tree();
	//random_shuffle(arr.begin(), arr.end());
	//v = 1;
	//for (vector<int>::iterator it = arr.begin();it != arr.end();++it) {
	//	tr->delkey(*it);
	//	v++;
	//	if (v % 100 == 0) {
	//		tr->print_tree();
	//		tr->print_leaf();
	//	}
	//}
	//random_shuffle(arr.begin(), arr.end());
	//v = 1;
	//for (vector<int>::iterator it = arr.begin();it != arr.end();++it) {
	//	tr->insert(*it, v);
	//	v++;
	//}
	//tr->print_tree();
	string teststr[334] = { "bbcs","clear","sides","deal","view","delivered","worse","reporters","have","into",
		"border","time","joint","the","thing","brexiteers","policy","might","prominent","proposals","there","regaining",
		"however","that","street","able","britains","end","negotiate","period","deals","result","another","house:","working",
		"sold","editor","customs","tory","donald","allow","kick","labour","event","export","suggesting","given","uk-eu","us",
		"prevent","exactly","no-one","some","prepare","it","uk","on","laying","advantageous","further","from","moment","mark",
		"currently","people","designed","united","largest","brexit","trump","wants","new","same","kingdom","vote","claiming",
		"mps","economy","great","days","jon","billions","withdrawal","sounds","you","seriously","snps","at","been","around",
		"british","legal","line","has","president","concerned","was","mp","get","washington","because","take","both","market",
		"acknowledged","dups","these","posts","its","aspect","now","delayed","sir","look","independent","suggested","did",
		"referendum","wouldnt","lucas","caroline","an","july","dodds","responding","december","countries","be","good",
		"eurosceptics","which","application","insurance","certainty","five","insisted","after","buts","open","later",
		"bilateral","north","needed","or","green","mays","struck","like","term","free-trade","party","theresa","brought",
		"told","not","until","pm","triggered","insisting","eu","if","again","home","tied","he","specify","dont","more",
		"through","basis","francois","trumps","sign","we","host","far","fought","had","members","may","appear","agreed",
		"2021","trade","including","before","pounds","absolutely","return","dem","being","businesses","as","hours","iain",
		"knew","visit","will","carry","between","vince","whether","single","parliament","fear","transition","fishermen",
		"could","over","unveiled","post-brexit","botched","force","of","way","2020","least","blackford","backstop","scottish",
		"meant","states","spokesman","while","leader","said","aims","prime","commons","were","provocative","unable","all","leave",
		"determine","bad","control","jeremy","for","to","with","doing","urge","met","senior","groups","her","forging","written",
		"times","lib","added","sopel","remarks","dodo","mrs","cable","strike","mr","put","downing","faced","allowed","criticism",
		"heavy","us-uk","rules","by","sunday","irish","much","administration","laws","money","full","very","world","can","america",
		"communication","in","nigel","his","under","flows","ifs","but","right","this","11","dead","threaten","sustained","others",
		"come","our","they","agreement","no","and","think","him","came","comments","what","white","called","january","formed","out",
		"is","long","outside","use","already","10","britain","so","groundwork","monday","during","possible","off","among","continue",
		"backbencher","she","corbyn","borders","ambitious","brussels","would","minister","absolute" };
	string s1 = "abc~";
	string s2 = "abcz";
	bool a = s1>s2;
	cout << s1 << ' ' << s2 << endl;
	cout << a << endl;
	return 0;
}