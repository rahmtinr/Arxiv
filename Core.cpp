#include<iostream>
#include<cstring>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
#include"Macro.h"
#include"Core.h"

using namespace std;
ofstream fout_experience_changed_name;
ofstream fout_heaps_law;
ofstream fout_author_heaps;
ofstream fout_learning;
ofstream fout_learning_unique_paper;
ofstream fout_learning_unique_authorPair;
ofstream fout_N_becomes_1;

const std::string TYPE = "";
const std::string SMART = "_nosmart"; // options are _nosmart or _smart
const bool has_skipped = false;
const int H = 1000 * 1000;
const int N_TO_1 = 2;
const int MACRO_LENGTH_FILTER = 20;
const int MACRO_PAPER_USAGE_FILTER = 30;

map<string, int> macro_to_num;
map<string, int> author_to_num;
map<int, int> local_author_id;
map<int, int> rev_local_auhtor_id;
set<pair<int, int> > unique_authorPair;
map<int, int> macro_paper_count; // value = number of papers used macro! = word_bucket[macro].size()
set<string> unique_paper;
vector<int> person_path[1000 * 1000];
int person_pointer[1000 * 1000];
int change[1000 * 1000];
set<int> adj_list[1000 * 1000];
string folder;

vector<Macro> macros;
set<string> important_macros;
vector<Macro> word_bucket[3000*1000];
map<int, int> author_experience; // at the end of the code it will be final experience but while running it's the current_exp
int num_above_cut = 0;
int data_points = 0;
int macro_counter = 1;


bool GoodMacro(Macro macro, int count, int len) {
    if(count < MACRO_PAPER_USAGE_FILTER || (len < MACRO_LENGTH_FILTER && folder == "/RawOutput/Name") ||
			(len > 50 && folder == "/RawOutput/Body") ) { // macro needs to be used and should have a length
        return false;
    }
    return true;
}

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


void preprocess() {
    map<pair<int, int>, int> author_macro_co_author; // value = number of co-authors of author(key.first) that used macro(key.second) 
    set<pair<pair<int, int>, int> > co_authors_macro; // did co authors meet and before that second author used the macro
    set<pair<int, int>> author_macro; // did author use macro?
    map<int, vector<int> > author_macros; // value = macros that author has used
    cerr << "Macros.size() = " << macros.size() << endl;
    for(int i = 0; i <(int) macros.size(); i++) {
        if(i % 100000 == 0 ) {
            cerr << i / 100000 << endl;;
        }
        Macro macro = macros[i];
        int macro_id = macro.macro_number;
        if(GoodMacro(macro, macro_paper_count[macro_id], rev_macro_to_num[macro_id].length())) { // macro needs to be used and should have a length
            for(int j = 0; j < (int)macro.authors.size(); j++) {
                int author1 = macro.authors[j];
                for(int k = 0; k < (int) macro.authors.size(); k++) {
                    if(j == k) {
                        continue;
                    }
                    int author2 = macro.authors[k];
                    if(author_macro.find(make_pair(author2, macro.macro_number)) != author_macro.end()) {
                        if(co_authors_macro.find(make_pair(make_pair(author1, author2), macro_id)) == co_authors_macro.end()) {
                            author_macro_co_author[make_pair(author1, macro_id)]++;
                            co_authors_macro.insert(make_pair(make_pair(author1, author2), macro_id));
                        }
                    }
                }
                macros[i].co_authors_count_used_previously.push_back(author_macro_co_author[make_pair(author1, macro_id)]);
            }
        }
        for(int author1 : macro.authors) {
            if(author_macro.find(make_pair(author1, macro_id)) == author_macro.end()) {
                author_macro.insert(make_pair(author1, macro_id));
                vector<int> temp = author_macros[author1];
                temp.push_back(macro_id);
                author_macros[author1] = temp;
            }
        }

        if(i == (int)macros.size() - 1 || ExactSame(macros[i], macros[i + 1]) == false) {
            for(int j = 0; j < (int)macros[i].authors.size(); j++) {
                int author1 = macros[i].authors[j];
                for(int k = 0; k < (int) macros[i].authors.size(); k++) {
                    if(j == k) {
                        continue;
                    }
                    int author2 = macros[i].authors[k];
                    if(author_macros.find(author2) != author_macros.end()) {
                        vector<int> author2_macros = author_macros[author2];
                        for(int macro_id : author2_macros) {
                            if(co_authors_macro.find(make_pair(make_pair(author1, author2), macro_id)) == co_authors_macro.end()) {
                                author_macro_co_author[make_pair(author1, macro_id)]++;
                                co_authors_macro.insert(make_pair(make_pair(author1, author2), macro_id));
                            }
                        }
                    }
                }
            }
        }
    }
    cerr << "for loop finished gonna start clearing" << endl;
    cerr << author_macro_co_author.size() << endl; author_macro_co_author.clear(); cerr << " 1" << endl;
    cerr << co_authors_macro.size() << endl; co_authors_macro.clear(); cerr << "2 " << endl;
    cerr << author_macro.size() << endl; author_macro.clear(); cerr << "3" << endl;
    cerr << author_macros.size() << endl; author_macros.clear(); cerr <<"4" << endl;
    cerr << "Finishing preprocessing" << endl;
}

void initialize(int& local_author_count, int x) {
    local_author_id.clear();
    rev_local_auhtor_id.clear();
    for(Macro macro : word_bucket[x]) {
        for(int author: macro.authors) {
            if(local_author_id.find(author) == local_author_id.end()) {
                local_author_id.insert(make_pair(author, local_author_count));
                rev_local_auhtor_id.insert(make_pair(local_author_count, author));
                local_author_count++;
            }
        }
    }

    for(int i = 0; i < local_author_count; i++) {
        person_path[i].clear();
        person_pointer[i] = 0;
        change[i] = 0;
        adj_list[i].clear();
    }

    for(int i = 0; i < (int)word_bucket[x].size(); i++) {
        for(int author : word_bucket[x][i].authors) {
            person_path[local_author_id[author]].push_back(i);
        }
    }
}

