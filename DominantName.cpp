#include<iostream>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
#include"HeapsLaw.h"
#include"Core.h"
#include"Macro.h"
#include"Graph.h"
#include"Fitness.h"
#include"Reader.h"

using namespace std;

map<int, vector<string>> body_names;

ofstream fout_similar_but_far_names("DominantName/SimilarButFarNames.txt");

bool solve(const pair<int, vector<string> > p) { // used at least a hundred times, has at least 4 names
	string body = rev_macro_to_num[p.first];
	vector<string> names = p.second;
	
	if(names.size() < 100) {
		return false;
	}

	int different_names = 0;
	int counter = 0;
	map<string, int> normalized;
	map<int, string> rev_normalized;
	{
		set<string> temp;
		for(string x : names) {
			if(temp.find(x) == temp.end()) {
				normalized[x] = counter;
				rev_normalized[counter] = x;
				counter++;
			}
			temp.insert(x);	
		}
		different_names = counter;
	}
	if(different_names < 4) {
		return false;
	}
	double *f[2];
	for(int i = 0; i < 2; i++) {
		f[i] = new double [different_names + 1];
		for(int j = 0; j < different_names; j++) {
			f[i][j] = 0;
		}
	}
	{
		double frac = 0.3;
		int bound = names.size() * frac;
		for (int i = 0; i < bound; i++) {
			int index2 = normalized[names[i]];
			f[0][index2] += 1.0 / bound;
		}
	}
	{
		double frac = 0.7;
		int bound = names.size() * frac;
		for (int i = names.size() - 1; i >= bound; i--) {
			int index2 = normalized[names[i]];
			f[1][index2] += 1.0 / (names.size() - bound);
		}
	}
	bool print = false;
	int max_index = 0;
	for(int j = 0; j < different_names; j++) {
		if(f[1][j] > 0.3) {
			print = true;
		}
		if(f[1][max_index] < f[1][j]) {
			max_index = j;
		}
	}
	if(print == false) {
		return false;
	}
	
	cerr << body << " " << names.size() << " " << f[1][max_index] << " " << rev_normalized[max_index] << endl;
	cout << body << " " << names.size()  << endl;
	for(int j = 0; j < different_names; j++) {
		cout << rev_normalized[j] << " , ";
	}
	cout << endl;
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < different_names; j++) {
			cout << f[i][j] << " , "; 
		}
		cout << endl;
	}
	cout << endl;
	return true;
}

int main() {
	folder = "RawOutput/Name/";
    if(SMART != "_smart" && SMART != "_nosmart") {
        cerr << " THERE IS A PROBLEM WITH THE SMART const variable" << endl;
        return 0;
    }
	{
		string s;
		ifstream fin_important_macros("important_macros.txt");
		while(getline(fin_important_macros, s)) {
			important_macros.insert(s);
		}
	}
    Read("All_Arxiv_Macros.txt");
	cerr << "Macro size:  " << macros.size() << endl;
    sort(macros.begin(), macros.end());
    
	
	for(int i = 0; i < (int) macros.size(); i++) {
		if(i % 1000000 == 0) { 
			cerr << i << endl;
		}
        macro_paper_count[macros[i].macro_number]++;
        for(int j = 0 ; j < (int)macros[i].authors.size(); j++) {
            int author = macros[i].authors[j];
            macros[i].experience.push_back(author_experience[author]);
        }
        if(i == 0 || !ExactSame(macros[i], macros[i - 1])) {
            for(int j = 0 ; j < (int)macros[i].authors.size(); j++) {
                int author = macros[i].authors[j];
                author_experience[author]++;
            }
        }
		if(body_names.find(macros[i].macro_number) == body_names.end()) {
			vector<string> temp;
			body_names.insert(make_pair(macros[i].macro_number, temp));
		}
		string body = rev_macro_to_num[macros[i].macro_number];
		if(body.length() < 2 || body.length() > 1000) {
			continue;
		}
		if(important_macros.find(rev_macro_to_num[macros[i].macro_number]) != important_macros.end()) {
			vector<string> temp = body_names[macros[i].macro_number];
			temp.push_back(macros[i].name);
			body_names.erase(macros[i].macro_number);
			body_names.insert(make_pair(macros[i].macro_number, temp));
		}
    }

    cerr << " $$$$$ " << endl;
	
//	int blah = 0;
	for(pair<int, vector<string> > p : body_names) {
		cerr << " -----> " << p.first << endl;
		solve(p);
/*
		blah++;
		if(blah == 10) {
			break;
		}
*/
	}
    return 0;
} 

