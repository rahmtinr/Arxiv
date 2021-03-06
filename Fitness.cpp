#include"Fitness.h"
using namespace std;

#if K_2K_DEF
ofstream fout_k_2k[20];
ofstream fout_k_2k_thresholds;
vector<OutputElement> k_2k[20];
void prediction_k_2k(int x) {
    if(word_bucket[x].size() == 0) {
        return;
    }
    if(folder == "RawOutput/Name/" && !GoodMacro(word_bucket[x][0], macro_paper_count[x], rev_macro_to_num[x].length())) { // macro needs to be used and should have a length
        return;
    }

    int local_author_count = 1;
    initialize(local_author_count, x);
    set<int> local_unique_authors;
    map<int, int> author_count_to_index;
    map<int, int> author_count_to_median_index;
	map<int, double> author_count_to_average_usage_exp;
	map<int, double> author_count_to_average_adoption_exp;
	{
		int threshold = 5;
		int sum_exp = 0;
		int num_exp = 0;
		int sum_adopt_exp = 0;
		int num_adopt_exp = 0;
		for(int i = 0 ; i <(int) word_bucket[x].size(); i++) {
			if(word_bucket[x][i].authors.size() > 20) {
				// REMOVE THIS IF YOU WANT ALL THE DATA FOR THE PREDICTION <--------------------------------------
				// Only macros that go over k without a paper with 20 authors
				break;
			}
			for(int j = 0; j < (int)word_bucket[x][i].authors.size(); j++) {
				if(local_unique_authors.find(word_bucket[x][i].authors[j]) == local_unique_authors.end()) {
					sum_adopt_exp += word_bucket[x][i].experience[j];
				}
				local_unique_authors.insert(word_bucket[x][i].authors[j]);
				sum_exp += word_bucket[x][i].experience[j];
				num_exp ++;
			}
			num_adopt_exp = local_unique_authors.size();
			while((int)local_unique_authors.size() >= threshold) { // Finds the earliest time we hit threshold unique authors 
				author_count_to_index[threshold] = i + 1;
				author_count_to_average_usage_exp[threshold] = sum_exp / (double) num_exp;
				author_count_to_average_adoption_exp[threshold] = sum_adopt_exp / (double) num_adopt_exp;
				threshold += 5;
			}
		}
	}
	int index_bef = 0;
	for(int out_counter = 0; out_counter < 20; out_counter++) {
		int threshold =  10 * (out_counter+1);

        if(author_count_to_index.find(threshold) == author_count_to_index.end()) {
            break;
        }
        int index = author_count_to_index[threshold];
        vector<int> global, local;
        set<string> unique_names;
        double global_sum = 0, local_sum = 0;
        local_unique_authors.clear();
        for(int i = 0; i < index; i++) {
            unique_names.insert(word_bucket[x][i].name);
            for(int j = 0; j < (int)word_bucket[x][i].authors.size(); j++) {
                int temp = word_bucket[x][i].authors[j];
                if(local_unique_authors.find(temp) == local_unique_authors.end()) {
                    local_unique_authors.insert(temp);
                    global.push_back(word_bucket[x][i].experience[j]);
                    local.push_back(person_pointer[local_author_id[temp]]);
                    global_sum += word_bucket[x][i].experience[j];
                    local_sum += person_pointer[local_author_id[temp]];
                }
            }
        }
		
		
		/*
        int max_author = - 1;
        for(int i = index_bef; i < index; i++) { // creating graph for clustering coeffs
            for(int author1 : word_bucket[x][i].authors) {
                for(int author2 : word_bucket[x][i].authors) {
                    if(author1 == author2) {
                        continue;
                    }
                    adj_list[author1].insert(author2);
                    adj_list[author2].insert(author1);
                    max_author = max(max_author, max(author1, author2));
                }
            }
        }
        index_bef = index;
        pair<double, double> local_global = ClusteringCoeff(max_author + 1);
		*/
		pair<double, double> local_global = make_pair(0, 0);
        sort(global.begin(), global.end());
        sort(local.begin(), local.end());
        OutputElement output_element;
        int eps = 1e-7;
        int special_characters = 0;
        int dollar_signs = 0;
        int back_slashes = 0;
        int depth = 0, max_depth = 0;
        string macro_body = rev_macro_to_num[x];
        for(int i = 0; i < (int)macro_body.length(); i++) {
            if(macro_body[i] == '{' || macro_body[i] == '#') {
                special_characters++;
            }
            if(macro_body[i] == '$') {
                dollar_signs++;
            }
            if(macro_body[i] == '\\') {
                back_slashes++;
            }
            if(macro_body[i] == '{') {
                depth++;
                max_depth = max(depth, max_depth);
            }
            if(macro_body[i] == '}') {
                depth--;
            }
        }

		Macro m0 = word_bucket[x][0];
		Macro m1 = word_bucket[x][author_count_to_index[threshold] - 1];
		int speed = (m1.year - m0.year) * 12 + (m1.month - m0.month);
		m1 = word_bucket[x][author_count_to_index[threshold/2] - 1];
		int speed_half = (m1.year - m0.year) * 12 + (m1.month - m0.month);

		double avg_using_exp = author_count_to_average_usage_exp[threshold];
		double avg_adopt_exp = author_count_to_average_adoption_exp[threshold];
		double avg_using_exp_half = author_count_to_average_usage_exp[threshold / 2];
		double avg_adopt_exp_half = author_count_to_average_adoption_exp[threshold / 2];

        output_element.output_string = to_string(word_bucket[x][0].authors.size()) + ", " + to_string(index) + ", " + to_string(global[global.size() / 2]) + ", " + to_string(global_sum / global.size()) + ", ";
        output_element.output_string += to_string(local_global.first + eps) + ", " + to_string(local_global.second + eps) + ", ";
        output_element.output_string += to_string(rev_macro_to_num[x].length()) + ", " + to_string(special_characters) + ", " + to_string(unique_names.size()) + ", ";
        output_element.output_string += to_string(dollar_signs) + ", " + to_string(back_slashes) + ", " + to_string(max_depth) + ", ";
		output_element.output_string += to_string(speed_half) + ", " + to_string(speed) + ", ";
		output_element.output_string += to_string(avg_using_exp) + ", " + to_string(avg_using_exp_half) + ", ";
		output_element.output_string += to_string(avg_adopt_exp) + ", " + to_string(avg_adopt_exp_half) + ", ";
        output_element.authors_count = local_author_count;
        k_2k[out_counter].push_back(output_element);
    }
}
#endif

#if LONGEVITY_DEF
ofstream fout_author_fitness[11];
void AuthorFitness() {
    int window = 5;
    for(int i = 1; i <= 10; i++) {
        fout_author_fitness[i].open(folder + "author_fitness_" + to_string(i * window)  + ".txt");
        fout_author_fitness[i] << "PapersRevealedCount, AllMacroUsage, UniqueMacroUsage, ";
        fout_author_fitness[i] << "CoAuthorCount, ChangeFraction, ChangeFractionHighUsage, FinalGlobalExp" << endl;

    }
    ofstream fout_35_dies_before_40_or_after_50("RawOutput/author_35_dies_before_40_or_reaches_50.txt");
    fout_35_dies_before_40_or_after_50 << "PapersRevealedCount, AllMacroUsage, UniqueMacroUsage, ";
    fout_35_dies_before_40_or_after_50 << "CoAuthorCount, ChangeFraction, Label" << endl;

    map<int, int> author_threshold;
    map<int, int> all_macro_usage; // value =  number of macros key used
    map<int, int> unique_macro_usage; // value = number of unique macros(body) key used
    map<pair<int, int>, int> author_macro_pair_count; // value =  how many times author(key.first) used the macro(key.second)
    map<int, vector<int> > author_macros; // value = a vector of macro numbers that key used
    set<pair<int, int> > co_authors; // who were co_authors
    map<int ,int> co_author_count; // value = how many unique co-authors key had
    map<pair<int, int>, string> author_macro_name; // value = the last name that author(key.first) used for macro(key.second)
    map<int, int> author_change_macro_name; // value = how many time author used a macro
    map<pair<int, int>, int> author_macro_pair_change_count; // value = how many times the author(key.first) changed the name for macro(key.second)
	
    for(int i = 0; i < (int) macros.size(); i++) {
        if(i % 100000 == 0 ) {
            cerr <<"Longevity " <<  i / 100000 << endl;
        }
        if(GoodMacro(macros[i], word_bucket[macros[i].macro_number].size(), rev_macro_to_num[macros[i].macro_number].length())) {
            for(int j = 0; j < (int)macros[i].authors.size(); j++) {
                int author1 = macros[i].authors[j];
                all_macro_usage[author1]++;
                author_macro_pair_count[make_pair(author1, macros[i].macro_number)]++;
                for(int k = j + 1; k < (int)macros[i].authors.size(); k++) {
                    int author2 = macros[i].authors[k];
                    if(co_authors.find(make_pair(author1, author2)) == co_authors.end()) {
                        co_authors.insert(make_pair(author1, author2));
                        co_authors.insert(make_pair(author2, author1));
                        co_author_count[author1]++;
                        co_author_count[author2]++;
                    }
                }
                if(author_macro_name.find(make_pair(author1, macros[i].macro_number)) == author_macro_name.end()) {
                    unique_macro_usage[author1]++;
                    author_macro_name[make_pair(author1, macros[i].macro_number)] = macros[i].name;
                    vector<int> temp = author_macros[author1];
                    temp.push_back(macros[i].macro_number);
                    author_macros[author1] = temp;
                } else {
                    if(author_macro_name[make_pair(author1,macros[i].macro_number)] != macros[i].name) {
                        author_change_macro_name[author1]++;
                        author_macro_pair_change_count[make_pair(author1, macros[i].macro_number)]++;
                    }
                }
            }
        }
        if((i == (int)macros.size() - 1 || ExactSame(macros[i], macros[i + 1]) == false) && macros[i].authors.size() < 20) {
            for(int j = 0; j < (int) macros[i].authors.size(); j++) {
                int author1 = macros[i].authors[j];
                if(author_threshold.find(author1) == author_threshold.end()) {
                    author_threshold[author1] = window;
                }
                int change_count = 0;
                int total_count = 0;
                if(author_threshold[author1] == macros[i].experience[j] && author_threshold[author1] < window * 10 + 1) {
                    for(auto kv : author_macros) { // kv.first is the author, kv.second is a vector of macro numbers used
                        for(int macro_id : kv.second) {
                            if(author_macro_pair_count[make_pair(author1, macro_id)] >= macros[i].experience[j] / 2) {
                                change_count += author_macro_pair_change_count[make_pair(author1, macro_id)];
                                total_count  += author_macro_pair_count[make_pair(author1, macro_id)] - 1;
                            }
                        }
                    }
                    double changed = author_change_macro_name[author1]
                        / (double)(all_macro_usage[author1] - unique_macro_usage[author1]);
                    double changed_high_usage = change_count / (double)total_count;
                    fout_author_fitness[author_threshold[author1] / window] << author_threshold[author1] << ", " << all_macro_usage[author1] << ", ";
                    fout_author_fitness[author_threshold[author1] / window] << unique_macro_usage[author1] << ", ";
                    fout_author_fitness[author_threshold[author1] / window] << co_author_count[author1] << ", ";
                    fout_author_fitness[author_threshold[author1] / window] << changed << ", " << changed_high_usage << ", " << author_experience[author1] << endl;

                    author_threshold[author1] = author_threshold[author1] + window;
                }
            }
        }
    }
}
#endif
