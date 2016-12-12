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
double RATIO;
string RATIO_S;
struct EarlyLate {
	int body;
	string early;
	string late;
	bool operator < (const EarlyLate &other) const {
		return body < other.body;
	}
};

ofstream fout_first_last;
ofstream fout_local_usage_matrix;
ofstream fout_local_conditional_prob;

set<int> interesting_bodies_for_paper;

set<EarlyLate> early_late_macros;
vector<double> usage_matrix[4][4];
vector<double> conditional_prob[4];

bool solve(const pair<int, vector<int> > &p) {
	int body = p.first;
	vector<int> indecies = p.second;
	
	if(indecies.size() < 100) {
		return false;
	}
	int counter = 0;
	map<string, int> normalized;
	map<int, string> rev_normalized;
	map<int, int> normalized_author;
	int counter_author = 0;
	vector<int> used_early_code, used_late_code;

	EarlyLate el;
	el.body = body;
	el = *(early_late_macros.find(el));
	{
		set<string> temp;
		for(int i = 0; i < (int)indecies.size(); i++) {
			Macro x = macros[indecies[i]];
			if(temp.find(x.name) == temp.end()) {
				normalized[x.name] = counter;
				rev_normalized[counter] = x.name;
				counter++;
			}
			temp.insert(x.name);
			if(i < indecies.size() * RATIO || i >= indecies.size() * (1 - RATIO)) {
				for(int author : x.authors) {
					if(normalized_author.find(author) == normalized_author.end()) {
						normalized_author[author] = counter_author;
						used_early_code.push_back(0);
						used_late_code.push_back(0);
						// 0 means used non
						// 1 means only used early name
						// 2 means only used late name
						// 3 used both
						counter_author++;
					}
				}
			}

		}
	}
	double f[2], rev_f[2];
	memset(f, 0, sizeof f); // First RATIO%
	memset(rev_f, 0, sizeof rev_f); // Last RATIO%

	set<int> unique_authors_per_name[2], unique_authors_all;
	set<int> authors_late_phase, authors_early_phase;
	double frac = RATIO;
	unique_authors_all.clear();
	int bound = indecies.size() * frac;
	for (int i = 0; i < bound; i++) {
		Macro macro = macros[indecies[i]];
		int index = -1;
		if(el.early == macro.name) {
			index = 0;
		}
		if(el.late == macro.name) {
			index = 1;
		}
		for(int author : macro.authors) {
			if(index != -1) {
				unique_authors_per_name[index].insert(author);
				int author_index = normalized_author[author];
				used_early_code[author_index] |= (index + 1);
				authors_early_phase.insert(author);
			}
			unique_authors_all.insert(author);
		}
	}
	for(int i = 0; i < 2; i++) {
		f[i] = unique_authors_per_name[i].size() / (double) unique_authors_all.size();
	}
	frac = 1 - RATIO;
	unique_authors_all.clear();
	for(int i = 0; i < 2; i++) {
		unique_authors_per_name[i].clear();
	}
	bound = indecies.size() * frac;
	for (int i = indecies.size() - 1; i >= bound; i--) {
		Macro macro = macros[indecies[i]];
		int index = -1;
		if(el.early == macro.name) {
			index = 0;
		}
		if(el.late == macro.name) {
			index = 1;
		}
		for(int author : macro.authors) {
			if(index != -1) {
				unique_authors_per_name[index].insert(author);
				int author_index = normalized_author[author];
				used_late_code[author_index] |= (index + 1);
				authors_late_phase.insert(author);
			}
			unique_authors_all.insert(author);
		}
	}
	for(int i = 0; i < 2; i++) {
		rev_f[i] = unique_authors_per_name[i].size() / (double) unique_authors_all.size();
	}
	double local_usage_matrix[4][4];
	memset(local_usage_matrix, 0, sizeof local_usage_matrix);
	for(int i = 0; i < counter_author; i++) {
		local_usage_matrix[used_early_code[i]][used_late_code[i]] += 1.0 / counter_author;
	}
	{
		fout_local_usage_matrix << rev_macro_to_num[body] << endl;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				usage_matrix[i][j].push_back(local_usage_matrix[i][j]);
				fout_local_usage_matrix << local_usage_matrix[i][j] << " ";
			}
			fout_local_usage_matrix << endl;
		}
		fout_local_usage_matrix << endl;
	}
	{
		// The early users have definitely used the early name
		// The late users not necessarily 
		vector<int> mutual_authors_late_and_early;
		for(int author : authors_early_phase) {
			if(authors_late_phase.find(author) != authors_late_phase.end()) {
				mutual_authors_late_and_early.push_back(normalized_author[author]);
			}
		}
		double local_conditional_prob[4];
		memset(local_conditional_prob, 0, sizeof local_conditional_prob);
		for(int local_author : mutual_authors_late_and_early) {
			local_conditional_prob[used_late_code[local_author]] += (1.0 / mutual_authors_late_and_early.size());
		}
		if(mutual_authors_late_and_early.size() > 0) { 
			fout_local_conditional_prob << rev_macro_to_num[body] << endl;
			fout_local_conditional_prob << "conditional authors for late and early: " << mutual_authors_late_and_early.size() << endl;
			for(int i = 0; i < 4; i++) {
				conditional_prob[i].push_back(local_conditional_prob[i]);
				fout_local_conditional_prob << local_conditional_prob[i] << " ";
			}
			fout_local_conditional_prob << endl << endl;
		}
	}

	fout_first_last << "Macro Body and number of total usage: " << rev_macro_to_num[body] << " ---- " << indecies.size() << endl;
	fout_first_last << "Strong starter: " << el.early << " " << f[0] << " ----> " << rev_f[0] << endl; 
	fout_first_last << "Strong finisher: " << el.late << " " << f[1] << "  ----> "<< rev_f[1] << endl;
	fout_first_last << endl;

	return true;
}

vector<double> num[100][2];
vector<string> exchange_point_strings;
void slidingWindow(const pair<int, vector<int> > &p) {
	int body = p.first;
	vector<int> indecies = p.second;
	ofstream fout("RawOutput/ImportantBodies/Sliding_Window_" + to_string(body) + ".txt");
	fout << body << " " << rev_macro_to_num[body] << endl;
	fout << "Size: " << indecies.size() << endl;
	if(indecies.size() < 100) {
		return;
	}
	int counter = 0;
	map<string, int> normalized;
	map<int, string> rev_normalized;
	{
		set<string> temp;
		for(int index : indecies) {
			Macro x = macros[index];
			if(temp.find(x.name) == temp.end()) {
				normalized[x.name] = counter;
				rev_normalized[counter] = x.name;
				counter++;
			}
			temp.insert(x.name);	
		}
	}
	double frac_lower = 0.00, frac_upper = 0.05;
	int index = 0;
	set<int> unique_authors_per_name[2], unique_authors_all;
	EarlyLate el;
	el.body = body;
	el = *(early_late_macros.find(el));

	exchange_point_strings.push_back(rev_macro_to_num[body] + " -- " + el.early + " -- " + el.late);
	while(frac_upper <= 1.002) { 
		unique_authors_per_name[0].clear();
		unique_authors_per_name[1].clear();
		unique_authors_all.clear();
		int lower_bound = frac_lower * indecies.size();
		int upper_bound = frac_upper * indecies.size();
		for(int i = lower_bound; i < upper_bound; i++) {
			Macro macro = macros[indecies[i]];
			if(macro.name == el.early) {
				for(int author : macro.authors) { 
					unique_authors_per_name[0].insert(author);
				}
			}
			if(macro.name == el.late) {
				for(int author : macro.authors) { 
					unique_authors_per_name[1].insert(author);
				}
			}
			for(int author : macro.authors) { 
				unique_authors_all.insert(author);
			}
		}
		fout << frac_lower*100 << " " << frac_upper*100 << " " << unique_authors_per_name[0].size() / (double)unique_authors_all.size() << " " << unique_authors_per_name[1].size() / (double)unique_authors_all.size() << endl; 
		num[index][0].push_back(unique_authors_per_name[0].size() / (double)unique_authors_all.size());
		num[index][1].push_back(unique_authors_per_name[1].size() / (double)unique_authors_all.size());
		frac_lower += 0.01;
		frac_upper += 0.01;
		index++;
	}
}

int main(int argc, char **argv) {
	RATIO = stod(argv[1]);
	RATIO_S = string(argv[1]);
	fout_first_last.open("DominantName/NameAuthorBalanceChanged_" + RATIO_S + ".txt");
	fout_local_usage_matrix.open("DominantName/LocalUsageMatrix_" + RATIO_S + ".txt");
	fout_local_conditional_prob.open("DominantName/LocalConditionalProb_" + RATIO_S + ".txt");


    if(SMART != "_smart" && SMART != "_nosmart") {
        cerr << " THERE IS A PROBLEM WITH THE SMART const variable" << endl;
        return 0;
    }

    Read("All_Arxiv_Macros.txt");
	{
		ifstream fin_early_late("DominantName/EarlyLateMacroNames_" + RATIO_S + ".txt");
		string body, late, early;
		while(getline(fin_early_late, body)) {
			getline(fin_early_late, early);
			getline(fin_early_late, late);
			int index = macro_to_num[body];
			EarlyLate el;
			el.body = index;
			el.early = early;
			el.late = late;
			early_late_macros.insert(el);
		}
		cerr << "Early Late size: " << early_late_macros.size() << endl;
	}
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
		EarlyLate el;
		el.body = macros[i].macro_number;
		if(early_late_macros.find(el) != early_late_macros.end() && (int)macros[i].authors.size() > 0) {
			vector<int> temp = body_names[macros[i].macro_number];
			temp.push_back(i);
			body_names.erase(macros[i].macro_number);
			body_names.insert(make_pair(macros[i].macro_number, temp));
		}
    }

    cerr << " $$$$$ " << endl;

	for(pair<int, vector<int> > p : body_names) {
		int index = p.first;
		EarlyLate el;
		el.body = index;
		if(early_late_macros.find(el) == early_late_macros.end()) { 
			continue;
		}
		cerr << " start solve " <<endl;
		solve(p);
		cerr << " start sliding window" << endl;

		slidingWindow(p);
		cerr << " end " << endl;
	}
	{
			cerr << "print sliding window" << endl;
		ofstream fout_sliding_window("DominantName/SlidingWindow_" + RATIO_S + ".txt");
		ofstream fout_changing_point("DominantName/ExchangePointOfEarlyAndLate_" + RATIO_S + ".txt");
		ofstream fout_exchange_point_samples("DominantName/ExchangePointSamples_" + RATIO_S + ".txt");
		fout_sliding_window << "Start End EarlyName LateName" << endl;
		fout_exchange_point_samples << "ExchangePoint Body Early Late" << endl;
		vector<int> changing_point;
		for(int k = 0; k < (int)num[0][0].size() ; k++) {
			for(int i = 0; i < 95; i++) {
				bool check = true;
				for(int j = 0; j < 10 && i + j <= 95; j++) { 
					if(num[i+j][0][k] > num[i+j][1][k]) {
						check = false;
						break;
					}
				}
				if(check == true) {
					changing_point.push_back(i);
					fout_exchange_point_samples << i << " " << exchange_point_strings[k] << endl;
					break;
				}
			}
		}
		for(int i = 0; i <= 95; i++) {
			if(num[i][0].size() == 0) {
				cerr << " -----> " << i << endl;
				continue;
			}
			sort(num[i][0].begin(), num[i][0].end());
			sort(num[i][1].begin(), num[i][1].end());
			fout_sliding_window << i / 100.0 << " " << (i+5) / 100.0 << " " << num[i][0][num[i][0].size() / 2] << " " << num[i][1][num[i][1].size() / 2]  << endl;
		}
		cerr << "Changing_point.size(): " << changing_point.size() << endl;
		sort(changing_point.begin(), changing_point.end());
		int count = 0;
		for(int i = 0; i < (int)changing_point.size(); i++) {
			count++;
			if(i + 1 == (int)changing_point.size() || changing_point[i] != changing_point[i+1]) {
				fout_changing_point << changing_point[i] << " " << count / (double)changing_point.size() << endl;
				count = 0;
			}
		}
	}
	{
		ofstream fout_matrix("DominantName/UsageMatrix_" + RATIO_S + ".txt");
		cerr << " print usage matrix" << endl;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				sort(usage_matrix[i][j].begin(), usage_matrix[i][j].end());
				fout_matrix << usage_matrix[i][j][usage_matrix[i][j].size() / 2] << " ";
			}
			fout_matrix << endl;
		}
	}
	{
		ofstream fout_conditional_prob("DominantName/ConditionalProb_" + RATIO_S + ".txt");
		fout_conditional_prob << "Median: ";
		for(int i = 0; i < 4; i++) {
			sort(conditional_prob[i].begin(), conditional_prob[i].end());
			fout_conditional_prob << conditional_prob[i][conditional_prob[i].size() / 2] << " ";
		}
		fout_conditional_prob << endl;
		fout_conditional_prob << "Mean: ";
		for(int i = 0; i < 4; i++) {
			double sum = 0;
			for(double x : conditional_prob[i]) {
				sum += x;
			}
			fout_conditional_prob << sum / conditional_prob[i].size() << " ";
		}
		fout_conditional_prob << endl;
	}
    return 0;
} 

