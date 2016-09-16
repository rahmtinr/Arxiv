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


bool solve(const pair<int, vector<string> > p) {
	int body = p.first;
	vector<string> names = p.second;
	
	cerr << p.first << endl;
	cerr << names.size() << endl;
	if(names.size() < 10) {
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
	cout << body << " " << names.size() << endl;
	double *f[11], *derivation[11];
	for(int i = 0; i < 11; i++) {
		f[i] = new double [different_names + 1];
		for(int j = 0; j < different_names; j++) {
			f[i][j] = 0;
		}
		derivation[i] = new double [different_names + 1];
	}
	int index = 0;
	for(double frac = 0.1; frac <= 1; frac += 0.1, index++) {
		int bound = names.size() * frac;
		for (int i = 0; i < bound; i++) {
			int index2 = normalized[names[i]];
			f[index][index2] += 1 / bound;
		}
	}
	for(int i = 0; i < 9; i++) {
		for(int j = 0; j < different_names; j++) {
			derivation[i][j] = f[i+1][j] - f[i][j];
		}
	}
	
	for(int j = 0; j < different_names; j++) {
		if(f[9][j] > 0.3) {
			cerr << "Biggest fraction: " << f[9][j] << " macro body:" << rev_macro_to_num[body] << " for macro name: " << rev_normalized[j] << endl;
			cerr << "-----> " << f[4][j] << endl;
		}
	}
	return true;
}

int main() {
    if(SMART != "_smart" && SMART != "_nosmart") {
        cerr << " THERE IS A PROBLEM WITH THE SMART const variable" << endl;
        return 0;
    }
    Read();
	cerr << "Macro size:  " << macros.size() << endl;
    sort(macros.begin(), macros.end());
    
	for(int i = 0; i < (int) macros.size(); i++) {
		if(i % 100000 == 0) { 
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
		vector<string> temp = body_names[macros[i].macro_number];
		temp.push_back(macros[i].name);
		body_names.erase(macros[i].macro_number);
		body_names.insert(make_pair(macros[i].macro_number, temp));
    }

    cerr << " $$$$$ " << endl;
	
	int blah = 0;
	for(pair<int, vector<string> > p : body_names) {
		cerr << " -----> " << p.first << endl;
		solve(p);
		blah++;
		if(blah == 10) {
			break;
		}
	}

    cerr << "-------------------------------------->" << data_points << endl;
    return 0;
} 

