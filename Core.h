#ifndef CORE_H
#define CORE_H

#include<utility>
#include<string>
#include<set>
#include"Macro.h"

#define K_2K_DEF true
#define SOLVE_DEF true
#define AUTHOR_HEAPS_LAW_DEF true
#define MACRO_HEAPS_LAW_DEF true
#define LONGEVITY_DEF true

extern ofstream fout_experience_changed_name;
extern ofstream fout_heaps_law;
extern ofstream fout_author_heaps;
extern ofstream fout_learning;
extern ofstream fout_learning_unique_paper;
extern ofstream fout_learning_unique_authorPair;
extern ofstream fout_N_becomes_1;

extern const std::string TYPE;
extern const std::string SMART; // options are _nosmart or _smart
extern const bool has_skipped;
extern const int H;
extern const int N_TO_1;
extern const int MACRO_LENGTH_FILTER;
extern const int MACRO_PAPER_USAGE_FILTER;

extern map<string, int> macro_to_num;
extern map<string, int> author_to_num;
extern map<int, int> local_author_id;
extern map<int, int> rev_local_auhtor_id;
extern set<pair<int, int> > unique_authorPair;
extern map<int, int> macro_paper_count; // value = number of papers used macro! = word_bucket[macro].size()
extern set<string> unique_paper;
extern vector<int> person_path[1000 * 1000];
extern int person_pointer[1000 * 1000];
extern int change[1000 * 1000];
extern set<int> adj_list[1000 * 1000];

extern vector<Macro> macros;
extern set<string> important_macros;
extern vector<Macro> word_bucket[3000*1000];
extern map<int, int> author_experience; // at the end of the code it will be final experience but while running it's the current_exp
extern int num_above_cut;
extern int data_points;
extern int macro_counter;


std::string SimpleIntToString(int);
std::string RemoveSpaces(std::string);
void preprocess();
void initialize(int, int);
bool ExactSame(Macro, Macro);
bool GoodMacro(Macro, int, int);

#endif
