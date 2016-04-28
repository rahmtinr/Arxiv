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


ofstream fout_experience_changed_name;
ofstream fout_heaps_law;
ofstream fout_author_heaps;
ofstream fout_learning;
ofstream fout_learning_unique_paper;
ofstream fout_learning_unique_authorPair;
ofstream fout_N_becomes_1;

map<string, int> macro_to_num;
map<int, string> rev_macro_to_num;
map<string, int> author_to_num;
map<int, string> rev_author_to_num;
map<int, int> local_author_id;
map<int, int> rev_local_auhtor_id;
set<pair<int, int> > unique_authorPair;
map<int, int> macro_paper_count; // value = number of papers used macro! = word_bucket[macro].size()
set<string> unique_paper;
vector<int> person_path[1000 * 1000];
int person_pointer[1000 * 1000];
int change[1000 * 1000];
set<int> adj_list[1000 * 1000];


const bool has_skipped = false;
const int H = 1000 * 1000;
const int N_TO_1 = 2;
#define K_2K_DEF false
#define SOLVE_DEF true
#define HEAPS_LAW_DEF false
#define LONGEVITY_DEF false
const string TYPE = "";
const string SMART = "_nosmart"; // options are _nosmart or _smart


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
    string name;
    string paper_id;
    vector<int> authors;
    vector<int> experience;
    vector<int> co_authors_count_used_previously;
    vector<string> categories;
    int day, month, year;
    int count;


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


struct OutputElement {
    string output_string;
    int authors_count;
    bool operator < (const OutputElement &other) const {
        return authors_count < other.authors_count;
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
set<string> important_macros;
vector<Macro> word_bucket[3000*1000]; 
map<int, int> author_experience; // at the end of the code it will be final experience but while running it's the current_exp
vector<OutputElement> k_2k[10];
int num_above_cut = 0;
int data_points = 0;
int macro_counter = 1;

bool GoodMacro(Macro macro) { 
    if(macro_paper_count[macro.macro_number] < Macro_paper_usage || rev_macro_to_num[macro.macro_number].length() < Macro_length_filter) { // macro needs to be used and should have a length
        return false;
    }
    return true;
}

pair<double, double> ClusteringCoeff(int n) { 
    double global_num = 0, global_denom = 0;
    double local_coeff = 0 ;
    for(int i = 1; i < n; i++) {
        double local_num = 0, local_denom = 0;
        for(int x: adj_list[i]) {
             for(int y : adj_list[i]) {
                 if(x >= y) {
                     continue;
                 }
                 global_denom++;
                 local_denom++;
                 if(adj_list[x].find(y) != adj_list[x].end()) {
                    global_num++;
                    local_num++;
                 }
             }
        }
        if(local_denom == 0) {
            local_denom = 1;
        }
        local_coeff += local_num / local_denom;
    }
    if(n == 1) { 
        n++;
    }
    if(global_denom == 0) {
        global_denom++;
    }
    return make_pair(local_coeff / (n - 1), (global_num / global_denom)); 

}

void initialize(int &local_author_count, int x) {
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

#if K_2K
ofstream fout_k_2k[10];
ofstream fout_k_2k_thresholds("RawOutput/k_2k_thresholds.txt");
void prediction_k_2k(int x) {
    if(!GoodMacro(word_bucket[x][0]) { // macro needs to be used and should have a length
        return;
    }
 
    int local_author_count = 1;
    initialize(local_author_count, x);
//  Prediction from k to 2k
    set<int> local_unique_authors;
    map<int, int> author_count_to_index;
    map<int, int> author_count_to_median_index;
    int threshold = 5;
    for(int i = 0 ; i <(int) word_bucket[x].size(); i++) {
        if(word_bucket[x][i].authors.size() > 20) { // REMOVE THIS IF YOU WANT ALL THE DATA FOR THE PREDICTION <--------------------------------------
            // Only macros that go over k without a paper with 20 authors
            break;
        }
        for(int j = 0; j < (int)word_bucket[x][i].authors.size(); j++) {
            local_unique_authors.insert(word_bucket[x][i].authors[j]);
        }
        while((int)local_unique_authors.size() >= threshold) {
            author_count_to_index[threshold] = i + 1;
            threshold++;
        }
    }
    int index_bef = 0;
    for(int out_counter = 1; out_counter < 10; out_counter++) {
        int threshold =  5 * ( 1 << (out_counter - 1));

        if(author_count_to_index.find(threshold) == author_count_to_index.end()) { 
            break;
        }
        int index = author_count_to_index[threshold];
        vector<int> global, local;
        double global_sum = 0, local_sum = 0;
        local_unique_authors.clear();
        for(int i = 0; i < index; i++) {
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
        sort(global.begin(), global.end());
        sort(local.begin(), local.end());
        OutputElement output_element;
        int eps = 1e-7;
        output_element.output_string = to_string(word_bucket[x][0].authors.size()) + ", " + to_string(index) + ", " + to_string(global[global.size() / 2]) + ", " + to_string(global_sum / global.size()) + ", ";
        output_element.output_string += to_string(local_global.first + eps) + ", " + to_string(local_global.second + eps) + ", ";
        output_element.authors_count = local_author_count;
        k_2k[out_counter].push_back(output_element);
    }
}
#endif


#if SOLVE_DEF
bool solve(int x) {
    if(!GoodMacro(word_bucket[x][0])) { // macro needs to be used and should have a length
        return false;
    }
    int local_author_count = 1;
    initialize(local_author_count, x);
    if(TYPE == "") { 
        if(local_author_count < 20) {
            return false;
        }
    } else if( TYPE == "-narrow") {
        if (local_author_count > 250 || local_author_count < 20) {
            return false;
        }
    } else if( TYPE == "-wide" ) {
        if(local_author_count < 250) {
            return false;
        }
    } else {
        return false;
    }


// Heaps law and name change 
   for(int i = 1 ; i < local_author_count; i++) {
        vector<string> names;
        set<string> names_set;
        string learned = word_bucket[x][0].name;
        int counter = 1;
        for(int j = 0; j < (int)person_path[i].size(); j++) {
            int index = person_path[i][j];
            names.push_back(word_bucket[x][index].name);
            bool change = false, new_name = false;
            if(j > 0 && names[j] != names[j - 1]) {
                change = true;
                counter++;
            }
            if(names_set.find(names[j]) == names_set.end()) { 
                new_name = true;
                names_set.insert(names[j]);
            }
            for(int k = 0; k < (int)word_bucket[x][index].authors.size(); k++) {
                int author = word_bucket[x][index].authors[k];
                if(author == rev_local_auhtor_id[i]) { // each person for each paper that uses the macro we print one line! 
                    fout_experience_changed_name << author_experience[author] << " " << person_path[i].size() << " " << word_bucket[x][index].experience[k] << " " << j << " " << change << " " << new_name << endl; 
                }
            }
        }
        fout_heaps_law << person_path[i].size() << " " << names_set.size() << " " << counter << endl; // each (person, macro) pair gets one line! 
    }
    cerr << "starting: " << x << endl; 
//  Most authoratitive author prediction
    for(int i = 0; i < (int)word_bucket[x].size(); i++) {
        if(x == 29297) {
            cerr <<  "### " << i << " " << word_bucket[x].size() << endl;
        }
        int winner = -1;
        Macro macro = word_bucket[x][i];
        int bound = min(N_TO_1, (int)macro.authors.size());
        bool check = true;
        int count_equal_names = 0;
        int null_names = 0;
        int min_index = 0, max_index = 0, most_recent_index = 1;
        vector<Macro> macros_used;
        macros_used.clear();
        macros_used.push_back(macro);
        for(int j = 0; j < bound && check == true; j++) {
            if(x == 29297) {
                cerr <<  "$$$$ " << j << endl;
            }
            int author_j = local_author_id[macro.authors[j]];
            if(person_pointer[author_j] <= person_pointer[local_author_id[macro.authors[min_index]]]) {
                min_index = j;
            }
            if(person_pointer[author_j] >= person_pointer[local_author_id[macro.authors[max_index]]]) {
                max_index = j;
            }
            Macro macro_j;
            if(person_pointer[author_j] > 0) {
                macro_j = word_bucket[x][person_path[author_j][person_pointer[author_j] - 1]];
                macros_used.push_back(macro_j);
            } else {
                null_names++;
                macro_j.name = macro.name;
                macros_used.push_back(macro);
            }
            for(int k = 0; k < j; k++) {
                int author_k = local_author_id[macro.authors[k]];
                Macro macro_k;
                if(person_pointer[author_k] == 0) {
                    macro_k.name = macro.name;
                } else {
                    macro_k = word_bucket[x][person_path[author_k][person_pointer[author_k] - 1]];
                }
                if(macro_k.name == macro_j.name) {
                    check = false;
                    break;
                }
            }
            if(macro.name == macro_j.name) {
                count_equal_names++;
                winner = j + 1;
            }
        }
        cerr << " salam" << endl;
        bool same_date = false;
        if(check == true && count_equal_names == 1 && macro.authors.size() == N_TO_1 && (null_names == 0 || (null_names == 1 && SMART == "_smart" && macro.authors.size() == N_TO_1))) {
            preprocess(x, word_bucket[x][i]);
            if(null_names == 1) {
                cerr <<" SHOUT SHOUT! There is a problem" << endl;
            }
            cerr << "starting the instace" << endl;
            for(int j = 2; j <= bound; j++) {
                if(macros_used[j] == macros_used[most_recent_index]) { // only checks the date
                    continue;
                }
                if(macros_used[j] < macros_used[most_recent_index]) {
                    most_recent_index = j;
                }
            }
            most_recent_index--;
            for(int j = 1; j <= N_TO_1; j++) { 
                if(macros_used[0] == macros_used[j]) {
                    same_date = true;
                    break;
                }
            }
            cerr << "Got here in the instance" << endl;
            if(same_date == false) {
                fout_N_becomes_1 << RemoveSpaces(rev_macro_to_num[macro.macro_number]) << ": ";
                for(int k = 0; k < (int)macros_used.size(); k++) {
                    fout_N_becomes_1 << macros_used[k].name << " ";
                }
                fout_N_becomes_1 << macro.paper_id << " ";
                for(int k = 1; k < (int)macros_used.size(); k++) {
                    string author_k = rev_author_to_num[macros_used[0].authors[k-1]];
                    fout_N_becomes_1 << RemoveSpaces(author_k) << " " << macros_used[k].paper_id << " " ;
                }
                fout_N_becomes_1 << endl;
                for(int k = 0; k < N_TO_1; k++) {
                    int is_max = 0;
                    int is_min = 0;
                    int is_most_recent = 0;
                    if(min_index == k) {
                        is_min = 1;
                    }
                    if(max_index == k) {
                        is_max = 1;
                    }
                    if(most_recent_index == k) {
                        is_most_recent = 1;
                    }
                    int local_author = local_author_id[macro.authors[k]]; 
                    double denom = (double)(max(1, person_pointer[local_author] - 1));
                    fout_learning << macro.experience[k] << ", " << person_pointer[local_author] << ", " << change[local_author] / denom << ", "; 
                    fout_learning << is_max << ", " << is_min << ", " << is_most_recent << ", ";
                    cerr << "printing" << endl;
                    cerr << macro.co_authors_count_used_previously.size() << " " << macros_used.size() << endl;
                    if(N_TO_1 == 2 && unique_authorPair.find(make_pair(macro.authors[0], macro.authors[1])) == unique_authorPair.end()) {
                        fout_learning_unique_authorPair << macro.experience[k] << ", " << person_pointer[local_author] << ", " << change[local_author] / denom << ", "; 
                        fout_learning_unique_authorPair << is_max << ", " << is_min << ", " << is_most_recent << ", " << macros_used[k + 1].name.length() << ", ";
                        fout_learning_unique_authorPair << macro.co_authors_count_used_previously[k] << ", ";
                    }
                    if(N_TO_1 == 2 && unique_paper.find(macro.paper_id) == unique_paper.end()) {
                        fout_learning_unique_paper << macro.experience[k] << ", " << person_pointer[local_author] << ", " << change[local_author] / denom << ", "; 
                        fout_learning_unique_paper << is_max << ", " << is_min << ", " << is_most_recent << ", " << macros_used[k + 1].name.length() << ", ";
                        fout_learning_unique_paper << macro.co_authors_count_used_previously[k] << ", ";
                    }
                    cerr << "finished printing" << endl;
                }
                fout_learning << winner << endl; 
                if(N_TO_1 == 2 && unique_authorPair.find(make_pair(macro.authors[0], macro.authors[1])) == unique_authorPair.end()) {
                    fout_learning_unique_authorPair << winner << endl;  
                }

                if(N_TO_1 == 2 && unique_paper.find(macro.paper_id) == unique_paper.end()) {
                    fout_learning_unique_paper << winner << endl; 
                }
                unique_authorPair.insert(make_pair(macro.authors[0], macro.authors[1]));  
                unique_paper.insert(macro.paper_id);
                data_points++;
            }
        }
        for(int author : macro.authors) {
            int local_author = local_author_id[author];
            person_pointer[local_author]++;
            if(person_pointer[local_author] == (int)person_path[local_author].size()) {
                continue;
            }
            if(person_pointer[local_author] > 1) {
                int index_cur = person_path[local_author][person_pointer[local_author] - 1];
                int index_bef = person_path[local_author][person_pointer[local_author] - 2];
                if(word_bucket[x][index_cur].name != word_bucket[x][index_bef].name) {
                    change[local_author]++;
                }
            }
        }
    }
    cerr << "Finishing: " << x << endl;
    return true;
}
#endif

#if HEAPS_LAW_DEF
void HeapsLawForAPerson() {
    set<int> authors_with_this_macro;
    map<int, int> author_num, author_denom;
    for(int i = 0; i <(int) macro_counter; i++) {
        for(int j = 0; j < (int) word_bucket[i].size(); j++) {
            for(int author : word_bucket[i][j].authors) {
                authors_with_this_macro.insert(author);
                author_denom[author]++;
            }
        }
        for(int author : authors_with_this_macro) {
            author_num[author]++;
        }
        authors_with_this_macro.clear();
    }
    

    bool check = false;
    ofstream fout_sample_author_macros("RawOutput/SampleAuthorMacroUsage.txt");
    for(pair<int, int> kv : author_num) {
        int macros_used_by_author = author_denom[kv.first];
        int uniq_macros_used_by_author = kv.second;
        fout_author_heaps << macros_used_by_author << " " << uniq_macros_used_by_author<< endl;
        // kv.second should be sqrt(author_denom[kv.first])
        if(macros_used_by_author == 200 && check == false && uniq_macros_used_by_author > 120) {
            check = true;
            fout_sample_author_macros << "The author and his final global macro exp is: " << rev_author_to_num[kv.first] << " " << author_experience[kv.first]<< endl;
            for(int i = 0; i <(int) macro_counter; i++) {
                for(int j = 0; j < (int) word_bucket[i].size(); j++) {
                    for(int k = 0; k < (int) word_bucket[i][j].authors.size(); k++) {
                        if(word_bucket[i][j].authors[k] == kv.first) {
                            fout_sample_author_macros << word_bucket[i][j].experience[k] << " " << rev_macro_to_num[i] << endl;
                        }
                    }
                }
            }
        }
    }
}
#endif

#if LONGEVITY_DEF
ofstream fout_author_fitness[11];
void AuthorFitness() {
    int window = 5;
    for(int i = 1; i <= 10; i++) {
        fout_author_fitness[i].open("RawOutput/author_fitness_" + to_string(i * window)  + ".txt");
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
        if(!(word_bucket[macros[i].macro_number].size() < Macro_paper_usage || rev_macro_to_num[macros[i].macro_number].length() < Macro_length_filter)) { // macro needs to be used and should have a length
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
        if(GoodMacro(macro)) { // macro needs to be used and should have a length
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

int main() {
    if(SMART != "_smart" && SMART != "_nosmart") {
        cerr << " THERE IS A PROBLEM WITH THE SMART const variable" << endl;
        return 0;
    }
    ifstream fin("All_Arxiv_Macros.txt");
    {
        ifstream fin_important_macros("Macros_with_length_above_4.txt");
        string s;
        while(getline(fin_important_macros, s)) {
            important_macros.insert(s);
        }
    }

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
            string t;
            getline(fin, s);
            stringstream ss(s);
            s.clear();
            ss >> s;
            while(ss >> s) {
                macro.categories.push_back(s);   
            }

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
            if(macro.authors.size() < 50 && temp != "") { 
                macros.push_back(macro); 
            }
        }
        getline(fin, s); // you have two empty lines
    }
    cerr << "------> " << skipped << endl;
    sort(macros.begin(), macros.end());
    for(int i = 0; i < (int) macros.size(); i++) {
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
    }
    cerr << " $$$$$ " << endl;
#if SOLVE_DEF
    preprocess();
#endif
    for(int i = 0; i < (int)macros.size(); i++) {
        word_bucket[macros[i].macro_number].push_back(macros[i]);
    }

#if LONGEVITY_DEF
    cerr << "Longevity started" << endl;
    //macros should not be cleared!
    AuthorFitness();
    cerr << "END" << endl;
#endif

    macros.clear();

#if SOLVE_DEF

    fout_experience_changed_name.open("RawOutput/Experience_changed_name" + TYPE + ".txt");
    fout_heaps_law.open("RawOutput/heaps_law" + TYPE + ".txt");
    // Global is number of papers before, Local is number of times used this macro, has changed is 1 if different name, Completely new if first time name has came up

    fout_learning.open("RawOutput/" + to_string(N_TO_1) + "_to_1_learning" + TYPE + SMART + ".txt");
    fout_learning_unique_paper.open("RawOutput/" + to_string(N_TO_1) + "_to_1_learning" + TYPE + "-unique_paper" + SMART + ".txt");
    fout_learning_unique_authorPair.open("RawOutput/" + to_string(N_TO_1) + "_to_1_learning" + TYPE + "-unique_authorPair" + SMART + ".txt");
    fout_N_becomes_1.open("RawOutput/" + to_string(N_TO_1) + "_to_1_samples" + TYPE + SMART + ".txt");
    fout_N_becomes_1 << "MacroBody FinalName ";
    for(int i = 1; i <= N_TO_1; i++) {
        fout_N_becomes_1 << "Author" + to_string(i) + "MacroName ";
    }
    fout_N_becomes_1 << "FinalPaperId ";
    for(int i = 1; i <= N_TO_1; i++) {
        fout_N_becomes_1 << "Author" + to_string(i) + "Name PrevPaperAuthor" + to_string(i) + " ";
    }
    fout_N_becomes_1 << endl;
    fout_experience_changed_name << "FinalGlobalExperience FinalLocalExperience GlobalCurrentExperience LocalCurrentExperience HasChanged CompletelyNew" << endl;
    fout_experience_changed_name << "FinalGlobalExperience FinalLocalExperience GlobalCurrentExperience LocalCurrentExperience HasChanged CompletelyNew" << endl;
    fout_heaps_law << "FinalLocalExperience Types Token" << endl;
    for(int i = 1; i <= N_TO_1; i++) {
        string temp = to_string(i);
        fout_learning << "GlobalCurExp" + temp + ", LocalCurExp" + temp + ", LocalFracChange" + temp + ", IsMax" + temp +", IsMin" + temp + ", " + "IsMostRecent" + temp + ", "; 
        fout_learning_unique_paper << "GlobalCurExp" + temp + ", LocalCurExp" + temp + ", LocalFracChange" + temp + ", IsMax" + temp +", IsMin" + temp + ", " + "IsMostRecent" + temp + ", "; 
        fout_learning_unique_authorPair << "GlobalCurExp" + temp + ", LocalCurExp" + temp + ", LocalFracChange" + temp + ", IsMax" + temp +", IsMin" + temp + ", " + "IsMostRecent" + temp + ", " + "MacroLength" + temp + ", " + "CoAuthorCountUsedMacro" + temp + ", "; 
    }
    fout_learning << "Label" << endl;
    fout_learning_unique_paper << "Label" << endl;
    fout_learning_unique_authorPair << "Label" << endl;
    cerr << "count of unique macros: " << macro_counter << endl;
    for(int i = 1; i < (int)macro_counter; i++) {
       if( i % 10000 == 0 ) {
           cerr << i << endl;
       }
       solve(i); 
    }
    cerr << "ending solve" << endl;
#endif

#if HEAPS_LAW_DEF
    // HEAPS LAW EACH PERSON BEING A BOOK
    fout_author_heaps.open("RawOutput/author_heaps.txt");
    fout_author_heaps << "Macros UniqueMacros" << endl;
    HeapsLawForAPerson();
#endif

#if K_2K_DEF
    for(int i = 1; i < 10; i++) {
        fout_k_2k[i].open("RawOutput/k_2k_prediction_" + to_string(5 * (1 << (i - 1)))  + ".txt");
        fout_k_2k[i] << "NumberofAuthorsOnFirstPaper, NumberOfPapers, GlobalMedian, GlobalMean, LocalClustering, GlobalClustering, Label" << endl;

    }
    cerr << "macro counter: " << macro_counter << endl;
    for(int i = 1; i < (int)macro_counter; i++) {
        if(i % 1000 == 0) {
            cerr << "$$$$ " << i << endl;
        }
        prediction_k_2k(i);
    }
    for(int i = 1 ; i < 10; i++) {
        sort(k_2k[i].begin(), k_2k[i].end());
        int threshold = k_2k[i][k_2k[i].size() / 2].authors_count;
        for(int j = 0; j < (int)k_2k[i].size(); j++) {
            fout_k_2k[i] << k_2k[i][j].output_string;
            if(k_2k[i][j].authors_count <= threshold) {
                fout_k_2k[i] << 0 << endl;
            } else { 
                fout_k_2k[i] << 1 << endl;
            }
        }
        fout_k_2k_thresholds << 5 * (1 << (i - 1)) << " " << threshold << " " << k_2k[i].size() << endl;
    }
#endif


    cerr << "-------------------------------------->" << data_points << endl;


    return 0;
} 

