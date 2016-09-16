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


vector<int> interesting_macros;

bool has_skipped = false;
string skipped_string = "";
//ofstream feynman_fout("Feynman_skipped.txt");
//ofstream feynman_fout("Feynman.txt");

ofstream feynman_fout("RawOutput/Feynman_summary" + skipped_string + ".txt");
ofstream fout_frac_length("RawOutput/Biggest_component_MacroLength" + skipped_string + ".txt");
ofstream fout_biggest_second_biggest("RawOutput/Biggest_SecondBiggest" + skipped_string + ".txt");
ofstream fout_heat_map("RawOutput/MacroLength_AllAuthors_Biggest" + skipped_string + ".txt");
ofstream fout_stats("RawOutput/Stats" + skipped_string + ".txt");
ofstream fout_exp_difference("RawOutput/AllExpDifference" + skipped_string + ".txt");
ofstream fout_max_path("RawOutput/MaxPath" + skipped_string + ".txt");
ofstream fout_jaccard("RawOutput/Jaccard" + skipped_string + ".txt");
ofstream fout_name_change("RawOutput/NameChange" + skipped_string + ".txt");
ofstream fout_width[15];
// ofstream fout_one_significant("RawOutput/OnlyOneSignificantRoot" + skipped_string + ".txt");



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


int BFS(int x) {
    vector<pair<int, int>> Q; // (SCC number, depth)
    vector<int> par;
    for(int i : comp[x]) {
        Q.push_back(make_pair(i, 0));
        par.push_back(-1);
        local_mark[i] = 1;
    }
    int head = 0;
    while(head < (int)Q.size()) {
        biggest_interval = max(biggest_interval, local_earliest[Q[head].first]);
        for(int temp : graph[Q[head].first]) {
            if(local_mark[temp] == 0) {
                local_mark[temp] = 1;
                Q.push_back(make_pair(temp, Q[head].second + 1));
                par.push_back(head);
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
            local_width[Q[i].second] ++;
            local_paper_depth_set.insert(local_earliest_index[Q[i].first]);
            if(i == head - 1 || Q[i].second != Q[i + 1].second) {
                paper_depth_count[depth][Q[i].second].push_back(local_paper_depth_set.size());
                local_paper_depth_set.clear();
            }
        }
        for(int i = 0; i <= depth; i++) {
            width[depth][i].push_back(local_width[i]);
        }
    }

    return Q[head - 1].second;
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
    fill(rev_local_author_id.begin(), rev_local_author_id.end(), 0);
    local_counter = 0;
    for(int i = 0; i < H; i++) {
        graph[i].clear();
        rev_graph[i].clear();
        comp[i].clear();
    }
    fill(local_earliest.begin(), local_earliest.end(), 0);
    fill(local_earliest_index.begin(), local_earliest_index.end(), 0);
    fin_time.clear();
    fill(local_mark.begin(), local_mark.end(), 0);
    fill(rev_comp.begin(), rev_comp.end(), 0); 

    cerr << "BUILDING GRAPH " << endl;
    for(int i = 0; i < (int)word_bucket[x].size(); i++) {
        cerr << i << " ----------- " << word_bucket[x].size()  << endl;
        for(int author : word_bucket[x][i].authors) {
            cerr << rev_author_to_num[author] << endl;
            if(local_author_id.find(author) == local_author_id.end()){
                rev_local_author_id[local_counter] = author;
                local_author_id[author] = local_counter++;
                local_earliest[local_counter - 1] = word_bucket[x][i].getTime();
                local_earliest_index[local_counter - 1] = i;
            }
        }
        int j = -1;
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
                    graph[first].push_back(second);
                    rev_graph[second].push_back(first);
                }
                if(local_earliest[first] < local_earliest[second] && local_earliest[second] == word_bucket[x][i].getTime()) {
                    fout_exp_difference << exp_first - exp_second << endl;
                }
            }
        }
    }
    cerr << "Number of authors: " << local_counter << endl;
#if 1
    //    cerr << "STARTED SCC" << endl;
    // strongly connected component
    for(int i = 0 ; i < local_counter; i++) {
        if(local_mark[i] == 0) {
            StrongDfs(i);
        }
    }
    cerr << "FINISHING TIME IS DONE" << endl;
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
    int max_people = 0;
    int best_comp = 0;
    int second_max_people = 0;
    int second_best_comp = 0;
    for(int x : comps_to_check) {
        fill(local_mark.begin(), local_mark.end(), 0);
        int temp = comp[x][0];
        int num = CountDfs(temp);
        if(max_people < num) {
            second_best_comp = best_comp;
            second_max_people = max_people;
            max_people = num;
            best_comp = x;
        } else if (second_max_people < num) {
            second_max_people = num;
            second_best_comp = x;
        }
    }
    // Find the intersection of two biggest sets.
    cerr << " Finding the intersection of two biggest sets" << endl;
    fill(local_mark.begin(), local_mark.end(), 0);
    int temp = comp[best_comp][0];
    CountDfs(temp);
    int intersection = second_max_people;
    if(comp[second_best_comp].size() > 0 && local_mark[comp[second_best_comp][0]] == 0) {
        intersection -= CountDfs(comp[second_best_comp][0]);
    }
    cerr << " LOOK INTO THE BEST COMPONENT" << endl;
    set<int> seed_authors;
    set<int> seed_authors_copy;
    set<int> seed_paper_indecies;
    for(int i = 0; i < (int)comp[best_comp].size() ; i++) {
        seed_authors.insert(rev_local_author_id[comp[best_comp][i]]);
        seed_authors_copy.insert(rev_local_author_id[comp[best_comp][i]]);
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
        int max_path1 = BFS(best_comp);
        local_biggest_interval = biggest_interval - local_earliest[comp[best_comp][0]];
        fill(local_mark.begin(), local_mark.end(), 0);
        int max_path2 = 0;
        if(second_best_comp > 0) {
            max_path2 = BFS(second_best_comp);
        }
        int author_count1 = 0, author_count2 = 0;
        int paper_count1 = 0, paper_count2 = 0;
        for(int i = 0; i < local_counter; i++) {
            if(rev_comp[i] == best_comp) {
                author_count1++;
            }
            if(rev_comp[i] == second_best_comp) {
                author_count2++;
            }
        }
        int root_paper = -1;
        for(int i = 0; i < (int)word_bucket[x].size(); i++) {
            if(word_bucket[x][i].getTime() == local_earliest[comp[best_comp][0]]){
                for(int author : word_bucket[x][i].authors) {
                    int current_id = local_author_id[author];
                    if(local_earliest[current_id] == word_bucket[x][i].getTime()) {
                        paper_count1++;
                        root_paper = i;
                        break;
                    }
                }
            }
            if(second_best_comp > 0 && word_bucket[x][i].getTime() == local_earliest[comp[second_best_comp][0]]){
                for(int author : word_bucket[x][i].authors) {
                    int current_id = local_author_id[author];
                    if(local_earliest[current_id] == word_bucket[x][i].getTime()) {
                        paper_count2++;
                        break;
                    }
                }
            }
        }
        if(root_paper == - 1){
            cout << "WE HAVE A PROBLEM" << endl;
            cout << rev_macro_to_num[word_bucket[x][0].macro_number] << endl; 
            exit(0);
        }
        fout_stats << paper_count1 << " " << paper_count2 << " " <<  word_bucket[x].size() << " ";
        fout_stats << author_count1 << " " << author_count2  << " " << max_people << " " << second_max_people << " " << all_people.size() << " ";
        fout_stats << local_biggest_interval << " " << max_path1 << " " << max_path2 << " ";
        for(int i = 0; i < (int)word_bucket[x][root_paper].categories.size(); i++) {
            fout_stats << word_bucket[x][root_paper].categories[i];
            if(i != (int)word_bucket[x][root_paper].categories.size() - 1) {
                fout_stats << "/";
            }
        }
        fout_stats << " " << rev_macro_to_num[word_bucket[x][0].macro_number] << endl; 
        if(max_path1 > 4) {
            interesting_macros.push_back(x);
        }
    }

    // cerr << seed_paper_indecies.size() << endl;
    // int best_index = *seed_paper_indecies.begin();
    // word, number of people on biggest impact, number of people on second biggest impact, intersecion of the two biggest sets,  all the people used that word, a sample text in the biggest component
    //    feynman_fout << rev_macro_to_num[word_bucket[x][best_index].macro_number] << endl;
    //    feynman_fout << "Number of people in max and second max component: " << max_people  << " " << second_max_people << endl;
    //    feynman_fout << "Intersection of the two biggest components and number of all people using the macro: " << intersection << " " << all_people.size() << " " << endl;

    feynman_fout << max_people  << " " << all_people.size() << endl;
    //    feynman_fout << max_people  << " " << second_max_people << endl;
    //    feynman_fout << intersection << " " << all_people.size() << " " << endl;
    // When the best paper was published, when the best burst started
    //    feynman_fout << word_bucket[x][best_index].day  << "/" << word_bucket[x][best_index].month << "/" << word_bucket[x][best_index].year << endl;
    //    for(int author : seed_authors_copy) {
    //       feynman_fout << "Author: " << rev_author_to_num[author] << endl;
    //    }
    //    feynman_fout << endl << "________________________________________" << endl;

    fout_frac_length << rev_macro_to_num[word_bucket[x][0].macro_number].length() << " " << max_people / (double)all_people.size() << endl;
    fout_biggest_second_biggest << max_people / (double) all_people.size()  << " " << second_max_people/(double) all_people.size() << endl;
    fout_heat_map <<  rev_macro_to_num[word_bucket[x][0].macro_number].length() << " " << all_people.size() << " " << max_people/(double) all_people.size() << endl; 
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
            if(macro.authors.size() < 50 && macro.authors.size() > 0 && temp != "") { 
                macros.push_back(macro); 
            }
        }
        getline(fin, s); // you have two empty lines
    }
    cerr << "------> " << skipped << endl;
    sort(macros.begin(), macros.end());
    for(int i = 0; i < (int) macros.size(); i++) {
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
    cerr << macro_counter << " different macros used in  " << macros.size() << " comments" << endl;
	cerr << "authors count:" << author_experience.size() << endl;

    for(int i = 0; i < (int)macros.size(); i++) {
        word_bucket[macros[i].macro_number].push_back(macros[i]);
    }
    macros.clear();
    fout_stats << "#papersRoot1 #papersRoot2 #papersAll #authorsRoot1 #authorsRoot2 #authorsComponent1 #authorsComponent2 #authorsAll BiggestAdaptingInterval MaxPathRoot1 MaxPathRoot2 Categories Macro" << endl;
    for(int i = 1; i < (int)macro_counter; i++) {
        solve(i); 
    }
    cerr << "num above cut: " <<  num_above_cut << endl;
    cerr << "macros with max_path larger than threshold: " << interesting_macros.size() << endl;
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

    { // max path components
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
            }
            fout_width[i].close();
        }
    }

    return 0;
} 

