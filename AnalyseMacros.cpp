#include<iostream>
#include<cstring>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
#include<cstdlib>


#define JACCARD false
#define NAME_PRINTER false
#define ONION_SHAPE_TREE false
#define VOLUME false

using namespace std;

const int Macro_length_filter = 20;
const int Macro_paper_usage = 30;

map<string, int> macro_to_num;
map<int, string> rev_macro_to_num;
map<string, int> author_to_num;
map<int, string> rev_author_to_num;
const int H = 1000 * 1000;
vector<int> indecies_contain_word;
map<int, int> local_author_id;
vector<int> rev_local_author_id(H);
vector<int> rev_comp(H);
int local_counter = 0;
vector<vector<int> > graph(H), rev_graph(H), graph_edge_index(H);
vector<int> local_earliest(H);
vector<int> local_earliest_index(H);
vector<int> fin_time;
vector<int> local_mark(H);
vector<vector<int> > comp(H);
int biggest_interval;
vector<int> width[15][15];
vector<int> paper_depth_count[15][15];
set<int> local_paper_depth_set;

vector<int> interesting_macros;
set<int> interesting_macros2; // macros that have depth more than 4
map<int, string> root_author;

bool has_skipped = false;
string skipped_string = "";

ofstream feynman_fout("RawOutput/Feynman_summary" + skipped_string + ".txt");
//ofstream fout_frac_length("RawOutput/ONLYBIG-Biggest_component_MacroLength" + skipped_string + ".txt");
ofstream fout_frac_length("RawOutput/Biggest_component_MacroLength" + skipped_string + ".txt");
ofstream fout_biggest_second_biggest("RawOutput/Biggest_SecondBiggest" + skipped_string + ".txt");
ofstream fout_heat_map("RawOutput/MacroLength_AllAuthors_Biggest" + skipped_string + ".txt");
ofstream fout_stats("RawOutput/Stats" + skipped_string + ".txt");
ofstream fout_stats2("RawOutput/TreeStats" + skipped_string + ".txt");
ofstream fout_exp_difference("RawOutput/AllExpDifference" + skipped_string + ".txt");
ofstream fout_max_path("RawOutput/MaxPath" + skipped_string + ".txt");
ofstream fout_max_path2("RawOutput/MaxPath2" + skipped_string + ".txt");
ofstream fout_jaccard("RawOutput/Jaccard" + skipped_string + ".txt");
ofstream fout_name_change("RawOutput/NameChange" + skipped_string + ".txt");
ofstream fout_good_samples("RawOutput/GoodSamples" + skipped_string + ".txt");
ofstream fout_tree_edges("RawOutput/TreeEdges" + skipped_string + ".txt");
ofstream fout_width[15];
// ofstream fout_one_significant("RawOutput/OnlyOneSignificantRoot" + skipped_string + ".txt");
ofstream fout_large_tree_macros("RawOutput/MacroTrees/LargeTreeMacros.txt");
ofstream fout_pass_on_summary("RawOutput/PassOnRawData.txt");
ofstream fout_inherited_summary("RawOutput/InheritanceRawData.txt");
ofstream fout_pass_on_binary_summary("RawOutput/PassOnBinaryRawData.txt");
ofstream fout_inherited_binary_summary("RawOutput/InheritanceBinaryRawData.txt");

void StrongDfs(int x) {
    local_mark[x] = 1;
    for(int temp : graph[x]) {
        if(local_mark[temp] == 0) {
            StrongDfs(temp);
        }
    } 
    fin_time.push_back(x);
}

void StrongDfsRev(int x, int comp_num) {
    rev_comp[x] = comp_num;
    for(int temp : rev_graph[x]) {  
        if(rev_comp[temp] == 0) {
            StrongDfsRev(temp, comp_num);
        }
    }
    comp[comp_num].push_back(x);
}

int CountDfs(int x) {
    local_mark[x] = 1;
    int ret = 1;
    for(int temp : graph[x]) {
        if(local_mark[temp] == 0) {
            ret += CountDfs(temp);
        }
    }
    return ret;
}




string SimpleIntToString(int x) {
    stringstream ss;
    ss << x;
    string ret;
    ss >> ret;
    return ret;
}

string RemoveSpaces(string s) {
    string ret = "";
    for(int i = 0 ; i < (int)s.length(); i++) {
        if(s[i] == ' ') {
            continue;
        }
        ret += s[i];
    }
    return ret;
}

struct Macro {
    int macro_number;
    string paper_id;
    string name;
    vector<int> authors;
    vector<int> experience;
    vector<string> categories;
    int day, month, year;
    int count;
	map<pair<int, int>, int> co_authorship;
    bool operator < (const Macro &other) const {
        if(year == other.year) {
            if(month == other.month) {
                if(day == other.day) {
                    if(authors.size() == other.authors.size()) {
                        for(int i = 0; i < (int)authors.size(); i++) {
                            if(authors[i] == other.authors[i]) {
                                continue;
                            }
                            return authors[i] < other.authors[i];
                        }
                        return macro_number < other.macro_number;
                    }
                    return authors.size() < other.authors.size();
                }
                return day < other.day;
            }
            return month < other.month;
        }
        return year < other.year;
    }
    bool operator == (const Macro &other) const { // published same day
        return ((year == other.year) && (month == other.month) && (day == other.day));
    }

    int getTime() {
        return 500 * year + 40 * month + day;
    }

    string ToString() {
        string ret ="";
        ret += (rev_macro_to_num[macro_number] + " " + SimpleIntToString(day) + "/" + SimpleIntToString(month) + "/" + SimpleIntToString(year) + "\n");
        ret += (paper_id + "\n");
		for(int i = 0; i < (int)authors.size(); i++) {
            ret += rev_author_to_num[authors[i]];
            if(i < (int)authors.size() - 1){
                ret += ", ";
            }
        }
        ret += "\n";
        return ret;
    }
};

bool ExactSame(Macro first, Macro other) {
    if(first.year == other.year) {
        if(first.month == other.month) {
            if(first.day == other.day) {
                if(first.authors.size() == other.authors.size()) {
                    for(int i = 0; i < (int)first.authors.size(); i++) {
                        if(first.authors[i] == other.authors[i]) {
                            continue;
                        }
                        return false;
                    }
                    return true;
                }
                return false;
            }
            return false;
        }
        return false;
    }
    return false;
}

vector<Macro> macros;
vector<Macro> word_bucket[3 * 1000 * 1000]; 
map<int, int> author_experience; // at the end of the code it will be final experience but while running it's the current_exp
int num_above_cut = 0;



int BFS(int x, int option, int macro_number) {
    vector<pair<int, int>> Q; // (SCC number, depth)
    vector<int> par;
    for(int i : comp[x]) {
        Q.push_back(make_pair(i, 0));
        par.push_back(-1);
        local_mark[i] = 1;
    }
    int head = 0;
	vector<pair<int, int>> max_path_2;
    while(head < (int)Q.size()) {
        biggest_interval = max(biggest_interval, local_earliest[Q[head].first]);
		Macro m1 = word_bucket[macro_number][local_earliest_index[Q[0].first]];
		Macro m2 = word_bucket[macro_number][local_earliest_index[Q[head].first]];
		int month_difference = (m2.year - m1.year) * 12 + (m2.month - m1.month);
		max_path_2.push_back(make_pair(Q[head].second, month_difference));
		int index = -1;
        for(int temp : graph[Q[head].first]) {
			index ++;
            if(local_mark[temp] == 0) {
                local_mark[temp] = 1;
                Q.push_back(make_pair(temp, Q[head].second + 1));
                par.push_back(head);
				if(option == 2) {
					fout_tree_edges << rev_author_to_num[rev_local_author_id[Q[head].first]] << " " << rev_local_author_id[Q[head].first] << " ";
					fout_tree_edges << rev_author_to_num[rev_local_author_id[temp]] << " " << rev_local_author_id[temp] << " " << Q.back().second;
					fout_tree_edges << " " << macro_number << " " << graph_edge_index[Q[head].first][index] << " " << word_bucket[macro_number][graph_edge_index[Q[head].first][index]].paper_id << endl;
				}
            }
        }
        head++;
    }
    
    int go_back = head - 1;
    int go_back_author;
    if(Q[head - 1].second > 4) {
        fout_max_path << rev_macro_to_num[x] << endl;
        while(go_back != -1) {
            go_back_author = Q[go_back].first;
            fout_max_path << "Depth: " << Q[go_back].second << ": "; 
            fout_max_path << rev_author_to_num[rev_local_author_id[go_back_author]] << "    ";
            fout_max_path << endl;
            go_back = par[go_back];
        }
        fout_max_path << "___________________________________" << endl;
        int depth = Q[head - 1].second;
        int local_width[15];
        memset(local_width, 0, sizeof local_width);
        local_paper_depth_set.clear();
        for(int i = 0; i < head; i++) {
            local_width[Q[i].second]++;
            local_paper_depth_set.insert(local_earliest_index[Q[i].first]);
            if(i == head - 1 || Q[i].second != Q[i + 1].second) {
                paper_depth_count[depth][Q[i].second].push_back(local_paper_depth_set.size());
                local_paper_depth_set.clear();
            }
        }
		if(option == 1) {
	        for(int i = 0; i <= depth; i++) {
	            width[depth][i].push_back(local_width[i]);
			}
		}
    }
	if(option == 2 && rev_macro_to_num[macro_number].length() >= Macro_length_filter) {
		for(auto p : max_path_2) {
			fout_max_path2 << Q[head-1].second << " " << p.first << " " << p.second << endl;
		}
	}
    return Q[head - 1].second;
}

bool MacroHasBigComponent(int x) {
	if(rev_macro_to_num[x] == "\\mathrel{\\raise0.35ex\\hbox{$\\scriptstyle >$}\\kern-0.6em\\lower0.40ex\\hbox{{$\\scriptstyle \\sim$}}}" ||
			rev_macro_to_num[x] == "\\mathrel{\\raise0.35ex\\hbox{$\\scriptstyle <$}\\kern-0.6em\\lower0.40ex\\hbox{{$\\scriptstyle \\sim$}}}" || 
			rev_macro_to_num[x] == "\\hbox{$\\ph\\cm^{-2}\\s^{-1}\\,$}" || 
			rev_macro_to_num[x] == "\\hbox{$\\erg\\cm^{-2}\\s^{-1}\\,$}" || 
			rev_macro_to_num[x] == "{\\rm\\thinspace gauss}" || 
			rev_macro_to_num[x] == "\\hbox{$\\Msun\\pc^{-3}\\,$}") {
		return true;
	}
	return false;
}

int break_index = - 1;
int final_size = 0;
bool solve(int x, int option) { // if option == 0 then all papers, if option == 1 go up to break_index;
	if(option == 0) {
		break_index = -1;
		final_size = 0;
	} else {
		if(break_index == -1) {
			return false;
		}
	}
	int macro_number = x;
    if(word_bucket[x].size() < Macro_paper_usage || rev_macro_to_num[word_bucket[x][0].macro_number].length() < Macro_length_filter) { // macro needs to be used and should have a length
//		if(word_bucket[x].size() >= Macro_paper_usage && rev_macro_to_num[x].length() > 10 && rand() % 5 == 0) {
//		
//		} else {
			return false;
//		}
    }
    cerr << x  << " " << rev_macro_to_num[word_bucket[x][0].macro_number] << endl;
    cerr << "Number of papers: " << word_bucket[x].size() << endl;
    num_above_cut++;
//       cerr << "STARTING FEYNMAN" << endl;
    // Richard Feynman trace back

    indecies_contain_word.clear();
    local_author_id.clear();
    fill(rev_local_author_id.begin(), rev_local_author_id.end(), 0);
    local_counter = 0;
    for(int i = 0; i < H; i++) {
        graph[i].clear();
        rev_graph[i].clear();
		graph_edge_index[i].clear();
        comp[i].clear();
    }
    fill(local_earliest.begin(), local_earliest.end(), 0);
    fill(local_earliest_index.begin(), local_earliest_index.end(), 0);
    fin_time.clear();
    fill(local_mark.begin(), local_mark.end(), 0);
    fill(rev_comp.begin(), rev_comp.end(), 0); 

    cerr << "BUILDING GRAPH " << endl;
    for(int i = 0; i < (int) word_bucket[x].size(); i++) {
		if(i > break_index && option == 1){
			break;
		}
        for(int author : word_bucket[x][i].authors) {
            if(local_author_id.find(author) == local_author_id.end()){
                rev_local_author_id[local_counter] = author;
                local_author_id[author] = local_counter++;
                local_earliest[local_counter - 1] = word_bucket[x][i].getTime();
                local_earliest_index[local_counter - 1] = i;
				if(local_counter >= 70 && option == 0 && break_index == -1){
					break_index = i;
				}
            }
        }

		bool check_pass_on = false;
		bool check_inherited[100], check_passed_on[100];
		memset(check_inherited, 0, sizeof check_inherited);
		memset(check_passed_on, 0, sizeof check_passed_on);

        int j = -1;
		int is_interesting = 0;
		if(interesting_macros2.find(macro_number) != interesting_macros2.end()) {
			is_interesting = 1;	
		}
        for(int author1 : word_bucket[x][i].authors) {
            j++;
            int k = -1;
            for(int author2 : word_bucket[x][i].authors) {
                k++;
                if(author1 == author2) {
                    continue;
                }
                int first = local_author_id[author1];
                int second = local_author_id[author2];
                int exp_first = word_bucket[x][i].experience[j];
                int exp_second = word_bucket[x][i].experience[k];
                if(local_earliest[first] <= local_earliest[second] && local_earliest[second] == word_bucket[x][i].getTime()) {
					check_pass_on = true;
					graph[first].push_back(second);
                    rev_graph[second].push_back(first);
					graph_edge_index[first].push_back(i);
                }
                if(local_earliest[first] < local_earliest[second] && local_earliest[second] == word_bucket[x][i].getTime() && option == 0) {
                    fout_exp_difference << exp_first - exp_second << endl;
					if(exp_first < 40 && check_passed_on[j] == false) { 
						if(check_passed_on[j] == false) {
							fout_pass_on_summary << author1 << " " <<  word_bucket[x][i].year * 12 + word_bucket[x][i].month << " ";
							fout_pass_on_summary << exp_first << " " << 1 + is_interesting << " " << author_experience[author1] << endl; 
							check_passed_on[j] = true;
						}
						if(check_inherited[k] == false) {
							fout_inherited_summary << author2 << " " <<  word_bucket[x][i].year * 12 + word_bucket[x][i].month << " ";
							fout_inherited_summary << exp_second << " " << 1 + is_interesting << " " << author_experience[author2] << endl; 
							check_inherited[k] = true;
						}
					}
                }
            }
        }
		j = -1;
		for(int author1 : word_bucket[x][i].authors) {	
			j++;
			int exp_first = word_bucket[x][i].experience[j];
			int has_passed_on = 1;
			int has_inherited = 1;
			if(check_passed_on[j] == false) {
				has_passed_on = 0;
			}
			if(check_inherited[j] == false) {
				has_inherited = 0;
			}
			fout_pass_on_binary_summary << author1 << " " << exp_first << " " << word_bucket[x][i].year * 12 + word_bucket[x][i].month << " ";
			fout_pass_on_binary_summary << has_passed_on << " " << author_experience[author1] << endl; 

			fout_inherited_binary_summary << author1 << " " << exp_first << " " << word_bucket[x][i].year * 12 + word_bucket[x][i].month << " ";
			fout_inherited_binary_summary << has_inherited << " " << author_experience[author1] << endl; 
		}
		if(check_pass_on == false) {
			j = -1;
			for(int author1 : word_bucket[x][i].authors) {
				j++;
				int exp_first = word_bucket[x][i].experience[j];
				if(exp_first < 30 && check_passed_on[j] == false) { 
					fout_pass_on_summary << author1 << " " <<  word_bucket[x][i].year * 12 + word_bucket[x][i].month << " ";
					fout_pass_on_summary << exp_first << " " << 0 << " " << author_experience[author1] << endl; 
					fout_inherited_summary << author1 << " " <<  word_bucket[x][i].year * 12 + word_bucket[x][i].month << " ";
					fout_inherited_summary << exp_first << " " << 0 << " " << author_experience[author1] << endl; 
				}
			}
		}

	}
	cerr << "option: break_index and local_counter" << option << ": " << break_index << " " << local_counter << endl;
	if(option == 0) {
		 final_size = local_counter;
	}
#if 0
    //    cerr << "STARTED SCC" << endl;
    // strongly connected component
    for(int i = 0 ; i < local_counter; i++) {
        if(local_mark[i] == 0) {
            StrongDfs(i);
        }
    }
    fill(local_mark.begin(), local_mark.end(), 0);
    int comp_num = 1;
    set<int> comps_to_check;
    for(int i = local_counter - 1; i >= 0; i--) {
        if(rev_comp[fin_time[i]] == 0) {
            comps_to_check.insert(comp_num);
            StrongDfsRev(fin_time[i], comp_num);
            /*
            cerr << i << " " << local_earliest[fin_time[i]] / 500 << " " << (local_earliest[fin_time[i]] % 500) / 40 << "        " << fin_time[i]<< endl;
            for(int k = 0; k < local_counter; k++) {
                if(local_mark[k] == comp_num) {
                    cerr <<  local_earliest[k] / 500 << " " << (local_earliest[k] % 500) / 40 << "     " << k << endl;
                }
            }
            cerr << "___________________________________________________" << endl;
            */
            comp_num++;
        }
    }
    cerr << "GOT THE COMPONENTS" << endl;
    for(int i = 0; i < local_counter; i++) {
        for(int x : graph[i]) {
            if(rev_comp[x] != rev_comp[i]) {
                comps_to_check.erase(rev_comp[x]);
            }
        }
    }
    cerr << "Num of components: " << comp_num << endl;
    cerr << "Num of candidate components to check: " << comps_to_check.size() << endl;
    cerr << "FIND THE BEST COMPONENT" << endl;
    int max_people[10]; // number of authors
    int best_comp[10]; // roots
	int max_path[10];
	memset(max_people, 0, sizeof max_people);
	memset(best_comp, 0, sizeof best_comp);
	set<pair<int, int>> big_comp_and_roots;
    for(int x : comps_to_check) {
        fill(local_mark.begin(), local_mark.end(), 0);
        int temp = comp[x][0];
        int num = CountDfs(temp);
		big_comp_and_roots.insert(make_pair(num, x));
		while(big_comp_and_roots.size() > 10) {
			big_comp_and_roots.erase(big_comp_and_roots.begin());
		}
    }
	cerr << "----> size of comps and roots set: " << big_comp_and_roots.size() << endl;
    // Find the intersection of two biggest sets.
	int comp_count = 0;
	{
		auto it = big_comp_and_roots.end();
		while(it != big_comp_and_roots.begin() && comp_count < 10) {
			it--;
			max_people[comp_count] = it->first;
			best_comp[comp_count] = it->second;
			comp_count++;
		}
	}

    cerr << " Finding the intersection of two biggest sets" << endl;
    fill(local_mark.begin(), local_mark.end(), 0);
    int temp = comp[best_comp[0]][0];
    CountDfs(temp);
    int intersection = max_people[1];
    if(comp[best_comp[1]].size() > 0 && local_mark[comp[best_comp[1]][0]] == 0) {
        intersection -= CountDfs(comp[best_comp[1]][0]);
    }
    cerr << " LOOK INTO THE BEST COMPONENT" << endl;
    set<int> seed_authors;
    set<int> seed_authors_copy;
    set<int> seed_paper_indecies;
    for(int i = 0; i < (int)comp[best_comp[0]].size() ; i++) {
        seed_authors.insert(rev_local_author_id[comp[best_comp[0]][i]]);
        seed_authors_copy.insert(rev_local_author_id[comp[best_comp[0]][i]]);
    }
    set<int> all_people;
    for(int i = 0; i < (int)word_bucket[x].size(); i++) {
        for(int author : word_bucket[x][i].authors) {
            all_people.insert(author);
            if(seed_authors.find(author) != seed_authors.end()) {
                seed_paper_indecies.insert(i);
                seed_authors.erase(author);
            }
        }
    }
    cerr << " Finding stats " << endl;
	{
        fill(local_mark.begin(), local_mark.end(), 0);
        int local_biggest_interval;
        biggest_interval = 0;
        max_path[0] = BFS(best_comp[0], 1, macro_number); // means we are looking for width and is best component
        local_biggest_interval = biggest_interval - local_earliest[comp[best_comp[0]][0]];
		for(int i = 1; i < comp_count; i++) {
			fill(local_mark.begin(), local_mark.end(), 0);
			if(best_comp[i] > 0) {
				max_path[i] = BFS(best_comp[i], 0, macro_number); // 0 means it's not the best component
			}
		}
        int author_count[10];
        int paper_count	[10];
		memset(author_count, 0, sizeof author_count);
		memset(paper_count, 0, sizeof paper_count);
        for(int i = 0; i < local_counter; i++) {
			for(int j = 0; j < comp_count; j++) {
			    if(rev_comp[i] == best_comp[j]) {
					 author_count[j]++;
				}
			}
		}
		int root_paper = -1;
		for(int i = 0; i < (int)word_bucket[x].size(); i++) {
			for(int j = 0; j < comp_count; j++) {
				if(word_bucket[x][i].getTime() == local_earliest[comp[best_comp[j]][0]]){
					bool check_all_are_new = true;
					bool first_author_exists = false;
					for(int author : word_bucket[x][i].authors) {
						int current_id = local_author_id[author];
						if(current_id == comp[best_comp[j]][0]) {
							first_author_exists = true;
						}
						if(local_earliest[current_id] != word_bucket[x][i].getTime()) {
							check_all_are_new = false;
							break;
						}
					}
					if(check_all_are_new == true && first_author_exists == true) {
						paper_count[j]++;
						if(j == 0) {
							root_paper = i;
						}
					}
				}
			}
		}
		if(root_paper == - 1){
			cout << "WE HAVE A PROBLEM" << endl;
			cout << rev_macro_to_num[word_bucket[x][0].macro_number] << endl; 
			exit(0);
		}
		if(option == 0){
			for(int i = 0; i < 10; i++) {
				fout_stats << paper_count[i] << " ";
			}

			fout_stats << word_bucket[x].size() << " ";
			for(int i = 0; i < 10; i++) {
				fout_stats << author_count[i] << " ";
			}
			for(int i = 0; i < 10; i++) {
				fout_stats << max_people[i] << " ";
			}
			fout_stats << all_people.size() << " ";
			fout_stats << local_biggest_interval << " ";
			for(int i = 0; i < 10; i++) {
				fout_stats << max_path[i] << " ";
			}
			for(int i = 0; i < (int)word_bucket[x][root_paper].categories.size(); i++) {
				fout_stats << word_bucket[x][root_paper].categories[i];
				if(i != (int)word_bucket[x][root_paper].categories.size() - 1) {
					fout_stats << "/";
				}
			}
			fout_stats << " " << rev_macro_to_num[word_bucket[x][0].macro_number] << endl; 
		} else if(option ==1 && final_size > 100){
			for(int i = 0; i < 10; i++) {
				fout_stats2 << paper_count[i] << " ";
			}

			fout_stats2 << break_index << " ";
			for(int i = 0; i < 10; i++) {
				fout_stats2 << author_count[i] << " ";
			}
			for(int i = 0; i < 10; i++) {
				fout_stats2 << max_people[i] << " ";
			}
			for(int i = 0; i < 10; i++) {
				fout_stats2 << max_path[i] << " ";
			}
			if(final_size > 176) {
				fout_stats2 << "1" << endl; 
			} else {
				fout_stats2 << "0" << endl; 
			}
		
		}

		if(max_path[0] > 4 && option == 0) {
			if(max_people[0] / (double) all_people.size() > 0.5) {
				interesting_macros.push_back(x);
				root_author[x] = rev_author_to_num[rev_local_author_id[comp[best_comp[0]][0]]];
			}
			fill(local_mark.begin(), local_mark.end(), 0);
			BFS(best_comp[0], 2, macro_number); // 2 means we want to print out edges
			fout_large_tree_macros << rev_macro_to_num[x] << endl;
		}
	}

	// cerr << seed_paper_indecies.size() << endl;
	// int best_index = *seed_paper_indecies.begin();
	// word, number of people on biggest impact, number of people on second biggest impact, intersecion of the two biggest sets,  all the people used that word, a sample text in the biggest component
	//    feynman_fout << rev_macro_to_num[word_bucket[x][best_index].macro_number] << endl;
	//    feynman_fout << "Number of people in max and second max component: " << max_people  << " " << second_max_people << endl;
	//    feynman_fout << "Intersection of the two biggest components and number of all people using the macro: " << intersection << " " << all_people.size() << " " << endl;

	//    feynman_fout << max_people  << " " << second_max_people << endl;
	//    feynman_fout << intersection << " " << all_people.size() << " " << endl;
	// When the best paper was published, when the best burst started
	//    feynman_fout << word_bucket[x][best_index].day  << "/" << word_bucket[x][best_index].month << "/" << word_bucket[x][best_index].year << endl;
	//    for(int author : seed_authors_copy) {
	//       feynman_fout << "Author: " << rev_author_to_num[author] << endl;
	//    }
	//    feynman_fout << endl << "________________________________________" << endl;

	if(option == 0) {
		feynman_fout << max_people  << " " << all_people.size() << endl;
		fout_frac_length << rev_macro_to_num[x].length() << " " << max_people[0] / (double) all_people.size() << endl;
		fout_biggest_second_biggest << max_people[0] / (double) all_people.size()  << " " << max_people[1] / (double) all_people.size() << endl;
		fout_heat_map <<  rev_macro_to_num[word_bucket[x][0].macro_number].length() << " " << all_people.size() << " " << max_people[0] / (double) all_people.size() << endl; 
		if(MacroHasBigComponent(x) == true) {
			for(int l = 0; l < (int)word_bucket[x].size(); l++) {
				fout_good_samples << l << endl;
				fout_good_samples << word_bucket[x][l].name << endl;
				fout_good_samples << word_bucket[x][l].ToString() << endl;
			}
			fout_good_samples << "_____________________________________________________________" << endl;
		}
	}
#endif
	return true;
}

double Jaccard(int x, int y) {
	set<int> x_authors;
	set<int> all_authors;
	set<int> intersect_authors;

	for(Macro macro : word_bucket[x]) {
		for(int author : macro.authors) {
			x_authors.insert(author);
			all_authors.insert(author);
		}
	}

	for(Macro macro : word_bucket[y]) {
		for(int author : macro.authors) {
			all_authors.insert(author);
			if(x_authors.find(author) != x_authors.end()) {
				intersect_authors.insert(author);
			}
		}
	}
	return ((double) intersect_authors.size()) / all_authors.size();
}

const int Max_N = 20000 + 10;
vector<int> bip_graph[Max_N];
int match[Max_N], mark[Max_N];

bool dfs(int x) {
	mark[x] = 1;
	for(int y : bip_graph[x]) {
		if(match[y] == 0) {
			match[y] = x;
			return true;
		}
		if(mark[match[y]] == 1) {
			continue;
		}
		if(dfs(match[y]) == true) {
			match[y] = x;
			return true;
		}
	}
	return false;
}


int BipartiteMatching(int x, int y, bool is_vol) {
	map<int, int> author_to_local_id, rev_author_to_local_id;
	int author_count = 1;
	memset(mark, 0, sizeof mark);
	memset(match, 0, sizeof match);
	int temp = 0;
	for(Macro macro : word_bucket[x]) {
		temp++;
		for(int author : macro.authors) {
			if(author_to_local_id.find(author) == author_to_local_id.end()) {
				author_to_local_id[author] = author_count;
				rev_author_to_local_id[author_count] = author;
				author_count++;
				if(author_count == y && is_vol == false) {
					return temp;
				}
			}
		}
	}
	int paper_count = 1;
	int volume = 0;
	for(Macro macro : word_bucket[x]) {
		for(int author : macro.authors) { // add edges for the new paper
			bip_graph[paper_count].push_back(author_to_local_id[author]);	
		}
		if(dfs(paper_count) == true) {
			memset(mark, 0, 4 * paper_count + 8);
			volume++;
			if(volume == y && is_vol == true) {
				break;
			}
		}
		paper_count++;
	}
	for(int i = 0; i < paper_count; i++) {
		bip_graph[i].clear();
	}
	if(y == 0) { // return volume if you want the volume metric but we can also return author_count
		if(is_vol == true) {
			return volume;
		} else {
			return author_count;
		}
	} else {
		return paper_count;	
	}
}

int main() {
	ifstream fin("All_Arxiv_Macros.txt");
	int macro_counter = 1;
	int author_counter = 1;
	string s, temp;
	int skipped = 0;

	while(getline(fin, s)) {
        Macro macro;
        {
            stringstream ss(s);
            ss >> s;
            getline(ss, s);
            int first_letter = 0;
            int last_letter = s.size() - 1;
            while(first_letter < (int)s.size() && s[first_letter] == ' ') {
                first_letter++;
            }
            while(last_letter > first_letter && s[last_letter] == ' ') {
                last_letter--;
            }
            if(first_letter == (int)s.size()) {
                temp = "";
            } else {
                temp = s.substr(first_letter, last_letter-first_letter + 1);
            }
            s = temp;
            if(macro_to_num.find(s) == macro_to_num.end()) {
                macro_to_num[s] = macro_counter;
                rev_macro_to_num[macro_counter] = s;
                macro_counter++;
            }
            macro.macro_number = macro_to_num[s];
        } 
        {
            getline(fin, s);
            stringstream ss(s);
            ss >> s;
            getline(ss, s);
            int first_letter = 0;
            int last_letter = s.size() - 1;
            while(first_letter < (int)s.size() && s[first_letter] == ' ') {
                first_letter++;
            }
            while(last_letter > first_letter && s[last_letter] == ' ') {
                last_letter--;
            }
            if(first_letter == (int)s.size()) {
                temp = "";
            } else {
                temp = s.substr(first_letter, last_letter-first_letter + 1);
            }
            s = temp;
            macro.name = s;
        }
        {
            getline(fin, s);
            stringstream ss(s);
            s.clear();
            ss >> s;
            ss >> macro.paper_id; 
        }
        {
            getline(fin, s);
            stringstream ss(s);
            s.clear();
            ss >> s;
            ss >> macro.count; 

        }
        {
            getline(fin, s);
            stringstream ss(s);
            s.clear();
            ss >> s;
            ss >> macro.day >> macro.month >> macro.year; 
        }
        {
			/*
            string t;
            getline(fin, s);
            stringstream ss(s);
            s.clear();
            ss >> s;
            while(ss >> s) {
                macro.categories.push_back(s);   
            }
			*/
        }
        {
            while(getline(fin, s)) {
                if(s == "") {
                    break;
                }
                stringstream ss(s);
                ss >> s;
                getline(ss, s);
                s = RemoveSpaces(s);
                if(author_to_num.find(s) == author_to_num.end()) {
                    author_to_num[s] = author_counter;
                    rev_author_to_num[author_counter] = s;
                    author_counter++;
                }
                macro.authors.push_back(author_to_num[s]);
            }
            if(macro.count == 1 && has_skipped == true) {
                skipped++;
                getline(fin, s);
                continue;
            }
            if(macro.authors.size() < 50 && macro.authors.size() > 0 && temp != "") { 
				macros.push_back(macro); 
            }
        }
        getline(fin, s); // you have two empty lines
    }

	{ // read "interesting" macros - max_depth > 4
		ifstream fin_interesting_macros("RawOutput/MacroTrees/LargeTreeMacros2.txt"); // 2 because we are writing on 1
		while(getline(fin_interesting_macros, s)) {
			interesting_macros2.insert(macro_to_num[s]);
		}
	}

	cerr << "------> " << skipped << endl;
	sort(macros.begin(), macros.end());
	{
		set<pair<int, string> > body_name;
		int paper_count = 0;
		int sum_authors_on_papers = 0;
		for(int i = 0; i < (int) macros.size(); i++) {
			body_name.insert(make_pair(macros[i].macro_number, macros[i].name));
			for(int j = 0 ; j < (int)macros[i].authors.size(); j++) {
				int author = macros[i].authors[j];
				if(author_experience.find(author) == author_experience.end()) {
					author_experience[author] = 0;
				}
				macros[i].experience.push_back(author_experience[author]);
			}
			if(i == 0 || !ExactSame(macros[i], macros[i - 1])) {
				paper_count++;
				sum_authors_on_papers += macros[i].authors.size();
				for(int j = 0 ; j < (int)macros[i].authors.size(); j++) {
					int author = macros[i].authors[j];
					author_experience[author]++;
				}
			}
		}
		cout << macro_counter << " different macros used in  " << macros.size() << " comments" << endl;
		cout << "authors count: " << author_experience.size() << endl;
		cout << "paper count: "	 << paper_count << endl;
		cout << "Avg authors per paper: " << sum_authors_on_papers / (double) paper_count << endl;
		cout << "Avg diff name for one body:  " << body_name.size() / (double) macro_counter << endl;
	}
    for(int i = 0; i < (int)macros.size(); i++) {
        word_bucket[macros[i].macro_number].push_back(macros[i]);
    }
    macros.clear();
	for(int i = 0; i < 10; i++) {
		fout_stats << "#papersRoot" << to_string(i) << " ";
		fout_stats2 << "#papersRoot" << to_string(i) << " ";
	}
	fout_stats << "#papersAll ";
	fout_stats2 << "BreakPapersFor70Authors ";
	for(int i = 0; i < 10; i++) {
		fout_stats << "#authorsRoot" << to_string(i) << " ";
		fout_stats2 << "#authorsRoot" << to_string(i) << " ";
	}
	for(int i = 0; i < 10; i++) {
		fout_stats << "#authorsComponent" << to_string(i) << " ";
		fout_stats2 << "#authorsComponent" << to_string(i) << " ";
	}
	fout_stats << "#authorsAll  ";
	fout_stats << "BiggestAdaptingInterval ";
	for(int i = 0; i < 10; i++) {
		fout_stats << "#MaxPathRoot" << to_string(i) << " ";
		fout_stats2 << "#MaxPathRoot" << to_string(i) << " ";
	}
	fout_stats << "Categories Macro" << endl;
	fout_stats2 << "Label" << endl;
	fout_tree_edges << "Source Destination DepthOfDestination MacroNumber IndexInWordBucket" << endl;
	fout_max_path2 << "FinalDepth CurrentDepth MonthTookFromRoot" << endl; 
	fout_pass_on_summary << "Author Date CurrentExperience DidPassOn FinalExperience" << endl;
    for(int i = 1; i < (int)macro_counter; i++) {
        if(solve(i, 0) == true) {
//			solve(i, 1);
		}
    }
    cerr << "num above cut: " <<  num_above_cut << endl;
    cerr << "macros with max_path larger than threshold: " << interesting_macros.size() << endl;

#if JACCARD	
// Find the jaccard score over authors of two macros! If two macros have high jaccard score then they have similar authors
    { // Jaccard scores
        vector<pair<double, pair<int, int>> > jaccard_scores;
        for(int macro1 : interesting_macros) {
            for(int macro2 : interesting_macros) {
                if(macro1 <= macro2) {
                    continue;
                }
                jaccard_scores.push_back(make_pair(Jaccard(macro1, macro2), make_pair(macro1, macro2)));
            }
        }
        sort(jaccard_scores.begin(), jaccard_scores.end());
        for(auto temp : jaccard_scores) {
            int x1 = temp.second.first;
            int x2 = temp.second.second;
            fout_jaccard << temp.first << " " << rev_macro_to_num[x1] << " ---- " << rev_macro_to_num[x2] << endl;
        }
    }
#endif
#if NAME_PRINTER
    { // This piece of code prints the different names used for a body
        set<string> names;
        for(int x : interesting_macros) {
            fout_name_change << rev_macro_to_num[x] << " ::::::::::::::::: "; 
            for(Macro macro : word_bucket[x]) {
                names.insert(macro.name);
            }
            for(string name : names) {
                fout_name_change << name << " " ;
            }
            fout_name_change << endl;
            names.clear();
        }
    }
#endif
#if ONION_SHAPE_TREES
    { // Width (number of authors) and number of papers for each level
        for(int i = 5; i < 13; i++) {
            fout_width[i].open("RawOutput/Width" + to_string(i) + skipped_string + ".txt");
            for(int j = 0; j <= i; j++) {
                if(width[i][j].size() == 0) {
                    continue;
                }
                sort(width[i][j].begin(), width[i][j].end());
                sort(paper_depth_count[i][j].begin(), paper_depth_count[i][j].end());
                fout_width[i] << j << " " << width[i][j][width[i][j].size() / 2] << " " << paper_depth_count[i][j][paper_depth_count[i][j].size() / 2] << endl;
				// each paper will give you an SCC which is the paper_depth_count, but each paper can have several authors! width is the author count of that particular depth
            }
            fout_width[i].close();
        }
    }
#endif

#if VOLUME
	bool is_vol = false;
	vector<pair<int, int> > volumes; // volume, macro_number
	for(int x : interesting_macros) {
		int volume = BipartiteMatching(x, 0, is_vol);
		volumes.push_back(make_pair(volume, x));
	}
	sort(volumes.begin(), volumes.end());
	cout << "vol size: " << volumes.size() << endl;
	int index_small = volumes.size() * 3 / 6;
	cout << "3/6 threshold: " << volumes[index_small].first << " " << volumes[index_small].second << endl;
	int index_big = volumes.size() * 4 / 6;
	cout << "4/6 threshold: " << volumes[index_big].first << " " << volumes[index_big].second << endl;
	fill(local_earliest.begin(), local_earliest.end(), 0);
	fill(local_earliest_index.begin(), local_earliest_index.end(), 0);
	local_author_id.clear();
	fill(rev_local_author_id.begin(), rev_local_author_id.end(), 0);

	ofstream fout_volumes("RawOutput/BipartiteThresholds.txt");
	fout_volumes << "FinalVolume BreakIndex Label RootAuthor MacroBody" << endl;
	for(int i = volumes.size() / 6; i <= index_small; i++) {
		int index = BipartiteMatching(volumes[i].second, volumes[index_small/3].first, is_vol);
		fout_volumes << volumes[i].first << " " << index << " " << 0 << " " << root_author[volumes[i].second] << " ";
		fout_volumes << rev_macro_to_num[volumes[i].second] << endl;
	}
	for(int i = index_big ; i < (int)volumes.size(); i++) {
		int index = BipartiteMatching(volumes[i].second, volumes[index_small/3].first, is_vol); 
		fout_volumes << volumes[i].first << " " << index << " " << 1 << " " << root_author[volumes[i].second] << " ";
		fout_volumes << rev_macro_to_num[volumes[i].second] << endl;
	}

	#endif
	return 0;
} 

