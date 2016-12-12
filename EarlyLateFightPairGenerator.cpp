#include<iostream>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
#include<string>
#include"HeapsLaw.h"
#include"Core.h"
#include"Macro.h"
#include"Graph.h"
#include"Fitness.h"
#include"Reader.h"

using namespace std;

map<int, vector<int>> body_names;
set<int> interesting_bodies_for_paper;

struct EarlyLate {
	int body;
	string early;
	string late;
	bool operator < (const EarlyLate &other) const {
		return body < other.body;
	}
};

struct Tuple { 
	double usage_ratio1, usage_ratio2;
	int macro_number;
	string name1;
	string name2;
	bool operator < (const Tuple &other)const {
		return usage_ratio1 + usage_ratio2 < other.usage_ratio1 + other.usage_ratio2;
	}
	string ToString() {
		return to_string(usage_ratio1) + " " + to_string(usage_ratio2) + " " + rev_macro_to_num[macro_number] + " " + name1 + " " + name2;
	}
};

struct FeatureTuple {
	double avg_exp1, avg_exp2;
	int len1, len2;
	int non_alpha1, non_alpha2; // non alphabetic chars
	int uppercase1, uppercase2;
	int author_count1, author_count2;
	double avg_adoption_exp1, avg_adoption_exp2;
	vector<double> avg_exp_window[2];
	vector<double> histogram[2];
	vector<double> avg_adoption_exp_window[2];
	int name1_won, name2_won;

	int started_from_late, converted_from_early, converted_from_other;

	string ToString() {
		string ret;
		ret = to_string(avg_exp1) + " " + to_string(len1) + " " + to_string(non_alpha1) + " " + to_string(uppercase1) + " " + to_string(author_count1) + " " + to_string(avg_adoption_exp1) + " ";
		for(int i = 0; i < 6; i++) {
			ret += to_string(avg_exp_window[0][i]) + " ";
		}
		for(int i = 0; i < 6; i++) {
			ret += to_string(avg_adoption_exp_window[0][i]) + " ";
		}
		ret += to_string(name1_won) + " ";
		ret += to_string(avg_exp2) + " " + to_string(len2) + " " + to_string(non_alpha2) + " " + to_string(uppercase2) + " " + to_string(author_count2) + " " + to_string(avg_adoption_exp2) + " ";
		for(int i = 0; i < 6; i++) {
			ret += to_string(avg_exp_window[1][i]) + " ";
		}
		for(int i = 0; i < 6; i++) {
			ret += to_string(avg_adoption_exp_window[1][i]) + " ";
		}
		ret += to_string(name2_won) + " ";
		return ret;
	}
};

set<EarlyLate> early_late_macros;

vector<Tuple> s1_tuples; // all pairs that strong early loses to weak early
set<pair<Tuple, Tuple> > head_to_head_fight;

void find_EN_LN_pairs(const pair<int, vector<int> > &p) {
	int body = p.first;
	vector<int> indecies = p.second;

	if(indecies.size() < 100) {
		return;
	}
	int counter = 0;
	map<string, int> normalized;
	map<int, string> rev_normalized;
	map<int, int> normalized_author;
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
		}
	}
	double f[2], rev_f[2];
	memset(f, 0, sizeof f); // First 30%
	memset(rev_f, 0, sizeof rev_f); // Last 30%
	set<int> unique_authors_per_name[2], unique_authors_all;
	set<int> authors_late_phase, authors_early_phase;
	double frac = 0.3;
	unique_authors_all.clear();
	int bound = indecies.size() * frac;
	for (int i = 0; i < bound; i++) {
		int index = -1;
		if(el.early == macros[indecies[i]].name) {
			index = 0;
		}
		if(el.late == macros[indecies[i]].name) {
			index = 1;
		}
		for(int author : macros[indecies[i]].authors) {
			if(index != -1) {
				unique_authors_per_name[index].insert(author);
				authors_early_phase.insert(author);
			}
			unique_authors_all.insert(author);
		}
	}
	for(int i = 0; i < 2; i++) {
		f[i] = unique_authors_per_name[i].size() / (double) unique_authors_all.size();
	}
	frac = 0.7;
	unique_authors_all.clear();
	for(int i = 0; i < 2; i++) {
		unique_authors_per_name[i].clear();
	}
	bound = indecies.size() * frac;
	for (int i = indecies.size() - 1; i >= bound; i--) {
		int index = -1;
		if(el.early == macros[indecies[i]].name) {
			index = 0;
		}
		if(el.late == macros[indecies[i]].name) {
			index = 1;
		}
		for(int author : macros[indecies[i]].authors) {
			if(index != -1) {
				unique_authors_per_name[index].insert(author);
				authors_late_phase.insert(author);
			}
			unique_authors_all.insert(author);
		}
	}
	for(int i = 0; i < 2; i++) {
		rev_f[i] = unique_authors_per_name[i].size() / (double) unique_authors_all.size();
	}
	if(f[0] >= 0.3) {
		Tuple tuple;
		tuple.usage_ratio1 = f[0];
		tuple.usage_ratio2 = f[1];
		tuple.macro_number = body;
		tuple.name1 = el.early;
		tuple.name2 = el.late;
		s1_tuples.push_back(tuple);
	}
}

void match(const pair<int, vector<int> > &p) {
	int body = p.first;
	vector<int> indecies = p.second;

	if(indecies.size() < 100) {
		return;
	}
	int counter = 0;
	int different_names;
	map<string, int> normalized;
	map<int, string> rev_normalized;
	map<int, int> normalized_author;
	vector<int> used_early_code, used_late_code;

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
		}
		different_names = temp.size();
	}
	if(different_names > 3000) { 
		return;
	}
	double *f, *rev_f;
	f = new double[different_names];
	rev_f = new double[different_names];
	for(int j = 0; j < different_names; j++) {
		f[j] = rev_f[j] = 0;
	}
	set<int> unique_authors_per_name[3010], unique_authors_all;
	set<int> authors_late_phase, authors_early_phase;
	double frac = 0.3;
	unique_authors_all.clear();
	int bound = indecies.size() * frac;
	for (int i = 0; i < bound; i++) {
		int index = normalized[macros[indecies[i]].name];
		for(int author : macros[indecies[i]].authors) {
			unique_authors_per_name[index].insert(author);
			authors_early_phase.insert(author);
			unique_authors_all.insert(author);
		}
	}
	int max_index_early = 0, max_index_late = 0;
	for(int j = 0; j < different_names; j++) {
		f[j] = unique_authors_per_name[j].size() / (double) unique_authors_all.size();
		if(f[j] > f[max_index_early]) {
			max_index_early = j;
		}
	}
	frac = 0.7;
	unique_authors_all.clear();
	for(int j = 0; j < different_names; j++) {
		unique_authors_per_name[j].clear();
	}
	bound = indecies.size() * frac;
	for (int i = indecies.size() - 1; i >= bound; i--) {
		int index = normalized[macros[indecies[i]].name];
		for(int author : macros[indecies[i]].authors) {
			unique_authors_per_name[index].insert(author);
			authors_late_phase.insert(author);
			unique_authors_all.insert(author);
		}
	}

	for(int j = 0; j < different_names; j++) {
		rev_f[j] = unique_authors_per_name[j].size() / (double) unique_authors_all.size();
		if(f[j] > f[max_index_late]) {
			max_index_late = j;
		}
	}
	if(max_index_late != max_index_early) { 
		return;
	}
	int winner = max_index_early;
	for(int j = 0; j < different_names; j++) {
		if(j == winner) {
			continue;
		}
		bool found_match = false;
		Tuple tuple, tuple2;
		tuple.usage_ratio1 = f[max_index_early];
		tuple.usage_ratio2 = f[j];
		int vacinity = lower_bound(s1_tuples.begin(), s1_tuples.end(), tuple) - s1_tuples.begin();
		for(int i = max(vacinity - 15, 0); i < min(vacinity + 15, (int)s1_tuples.size()); i++) {
			double ratio = body_names[s1_tuples[i].macro_number].size() / (double) body_names[body].size();
			cerr << "ratio:" << ratio << endl;
			if(ratio > 1) {
				ratio = 1 / ratio;
			}
		
			if(abs(tuple.usage_ratio1 - s1_tuples[i].usage_ratio1) < 0.02 && abs(tuple.usage_ratio2 - s1_tuples[i].usage_ratio2) < 0.02) {
				cerr << " got in!" << endl;
				if (ratio > (1/1.1)) {
					tuple2 = s1_tuples[i];
					found_match = true;
					tuple.macro_number = body;
					tuple.name1 = rev_normalized[winner];
					tuple.name2 = rev_normalized[j];
					head_to_head_fight.insert(make_pair(tuple2, tuple)); // tuple2 has the EN in usage_ratio1 and LN in usage_ratio2, tuple has ELN in usage_ratio1 and a random name in usage_ratio2
					s1_tuples.erase(s1_tuples.begin() + i);
					cerr << " ha ha " << endl;
					break;
				}
				cerr << "ratio didn't let me get the match" << endl;
			}
		}
		if(found_match == true) {
			return;
		}
	}
}

FeatureTuple FeatureExtraction(Tuple t, int change_over_happened) {
	int body = t.macro_number;
	vector<int> indecies = body_names[body];
	set<int> authors1, authors2;
	int sum_experience1 = 0, sum_experience2 = 0;
	int sum_adoption_experience1 = 0, sum_adoption_experience2 = 0;
	int num_experience1 = 0, num_experience2 = 0;
	double f[2];
	double rev_f[2];
	f[0] = f[1] = 0;
	{
		set<int> name1_authors, name2_authors;
		set<int> all_authors;
		for(int i = 0; i < (int)indecies.size(); i++) {
			int after = -1;
			if(i < 0.3 * indecies.size()) {
				after = 0;
			}
			if(i > 0.7 * indecies.size()) {
				after = 1;
			}
			if(after == -1 && (i - 1) < 0.3 * indecies.size()) {
				f[0] = name1_authors.size() / (double) all_authors.size();
				f[1] = name2_authors.size() / (double) all_authors.size();
				all_authors.clear();
				name1_authors.clear();
				name2_authors.clear();
			}
			if(after == - 1) {
				continue;
			}
			for(int author : macros[indecies[i]].authors) {
				all_authors.insert(author);
				if(macros[indecies[i]].name == t.name1) {
					name1_authors.insert(author);
				}
				if(macros[indecies[i]].name == t.name2) {
					name2_authors.insert(author);
				}
			}
		}
		rev_f[0] = name1_authors.size() / (double) all_authors.size();
		rev_f[1] = name2_authors.size() / (double) all_authors.size();
	}
	FeatureTuple ft; 
	{
//	Find average using and adoption exp
		set<int> authors[2];
		for(int step_count = 0; step_count < 20; step_count++) {
			int sum_exp1 = 0, sum_exp2 = 0, num_exp1 = 0, num_exp2 = 0;
			int sum_adopt_exp1 = 0, sum_adopt_exp2 = 0, num_adopt_exp1 = 0, num_adopt_exp2 = 0;
			int prev_threshold = 0.05 * (step_count) * indecies.size();
			int next_threshold = 0.05 * (step_count + 1) * indecies.size();

			int author, exp;

			for(int i = prev_threshold; i < next_threshold; i++) {
				if(macros[indecies[i]].name == t.name1) {
					for(int k = 0; k < (int)macros[indecies[i]].experience.size(); k++) {
						author = macros[indecies[i]].authors[k];
						exp = macros[indecies[i]].experience[k];
						if(authors[0].find(author) == authors[0].end()) {
							authors[0].insert(author);
							sum_adopt_exp1 += exp;
							num_adopt_exp1++;
						}
						sum_exp1 += exp;
						num_exp1++;
					}
				}
				if(macros[indecies[i]].name == t.name2) {
					for(int k = 0; k < (int)macros[indecies[i]].experience.size(); k++) {
						author = macros[indecies[i]].authors[k];
						exp = macros[indecies[i]].experience[k];
						if(authors[1].find(author) == authors[1].end()) {
							authors[1].insert(author);
							sum_adopt_exp2 += exp;
							num_adopt_exp2++;
						}
						sum_exp2 += exp;
						num_exp2++;
					}
				}

			}
			ft.avg_exp_window[0].push_back(sum_exp1 / (double) max(1, num_exp1));
			ft.avg_exp_window[1].push_back(sum_exp2 / (double) max(1, num_exp2));

			ft.avg_adoption_exp_window[0].push_back(sum_adopt_exp1 / (double) max(1, num_adopt_exp1));
			ft.avg_adoption_exp_window[1].push_back(sum_adopt_exp2 / (double) max(1, num_adopt_exp2));

		}
	}

	{ // fights before 30%
		int early_name_won = 0;
		int late_name_won = 0;
		int bound = indecies.size() * 0.3;
		map<int, int> author_lastname_used;
		for(int i = 0; i < bound; i++) {
			Macro macro = macros[indecies[i]];
			int late_index = -1, early_index = -1;
			for(int j = 0; j < (int)macro.authors.size(); j++) {
				if(author_lastname_used.find(macro.authors[j]) == author_lastname_used.end()) {
					continue;
				}
				if(macros[author_lastname_used[macro.authors[j]]].name == t.name1) {
					early_index = j;
				}
				if(macros[author_lastname_used[macro.authors[j]]].name == t.name2){
					late_index = j;
				}
			}
			int is_current_late = -1;
			if(macro.name == t.name1) {
				is_current_late = 0;
			} else if(macro.name == t.name2) {
				is_current_late = 1;
			}
			if(late_index == -1 || early_index == -1 || is_current_late == -1) {
				// either at least one ofthe names weren't used in the prev papers of authors or this paper is not one of those names
			} else { 
				// fight happened and we can find which name won.
				if(change_over_happened==1){
					cerr << "FIGHT: " << body << " " << rev_macro_to_num[body] << " " << (is_current_late==1 ? "Late":"Early") <<  " EN: " <<  t.name1 << "     LN: " << t.name2 << " " ;
					cerr << (change_over_happened==1 ? "is Change Over":"not change over") << endl;
					cerr << "FightPaper: " << macro.paper_id << endl;
					cerr << early_index << "AuthorPrevName: " << macros[author_lastname_used[macro.authors[early_index]]].paper_id << " " <<  macros[author_lastname_used[macro.authors[early_index]]].name << endl; 
					cerr << late_index  << "AuthorPrevName: " << macros[author_lastname_used[macro.authors[late_index]]].paper_id << " " <<   macros[author_lastname_used[macro.authors[late_index]]].name << endl; 
					cerr << "Winner is: " << macro.name << endl;

					cerr << "______________________________________________" << endl;
/*
					if(interesting_bodies_for_paper.find(body) != interesting_bodies_for_paper.end()) {
						interesting_bodies_for_paper.erase(body);
						ofstream fout("RawOutput/ImportantBodies/"+ to_string(body) + ".txt");
						fout << "Body: " << rev_macro_to_num[body] << endl;
						fout << "PaperCount: " << indecies.size() << endl;
						for(int i = 0; i < (int)indecies.size(); i++) {
							fout << i << ": " <<  "Id: " << macros[indecies[i]].paper_id << " NameUsed: " << macros[indecies[i]].name << " AuthorCount: " << macros[indecies[i]].authors.size()  << endl;
						}
					}
*/
				}
				if(is_current_late == 1) {
					late_name_won++;
				} else{ 
					early_name_won++;
				}
			}
			if(is_current_late > -1) {
				for(int author : macro.authors) {
					author_lastname_used[author] = indecies[i];
				}
			}
		}
		ft.name1_won = early_name_won;
		ft.name2_won = late_name_won;
	}

	{ // switch overs in first  100% // 30%
		map<int, string> author_lastname_used;
		int started_from_late = 0;
		int converted_from_early = 0;
		int converted_from_other = 0;
		int bound = indecies.size(); // * 0.3;
		for(int i = 0; i < bound; i++) {
			Macro macro = macros[indecies[i]];
			for(int j = 0; j < (int)macro.authors.size(); j++) {
				if(author_lastname_used.find(macro.authors[j]) == author_lastname_used.end()) {
					if(macro.name == t.name2) {
						started_from_late++;
					}
					author_lastname_used[macro.authors[j]] = macro.name;
				} else {
					if(author_lastname_used[macro.authors[j]] == t.name2) {
						continue;
					}
					if(author_lastname_used[macro.authors[j]] == t.name1){
						if(macro.name == t.name2) {
							converted_from_early++;
							author_lastname_used[macro.authors[j]] = macro.name;
						}
					} else {
						if(macro.name == t.name2) {
							converted_from_other++;
						}
						author_lastname_used[macro.authors[j]] = macro.name;
					}
				}
			}
		}
		ft.started_from_late = started_from_late;
		ft.converted_from_early = converted_from_early;
		ft.converted_from_other = converted_from_other;
	}

	{
		// bullshit histogram
		for(int step_count = 0; step_count < 3; step_count++) {
			int sum_exp1 = 0, sum_exp2 = 0, num_exp1 = 0, num_exp2 = 0;
			int prev_threshold = 0.1 * (step_count) * indecies.size();
			int next_threshold = 0.1 * (step_count + 1) * indecies.size();
			for(int i = prev_threshold; i < next_threshold; i++) {
				if(macros[indecies[i]].name == t.name1) {
					for(int exp : macros[indecies[i]].experience) {
						sum_exp1 += exp;
						num_exp1++;
					}
				}
				if(macros[indecies[i]].name == t.name2) {
					for(int exp : macros[indecies[i]].experience) {
						sum_exp2 += exp;
						num_exp2++;
					}
				}

			}
			ft.histogram[0].push_back(sum_exp1 / (double) max(1, num_exp1));
			ft.histogram[1].push_back(sum_exp2 / (double) max(1, num_exp2));

		}
	}
	if(abs(rev_f[1]-rev_f[0]) > 0.2) {
		// there is a clear winner get the average experience for the first 30%
		int bound = 0.3 * indecies.size();
		for(int i = 0; i < bound; i++) {
			if(macros[indecies[i]].name == t.name1) {
				for(int k = 0; k < (int)macros[indecies[i]].authors.size(); k++) {
					int author = macros[indecies[i]].authors[k];
					if(authors1.find(author) == authors1.end()) {
						sum_adoption_experience1 += macros[indecies[i]].experience[k];
					}
					authors1.insert(author);
					sum_experience1 += macros[indecies[i]].experience[k]; 
					num_experience1++;
				}
			} else if(macros[indecies[i]].name == t.name2) {
				for(int k = 0; k < (int)macros[indecies[i]].authors.size(); k++) {
					int author = macros[indecies[i]].authors[k];
					if(authors2.find(author) == authors2.end()) {
						sum_adoption_experience2 += macros[indecies[i]].experience[k];
					}
					authors2.insert(author);
					sum_experience2 += macros[indecies[i]].experience[k]; 
					num_experience2++;
				}
			}
		}
		// get name features
		int non_alpha1 = 0, uppercase1 = 0, non_alpha2 = 0, uppercase2 = 0;
		for(int i = 0; i < (int)t.name1.size(); i++) {
			if(t.name1[i] <= 'Z' && t.name1[i] >= 'A') {
				uppercase1++;
			}
			if(isalpha(t.name1[i]) == 0) {
				non_alpha1++;
			}
		}
		for(int i = 0; i < (int)t.name2.size(); i++) {
			if(t.name2[i] <= 'Z' && t.name2[i] >= 'A') {
				uppercase2++;
			}
			if(isalpha(t.name2[i]) == 0) {
				non_alpha2++;
			}
		}
		if(authors2.size() == 0 || authors1.size() == 0) { 
			ft.len1 = -1;
		} else {
			ft.avg_exp1 = sum_experience1 / (double) num_experience1;
			ft.len1 = t.name1.length();
			ft.author_count1 = num_experience1;
			ft.avg_adoption_exp1 = sum_adoption_experience1 / (double) authors1.size();
			ft.uppercase1 = uppercase1;
			ft.non_alpha1 = non_alpha1;
			ft.avg_exp2 = sum_experience2 / (double) num_experience2;
			ft.len2 = t.name2.length();
			ft.author_count2 = num_experience2;
			ft.avg_adoption_exp2 = sum_adoption_experience2 / (double) authors2.size();
			ft.uppercase2 = uppercase2;
			ft.non_alpha2 = non_alpha2;
		}
	} else {
		ft.len1 = -1; // check in main to not print the pair
	}
	return ft;
}

int main() {
	folder = "RawOutput/Name/";
	if(SMART != "_smart" && SMART != "_nosmart") {
		cerr << " THERE IS A PROBLEM WITH THE SMART const variable" << endl;
		return 0;
	}

	Read("All_Arxiv_Macros.txt");
	{
		ifstream fin_early_late("DominantName/EarlyLateMacroNames_0.3.txt");
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
		/*
		EarlyLate el;
		el.body = macros[i].macro_number;
		*/
		if((int)macros[i].authors.size() > 0) {
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
		find_EN_LN_pairs(p);
	}
	cerr << "started sorting" << endl;
	sort(s1_tuples.begin(), s1_tuples.end());
	cerr << "sorted" << endl;
	for(pair<int, vector<int> > p : body_names) { 
		int index = p.first;
		EarlyLate el;
		el.body = index;
		if(early_late_macros.find(el) != early_late_macros.end()) { 
			continue;
		}
		cerr << "Match: " << index << endl;
		match(p);
	}
	cerr << "------> done matching" << endl;
	cerr << "size of matching: " << head_to_head_fight.size() << endl;	
	
	{
		/*
		interesting_bodies_for_paper.insert(86507);
		interesting_bodies_for_paper.insert(3270);
		interesting_bodies_for_paper.insert(154157);
		interesting_bodies_for_paper.insert(85966);
		interesting_bodies_for_paper.insert(10317);
		interesting_bodies_for_paper.insert(44039);
		*/

		double sum[4][20], sum2[4][20];
		double sum_adopt[4][20];
		memset(sum, 0, sizeof sum);
		memset(sum2, 0, sizeof sum2);
		memset(sum_adopt, 0, sizeof sum_adopt);

		ofstream fout_head_to_head_fights("DominantName/HeadToHeadFights_DenomBalanced.txt");
		ofstream fout_avg_exp_window("DominantName/AvgExpWindow.txt");
		ofstream fout_avg_exp_window_diff_first("DominantName/AvgExpWindow_DiffFirst.txt");
		ofstream fout_avg_exp_window_derivitive("DominantName/AvgExpWindow_derivitive.txt");
		ofstream fout_avg_exp_histogram("DominantName/AvgExpHistogram.txt");

		ofstream fout_avg_adopt_exp_window("DominantName/AvgAdoptExpWindow.txt");

		ofstream fout_convert_percentages("DominantName/ConvertToLNPercenteages.txt");

		fout_head_to_head_fights << "AvgExp1 Len1 NonAlphabetic1 Uppercase1 AuthorCount1 AvgAdoptionExp2 AvgExp1_0.05 AvgExp1_0.10 AvgExp1_0.15 AvgExp1_0.20 AvgExp1_0.25 AvgExp1_0.30 ";
		fout_head_to_head_fights << "AvgAdoptExp1_0.05 AvgAdoptExp1_0.10 AvgAdoptExp1_0.15 AvgAdoptExp1_0.20 AvgAdoptExp1_0.25 AvgAdoptExp1_0.30 FightsWon1 ";
		fout_head_to_head_fights << "AvgExp2 Len2 NonAlphabetic2 Uppercase2 AuthorCount2 AvgAdoptionExp2 AvgExp2_0.05 AvgExp2_0.10 AvgExp2_0.15 AvgExp2_0.20 AvgExp2_0.25 AvgExp2_0.30 ";
		fout_head_to_head_fights << "AvgAdoptExp2_0.05 AvgAdoptExp2_0.10 AvgAdoptExp2_0.15 AvgAdoptExp2_0.20 AvgAdoptExp2_0.25 AvgAdoptExp2_0.30 FightsWon2 ";
		fout_head_to_head_fights << "Label" <<endl;

		fout_avg_exp_window << "0.05Window EN EN_err LN LN_err ELN ELN_err ON ON_err" << endl;
		fout_avg_exp_window_diff_first << "0.05Window EN LN ELN ON" << endl;
		fout_avg_exp_window_derivitive << "0.05Window DerivitiveEN DerivitiveLN DerivitiveELN DerivitiveON" << endl;
		fout_avg_exp_histogram << "0.1Window EN LN ELN ON" << endl;
		
		fout_avg_adopt_exp_window << "0.05Window EN EN_err LN LN_err ELN ELN_err ON ON_err" << endl;

		fout_convert_percentages << "ChangeOverHappened FirstNameIsLN ConvertedFromEN ConvertedFromOther MacroBody" << endl;

		int num_final_matches = 0;
		vector<double> values[4][20], values_adopt[4][20];
		for(pair<Tuple, Tuple> p : head_to_head_fight) {
			FeatureTuple ft1, ft2;
			ft1 = FeatureExtraction(p.first, 1);
			ft2 = FeatureExtraction(p.second, 0);
			if(ft1.len1 != -1 && ft2.len1 != -1) {
				num_final_matches++;
				for(int i = 0; i < 2; i++) {
					for(int j = 0; j < 20; j++) {
						sum[i][j] += ft1.avg_exp_window[i][j];
						sum[i+2][j] += ft2.avg_exp_window[i][j];
						values[i][j].push_back(ft1.avg_exp_window[i][j]);
						values[i+2][j].push_back(ft2.avg_exp_window[i][j]);

						sum_adopt[i][j] += ft1.avg_adoption_exp_window[i][j];
						sum_adopt[i+2][j] += ft2.avg_adoption_exp_window[i][j];
						values_adopt[i][j].push_back(ft1.avg_adoption_exp_window[i][j]);
						values_adopt[i+2][j].push_back(ft2.avg_adoption_exp_window[i][j]);
					}
					for(int j = 0; j < 3; j++) {
						sum2[i][j] += ft1.histogram[i][j];
						sum2[i+2][j] += ft2.histogram[i][j];
					}
				}
				fout_head_to_head_fights << ft1.ToString() << " " << 1 << endl; // second name takes over
				fout_head_to_head_fights << ft2.ToString() << " " << 0 << endl; // first name takes over

				fout_convert_percentages << 1 << " " << ft1.started_from_late << " " << ft1.converted_from_early << " " << ft1.converted_from_other << " " << rev_macro_to_num[p.first.macro_number] << endl;	
				fout_convert_percentages << 0 << " " << ft2.started_from_late << " " << ft2.converted_from_early << " " << ft2.converted_from_other << " " << rev_macro_to_num[p.second.macro_number] << endl;	
			}
		}
		int j_bound = 20;
		for(int j = 0; j < j_bound; j++) {
			fout_avg_exp_window << j*0.05 << " ";
			fout_avg_exp_window_diff_first << j*0.05 << " ";
			fout_avg_adopt_exp_window << j*0.05 << " ";
			if(j < j_bound - 1) {
				fout_avg_exp_window_derivitive << (j+1)*0.05 << " ";
			}
			if(j < 3) {
				fout_avg_exp_histogram << j * 0.1 << " ";
			}
			for(int i = 0; i < 4; i++) {
				double avg = sum[i][j] / num_final_matches, avg_adopt = sum_adopt[i][j] / num_final_matches;
				double variance = 0, variance_adopt = 0;
				for(double k : values[i][j]) {
					variance += (k - avg) * (k - avg);
				}
				for(double k : values_adopt[i][j]) {
					variance_adopt += (k - avg_adopt) * (k - avg_adopt);
				}
				double error_bar = sqrt(variance) / values[i][j].size();
				double error_bar_adopt = sqrt(variance_adopt) / values_adopt[i][j].size();
				fout_avg_exp_window << avg << " " << error_bar << " ";
				fout_avg_exp_window_diff_first << sum[i][j] / num_final_matches - sum[i][0] / num_final_matches << " ";
				fout_avg_adopt_exp_window << sum_adopt[i][j] / num_final_matches << " " << error_bar_adopt << " ";
				if(j < j_bound - 1) {
					fout_avg_exp_window_derivitive << sum[i][j+1] / num_final_matches - sum[i][j] / num_final_matches << " ";
				}
				if(j < 3) {
					fout_avg_exp_histogram << sum2[i][j] / num_final_matches << " ";
				}
			}
			fout_avg_exp_window << endl;
			fout_avg_exp_window_diff_first << endl;
			fout_avg_adopt_exp_window << endl;
			if(j < j_bound - 1) {
				fout_avg_exp_window_derivitive << endl;
			}
			if(j < 3) {
				fout_avg_exp_histogram << endl;
			}
		}
	}
	return 0;
} 

