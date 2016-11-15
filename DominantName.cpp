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

map<int, vector<int>> body_names;

ofstream fout_name_evolution;
double RATIO;

bool solve(const pair<int, vector<int> > p) { // used at least a hundred times, has at least 4 names
	string body = rev_macro_to_num[p.first];
	vector<int> indecies = p.second;
	
	if(indecies.size() < 100) {
		return false;
	}

	int different_names = 0;
	int counter = 0;
	map<string, int> normalized;
	map<int, string> rev_normalized;
	{
		set<string> temp;
		for(int index : indecies) {
			string x = macros[index].name;
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
		double frac = RATIO;
		int bound = indecies.size() * frac;
		for (int i = 0; i < bound; i++) {
			int index2 = normalized[macros[indecies[i]].name];
			f[0][index2] += 1.0 / bound;
		}
	}
	{
		double frac = 1 - RATIO;
		int bound = indecies.size() * frac;
		for (int i = indecies.size() - 1; i >= bound; i--) {
			int index2 = normalized[macros[indecies[i]].name];
			f[1][index2] += 1.0 / (indecies.size() - bound);
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
	
	cerr << body << " " << indecies.size() << " " << f[1][max_index] << " " << rev_normalized[max_index] << endl;
	fout_name_evolution << body << " " << indecies.size()  << endl;
	for(int j = 0; j < different_names; j++) {
		fout_name_evolution << rev_normalized[j] << " , ";
	}
	fout_name_evolution << endl;
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < different_names; j++) {
			fout_name_evolution << f[i][j] << " , "; 
		}
		fout_name_evolution << endl;
	}
	fout_name_evolution << endl;
	return true;
}

int main(int argc, char **argv) {
	RATIO = stod(argv[1]);
	fout_name_evolution.open("DominantName/NameEvolution_" + to_string(RATIO).substr(0,3) + ".txt");
	cerr << "RATIO: " << RATIO << endl;
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
			vector<int> temp;
			body_names.insert(make_pair(macros[i].macro_number, temp));
		}
		string body = rev_macro_to_num[macros[i].macro_number];
		if(body.length() < 2 || body.length() > 1000) {
			continue;
		}
		if(important_macros.find(rev_macro_to_num[macros[i].macro_number]) != important_macros.end()) {
			vector<int> temp = body_names[macros[i].macro_number];
			temp.push_back(i);
			body_names.erase(macros[i].macro_number);
			body_names.insert(make_pair(macros[i].macro_number, temp));
		}
    }

    cerr << " $$$$$ " << endl;
	
	for(pair<int, vector<int> > p : body_names) {
		cerr << " -----> " << p.first << endl;
		solve(p);
	}
    return 0;
} 

