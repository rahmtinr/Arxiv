#include<iostream>
#include<cstring>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>

using namespace std;

const int Macro_length_filter = 20;
const int Macro_paper_usage = 30;
const int BASE_YEAR = 1990;

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
vector<vector<int> > graph(H),rev_graph(H);
vector<int> local_earliest(H);
vector<int> local_earliest_index(H);
vector<int> fin_time;
vector<int> local_mark(H);
vector<vector<int> > comp(H);
int biggest_interval;
vector<int> width[15][15];
vector<int> paper_depth_count[15][15];
set<int> local_paper_depth_set;
vector<int> current_experiences_on_time_windows[100];

map<pair<int, int>, int> global_co_authorship; // at each time as we move forward it has the number of papers two authors have written together
map<int, int> global_co_authorship_distribution_current, tree_co_authorship_distribution_current; // from co_authorship count to number of instances
map<int, int> global_co_authorship_distribution_final, tree_co_authorship_distribution_final; // from co_authorship count to number of instances
int total_co_authorships, total_edge_count;
map<int, int> local_co_authorship_distribution_current, local_co_authorship_distribution_final;
int local_tree_edges = 0;

vector<int> interesting_macros;
set<pair<string, int>> bfs_out_deg; // author_name and macro_number

bool has_skipped = false;
string skipped_string = "";
int macro_counter = 1;
int author_counter = 1;
int skipped = 0;

//ofstream feynman_fout("Feynman_skipped.txt");
//ofstream feynman_fout("Feynman.txt");

ofstream fout_macro_indexer("RawOutput/MacroTrees/indexer.txt");
void StrongDfsRev(int x, int comp_num) {
    rev_comp[x] = comp_num;
    for(int temp : rev_graph[x]) {  
        if(rev_comp[temp] == 0) {
            StrongDfsRev(temp, comp_num);
        }
    }
    comp[comp_num].push_back(x);
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

int MacroDifferenceInMonth(Macro m1, Macro m2) {
	if(m1.year > m2.year) {
		swap(m1,m2);
	} else if(m1.year == m2.year && m1.month > m2.month) {
		swap(m1, m2);
	}
	return (m2.year - m1.year) * 12 + (m2.month - m1.month);
}

bool cmp(pair<pair<int, Macro>, int> a, pair<pair<int, Macro>, int> b) {
	return a.first < b.first;
}

vector<Macro> macros;
vector<Macro> word_bucket[3 * 1000 * 1000]; 
map<int, int> author_experience; // at the end of the code it will be final experience but while running it's the current_exp
vector<pair<pair<int, Macro>, int> > dag_edges[40]; // Their current experience and final experience.
vector<pair<pair<int, Macro>, int> > internal_dag_edges[40]; // Their current experience and final experience.
int num_above_cut = 0;

void read() {
    ifstream fin("All_Arxiv_Macros.txt");
    string s, temp;
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
		ifstream fin_interesting_macros("RawOutput/MacroTrees/LargeTreeMacros.txt");
		while(getline(fin_interesting_macros, s)) {
			interesting_macros.push_back(macro_to_num[s]);
		}
	}
}

bool solve(int x) {
	cerr << x << endl;
    if(word_bucket[x].size() < Macro_paper_usage || rev_macro_to_num[word_bucket[x][0].macro_number].length() < Macro_length_filter) { // macro needs to be used and should have a length
        return false;
    }
    cerr << x  << " " << rev_macro_to_num[word_bucket[x][0].macro_number] << endl;
    cerr << "Number of papers: " << word_bucket[x].size() << endl;
    num_above_cut++;
//       cerr << "STARTING FEYNMAN" << endl;
    // Richard Feynman trace back

    indecies_contain_word.clear();
    local_author_id.clear();
	local_co_authorship_distribution_current.clear();
	local_co_authorship_distribution_final.clear();
	local_tree_edges = 0;
    fill(rev_local_author_id.begin(), rev_local_author_id.end(), 0);
    local_counter = 0;
    fill(local_earliest.begin(), local_earliest.end(), 0);
    fill(local_earliest_index.begin(), local_earliest_index.end(), 0);

	ofstream fout_macro_tree_nodes("RawOutput/MacroTrees/nodes-" + to_string(x) + ".txt");
	ofstream fout_macro_tree_edges("RawOutput/MacroTrees/edges-" + to_string(x) + ".txt");
	fout_macro_tree_nodes << "Id Label" << endl;
	fout_macro_tree_edges << "Source Target Label" << endl;
	fout_macro_indexer << x << " " << rev_macro_to_num[x] << endl;
    for(int i = 0; i < (int)word_bucket[x].size(); i++) {
        for(int author : word_bucket[x][i].authors) {
            if(local_author_id.find(author) == local_author_id.end()){
                rev_local_author_id[local_counter] = author;
                local_author_id[author] = local_counter++;
                local_earliest[local_counter - 1] = word_bucket[x][i].getTime();
                local_earliest_index[local_counter - 1] = i;
            }
        }
        int j = -1;
		int earliest_author = 1000 * 1000 * 1000;
		for(int author1 : word_bucket[x][i].authors) {
			earliest_author = min(earliest_author, local_earliest[local_author_id[author1]]);
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
                if((local_earliest[first] < local_earliest[second] || local_earliest[first] == earliest_author) && local_earliest[second] == word_bucket[x][i].getTime()) {
					fout_macro_tree_edges << rev_author_to_num[author1] << " " << rev_author_to_num[author2] << " " << word_bucket[x][i].year;
					fout_macro_tree_edges << " " << word_bucket[x][i].month << " " << word_bucket[x][i].paper_id << endl;
						int rel_year = word_bucket[x][i].year - BASE_YEAR; 
						int co_authorship_exp_current = word_bucket[x][i].co_authorship[make_pair(min(author1, author2), max(author1, author2))];
						int co_authorship_exp_final = global_co_authorship[make_pair(min(author1, author2), max(author1, author2))];
						if(co_authorship_exp_current == 1) {
							local_co_authorship_distribution_current[co_authorship_exp_current]++;
							local_co_authorship_distribution_final[co_authorship_exp_final]++;
							local_tree_edges++;
							if(bfs_out_deg.find(make_pair(rev_author_to_num[author2], x)) == bfs_out_deg.end()) {
								dag_edges[rel_year-(rel_year%2)].push_back(make_pair(make_pair(co_authorship_exp_current, word_bucket[x][i]), co_authorship_exp_final));
							}
							if(bfs_out_deg.find(make_pair(rev_author_to_num[author2], x)) != bfs_out_deg.end()) {
								if(word_bucket[x][i].year == 2006 || word_bucket[x][i].year == 2007) {
									cerr << "Internal: " << rev_author_to_num[author1] << " " << rev_author_to_num[author2] << " " 
										<< co_authorship_exp_current << " " << co_authorship_exp_final << endl;
								}
								internal_dag_edges[rel_year-(rel_year%2)].push_back(make_pair(make_pair(co_authorship_exp_current, word_bucket[x][i]), co_authorship_exp_final));
							}
						}
                }
            }
        }
    }
	for(pair<int, int> p : local_author_id) {
		fout_macro_tree_nodes << rev_author_to_num[p.first] << " " << rev_author_to_num[p.first] << endl;
	}

	total_edge_count += local_tree_edges;	
	for(pair<int, int> p : local_co_authorship_distribution_current) {
		tree_co_authorship_distribution_current[p.first] += p.second;
	}
	for(pair<int, int> p : local_co_authorship_distribution_final) {
		tree_co_authorship_distribution_final[p.first] += p.second;
	}
    return true;
}

int main() {
	read();
    sort(macros.begin(), macros.end());
	{
		set<pair<int, string> > body_name;
		int paper_count = 0;
		int sum_authors_on_papers = 0;
		for(int i = 0; i < (int) macros.size(); i++) {
			body_name.insert(make_pair(macros[i].macro_number, macros[i].name));
			for(int j = 0 ; j < (int)macros[i].authors.size(); j++) {
				int author = macros[i].authors[j];
				macros[i].experience.push_back(author_experience[author]);
			}
			if(i == 0 || !ExactSame(macros[i], macros[i - 1])) {
				paper_count++;
				sum_authors_on_papers += macros[i].authors.size();
				for(int j = 0 ; j < (int)macros[i].authors.size(); j++) {
					int author = macros[i].authors[j];
					author_experience[author]++;
				}
				for(int author1 : macros[i].authors) {
					for(int author2 : macros[i].authors) {
						if(author1 >= author2) {
							continue;
						}
						global_co_authorship[make_pair(author1, author2)]++;
					}
				}
			}
			for(int author1 : macros[i].authors) {
				for(int author2 : macros[i].authors) {
					if(author1 >= author2) {
						continue;
					}
					macros[i].co_authorship[make_pair(author1, author2)] = global_co_authorship[make_pair(author1, author2)];
				}
			}
		}
		cout << macro_counter << " different macros used in  " << macros.size() << " comments" << endl;
		cout << "authors count: " << author_experience.size() << endl;
		cout << "paper count: "	 << paper_count << endl;
		cout << "Avg authors per paper: " << sum_authors_on_papers / (double) paper_count << endl;
		cout << "Avg diff name for one body:  " << body_name.size() / (double) macro_counter << endl;
	}
	int max_time_index = - 1;
    for(int i = 0; i < (int)macros.size(); i++) {
        word_bucket[macros[i].macro_number].push_back(macros[i]);
		int time_index = 2 * (macros[i].year - 1990) +  macros[i].month / 6;
		max_time_index = max(max_time_index, time_index);
		for(int author1 : macros[i].authors) { 
			for(int author2 : macros[i].authors)  {
				if(author1 >= author2) {
					continue;
				}
				current_experiences_on_time_windows[time_index].push_back(macros[i].co_authorship[make_pair(author1, author2)]);
			}
		}
    }
	for(int i = 0; i <= max_time_index; i++) {
		sort(current_experiences_on_time_windows[i].begin(), current_experiences_on_time_windows[i].end());
		cout << "current_experiences index and size: "<< i << " ------- " << current_experiences_on_time_windows[i].size() << endl;
	}
    macros.clear();
  	{ // read edges on BFS tree an depth only for interesting macros
		string s,t;
		int depth;
		int count[15], sum_final_experience[15];
		double sum_current_experience[15];

		int count_internal[15], sum_internal_final_experience[15];
		double sum_internal_current_experience[15];
		memset(count, 0, sizeof count);
		memset(sum_current_experience, 0, sizeof sum_current_experience);
		memset(sum_final_experience, 0, sizeof sum_final_experience);
		int macro_number, index;
		int num1, num2;
		string paper_id;
		{
			ifstream fin_tree_edges("RawOutput/TreeEdges.txt");
			getline(fin_tree_edges, s);
			while(fin_tree_edges >> s >> num1 >> t >> num2 >> depth >> macro_number >> index >> paper_id) {
				int author1 = author_to_num[s];
				int author2 = author_to_num[t];
				if(author1 > author2) {
					swap(author1, author2);
				}
				count[depth] ++;
				bfs_out_deg.insert(make_pair(s, macro_number));
				sum_final_experience[depth] += global_co_authorship[make_pair(author1, author2)];
				int current_experience = word_bucket[macro_number][index].co_authorship[make_pair(author1, author2)];
				int time_index = 2 * (word_bucket[macro_number][index].year - 1990) + word_bucket[macro_number][index].month / 6;
				int index = lower_bound(current_experiences_on_time_windows[time_index].begin(), current_experiences_on_time_windows[time_index].end(), current_experience)
					- current_experiences_on_time_windows[time_index].begin();
				//			cerr << "time index: " << time_index << " " << current_experiences_on_time_windows[time_index].size() << " " << current_experience << endl;
				sum_current_experience[depth] += index / (double) current_experiences_on_time_windows[time_index].size();

			}
			fin_tree_edges.close();
		}
	{
			ifstream fin_tree_edges("RawOutput/TreeEdges.txt");
			getline(fin_tree_edges, s);
			while(fin_tree_edges >> s >> num1 >> t >> num2 >> depth >> macro_number >> index >> paper_id) {
				int author1 = author_to_num[s];
				int author2 = author_to_num[t];
				if(author1 > author2) {
					swap(author1, author2);
				}
				if(bfs_out_deg.find(make_pair(s, macro_number)) != bfs_out_deg.end()) {
					count_internal[depth] ++;
					sum_internal_final_experience[depth] += global_co_authorship[make_pair(author1, author2)];
					int current_experience = word_bucket[macro_number][index].co_authorship[make_pair(author1, author2)];
					int time_index = 2 * (word_bucket[macro_number][index].year - 1990) + word_bucket[macro_number][index].month / 6;
					int index = lower_bound(current_experiences_on_time_windows[time_index].begin(), current_experiences_on_time_windows[time_index].end(), current_experience)
						- current_experiences_on_time_windows[time_index].begin();
				//			cerr << "time index: " << time_index << " " << current_experiences_on_time_windows[time_index].size() << " " << current_experience << endl;
					sum_internal_current_experience[depth] += index / (double) current_experiences_on_time_windows[time_index].size();
				}

			}
			fin_tree_edges.close();
		}
		ofstream fout_tree_edges("RawOutput/TreeLayerAvgFinalExperience.txt");
		fout_tree_edges << "Depth EdgeCount SumFinalExperience SumCurrentExperience AvgFinalExperience AvgCurrentExperience "
			<< "InternalAvgFinalExperience InternalAvgCurrentExperience" << endl;
		for(int i = 1; i < 12; i++) {
			fout_tree_edges << i << " " << count_internal[i] << " " << sum_final_experience[i] << " " << sum_current_experience[i] << " " 
				<< sum_final_experience[i] / (double)count_internal[i] << " " <<  sum_current_experience[i] / (double) count_internal[i] << " "
				<< sum_internal_final_experience[i] / (double)count_internal[i] << " " <<  sum_internal_current_experience[i] / (double) count_internal[i] << endl;
		}
	}
	
	for(int x : interesting_macros) {
        solve(x); 
		for(Macro macro : word_bucket[x]) {
			macros.push_back(macro);
		}
    }

	{ // here we build the global distribution of co_authorship on interesting macros
		for (int i = 0; i < (int) macro_counter; i++) {
			for(Macro macro : word_bucket[i]) {
				for(int author1 : macro.authors) {
					for(int author2 : macro.authors) {
						if(author1 >= author2) {
							continue;
						}
						global_co_authorship_distribution_current[macro.co_authorship[make_pair(author1, author2)]]++;
						global_co_authorship_distribution_final[global_co_authorship[make_pair(author1, author2)]]++;
						total_co_authorships++;
					}
				}
			}
		}
		{
			// find the cummalitive distributions for tree edges and all co-authorships for interesting macros -- Current
			int dag_edges_seen = 0, co_authorships_seen = 0; 
			ofstream fout_tie_strengh_global_vs_dag_current("RawOutput/TieStrengthGlobalVsDag_current.txt");
			fout_tie_strengh_global_vs_dag_current << "TieStrength AllCoAuthorships DagEdges" << endl;
			for(pair<int, int> p : global_co_authorship_distribution_current) {
				co_authorships_seen += p.second;
				dag_edges_seen += tree_co_authorship_distribution_current[p.first];
				fout_tie_strengh_global_vs_dag_current << p.first << " " << co_authorships_seen / (double) total_co_authorships << " " << dag_edges_seen / (double) total_edge_count << endl;
			}
		}
		{
			// find the cummalitive distributions for tree edges and all co-authorships for interesting macros -- Final
			int dag_edges_seen = 0, co_authorships_seen = 0; 
			ofstream fout_tie_strengh_global_vs_dag_final("RawOutput/TieStrengthGlobalVsDag_final.txt");
			fout_tie_strengh_global_vs_dag_final << "TieStrength AllCoAuthorships DagEdges" << endl;
			for(pair<int, int> p : global_co_authorship_distribution_final) {
				co_authorships_seen += p.second;
				dag_edges_seen += tree_co_authorship_distribution_final[p.first];
				fout_tie_strengh_global_vs_dag_final << p.first << " " << co_authorships_seen / (double) total_co_authorships << " " << dag_edges_seen / (double) total_edge_count << endl;
			}
		}
	}
	
	{ // look at random shuffled co-authroship and find a match for people in dag_edges
		vector<pair<pair<int, Macro>, int> > all_edges[40];
		for(Macro macro : macros) {
			for(int author1 : macro.authors) {
				for(int author2 : macro.authors) {
					if(author1 >= author2) {
						continue;
					}
					// ONLY LOOK AT 2005 // TODO
//					if(macro.year == 2005) {
						int rel_year = macro.year - BASE_YEAR;
						int co_authorship_exp_current = macro.co_authorship[make_pair(min(author1, author2), max(author1, author2))];
						int co_authorship_exp_final = global_co_authorship[make_pair(min(author1, author2), max(author1, author2))];
						if(co_authorship_exp_current == 1) { // current includes the on in this macro
							all_edges[rel_year-(rel_year%2)].push_back(make_pair(make_pair(co_authorship_exp_current, macro), co_authorship_exp_final));
							if(macro.year == 2006 || macro.year == 2007) {
								cerr << "All-edges: " << rev_author_to_num[author1] << " " << rev_author_to_num[author2] << " " 
									<< co_authorship_exp_current << " " << co_authorship_exp_final << endl;
							}
						}
//					}
				}
			}
		}
		for(int i = 0; i < 20; i++) {
			sort(all_edges[i].begin(), all_edges[i].end(), cmp); // cmp only needs to compare the first in the pair which is a pair itself - second is final_exp
			sort(dag_edges[i].begin(), dag_edges[i].end(), cmp);
			sort(internal_dag_edges[i].begin(), internal_dag_edges[i].end(), cmp);
		}
		{
			ofstream fout_dag_vs_all_edges("RawOutput/TerminalDagVsAllEdgePrediction.txt");
			fout_dag_vs_all_edges << "Year SuccessRateOnDag deltay" << endl;
			for(int rel_year = 0; rel_year < 20; rel_year++) {
				int p = 0, q = 0;
				int all_has_higher_final = 0, dag_has_higher_final = 0, equal_final = 0;
				int all_is_earlier = 0, dag_is_earlier = 0, equal_date = 0;
				while(p < (int)all_edges[rel_year].size() && q < (int) dag_edges[rel_year].size()) {
					if(all_edges[rel_year][p].first.first == dag_edges[rel_year][q].first.first &&
							MacroDifferenceInMonth(all_edges[rel_year][p].first.second, dag_edges[rel_year][q].first.second) < 1) {
						// match found!
						if(all_edges[rel_year][p].first.second.getTime() < dag_edges[rel_year][q].first.second.getTime()) {
							all_is_earlier++;	
						} else if (dag_edges[rel_year][q].first.second.getTime() < all_edges[rel_year][p].first.second.getTime()) {
							dag_is_earlier++;
						} else {
							equal_date++;
						}

						if(dag_edges[rel_year][q].second > all_edges[rel_year][p].second) {
							dag_has_higher_final++;
						} else if(dag_edges[rel_year][q].second < all_edges[rel_year][p].second){
							all_has_higher_final++;
						} else {
							equal_final++;
						}
						p++; q++;
						continue;
					}

					if (all_edges[rel_year][p] < dag_edges[rel_year][q]) {
						p++;
					} else if(dag_edges[rel_year][q] < all_edges[rel_year][p]) {
						q++;
					}
				}
				double denom = dag_has_higher_final + all_has_higher_final;
				double frac = dag_has_higher_final / denom;
				fout_dag_vs_all_edges << rel_year + BASE_YEAR << " " << 100 * frac << " " << 100 * sqrt((frac * (1-frac))/(denom)) << endl; 
			}
		}
		{
			ofstream fout_dag_vs_all_edges("RawOutput/InternalDagVsAllEdgePrediction.txt");
			fout_dag_vs_all_edges << "Year SuccessRateOnInternalDag deltay" << endl;
			for(int rel_year = 0; rel_year < 20; rel_year++) {
				int p = 0, q = 0;
				int all_has_higher_final = 0, dag_has_higher_final = 0, equal_final = 0;
				int all_is_earlier = 0, dag_is_earlier = 0, equal_date = 0;
				while(p < (int)all_edges[rel_year].size() && q < (int) internal_dag_edges[rel_year].size()) {
					if(all_edges[rel_year][p].first.first == internal_dag_edges[rel_year][q].first.first &&
							MacroDifferenceInMonth(all_edges[rel_year][p].first.second, internal_dag_edges[rel_year][q].first.second) < 1) {
						// match found!
						if(all_edges[rel_year][p].first.second.getTime() < internal_dag_edges[rel_year][q].first.second.getTime()) {
							all_is_earlier++;	
						} else if (internal_dag_edges[rel_year][q].first.second.getTime() < all_edges[rel_year][p].first.second.getTime()) {
							dag_is_earlier++;
						} else {
							equal_date++;
						}
						if(internal_dag_edges[rel_year][q].second > all_edges[rel_year][p].second) {
							dag_has_higher_final++;
						} else if(internal_dag_edges[rel_year][q].second < all_edges[rel_year][p].second){
							all_has_higher_final++;
						} else {
							equal_final++;
						}
						p++; q++;
						continue;
					}

					if (all_edges[rel_year][p] < internal_dag_edges[rel_year][q]) {
						p++;
					} else if(internal_dag_edges[rel_year][q] < all_edges[rel_year][p]) {
						q++;
					}
				}
				double denom = dag_has_higher_final + all_has_higher_final;
				double frac = dag_has_higher_final / denom;
				fout_dag_vs_all_edges << rel_year + BASE_YEAR << " " << 100 * frac << " " << 100 * sqrt((frac * (1-frac))/(denom)) << endl; 
			}
		}
		{
			ofstream fout_dag_vs_all_edges("RawOutput/InternalDagVsTerminalDag.txt");
			fout_dag_vs_all_edges << "Year SuccessRateOnInternalDag deltay" << endl;
			for(int rel_year = 0; rel_year < 20; rel_year++) {
				int p = 0, q = 0;
				int internal_has_higher_final = 0, dag_has_higher_final = 0, equal_final = 0;
				int internal_is_earlier = 0, dag_is_earlier = 0, equal_date = 0;
				while(p < (int)dag_edges[rel_year].size() && q < (int) internal_dag_edges[rel_year].size()) {
					if(dag_edges[rel_year][p].first.first == internal_dag_edges[rel_year][q].first.first 
							&& MacroDifferenceInMonth(dag_edges[rel_year][p].first.second, internal_dag_edges[rel_year][q].first.second) < 1) {
						// match found!
						if(dag_edges[rel_year][p].first.second.getTime() < internal_dag_edges[rel_year][q].first.second.getTime()) {
							dag_is_earlier++;	
						} else if (internal_dag_edges[rel_year][q].first.second.getTime() < dag_edges[rel_year][p].first.second.getTime()) {
							internal_is_earlier++;
						} else {
							equal_date++;
						}
						if(internal_dag_edges[rel_year][q].second > dag_edges[rel_year][p].second) {
							internal_has_higher_final++;
						} else if(internal_dag_edges[rel_year][q].second < all_edges[rel_year][p].second){
							dag_has_higher_final++;
						} else {
							equal_final++;
						}
						p++; q++;
						continue;
					}

					if (dag_edges[rel_year][p] < internal_dag_edges[rel_year][q]) {
						p++;
					} else if(internal_dag_edges[rel_year][q] < dag_edges[rel_year][p]) {
						q++;
					}
				}
				double denom = (dag_has_higher_final + internal_has_higher_final);
				double frac = internal_has_higher_final / denom;
				fout_dag_vs_all_edges << rel_year + BASE_YEAR << " " << 100 * frac << " " << 100 * sqrt((frac * (1-frac))/(denom)) << endl; 
			}
		}
	}


	return 0;
} 

