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
int local_counter = 0;
vector<int> local_earliest(H);
vector<int> local_earliest_index(H);
vector<int> graph[H];
int mark[H];
int biggest_interval;
int Q[10000], depth[10000];
int width[12];

bool has_skipped = false;
string skipped_string = "";
int macro_counter = 1;
int author_counter = 1;
int skipped = 0;

ofstream fout_tree_properties("RawOutput/TreeProperties.txt");

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

struct InterestingMacro {
	int final_vol, break_index, label;
	string body, root_author;	
};

vector<InterestingMacro> interesting_macros;
vector<Macro> macros;
vector<Macro> word_bucket[3 * 1000 * 1000]; 
map<int, int> author_experience; // at the end of the code it will be final experience but while running it's the current_exp
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

	{ // read "interesting" macros with some side information
		ifstream fin_interesting_macros("RawOutput/BipartiteThresholds.txt");
		getline(fin_interesting_macros, s);
		int final_vol, break_index, label;
		string body, root_author;
		InterestingMacro im;
		while(fin_interesting_macros >>	final_vol >> break_index >> label >> root_author) {
			fin_interesting_macros >> std::ws;
			getline(fin_interesting_macros, body);
			im.final_vol = final_vol;
			im.break_index = break_index;
			im.label = label;
			im.root_author = root_author;
			im.body = body;
			interesting_macros.push_back(im);
		}
	}
}

void BFS(int x) {
	memset(mark, 0, sizeof mark);
	memset(width, 0, sizeof width);
	depth[x] = 0;
	int head = 0, tail = 0;
	Q[tail++] = x;
	mark[x] = 1;
	while(head < tail) {
		int y = Q[head];
		for(int neighbor : graph[y]) {
			if(mark[neighbor] == 0) {
				mark[neighbor] = 1;
				depth[neighbor] = depth[y] + 1;
				width[depth[neighbor]]++;
				Q[tail++] = neighbor;
			}
		}
		head++;
	}
}

bool solve(InterestingMacro im) {
	int x  = macro_to_num[im.body];
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
    fill(local_earliest.begin(), local_earliest.end(), 0);
    fill(local_earliest_index.begin(), local_earliest_index.end(), 0);

    for(int i = 0; i < (int)im.break_index; i++) {
        for(int author : word_bucket[x][i].authors) {
            if(local_author_id.find(author) == local_author_id.end()){
				graph[local_counter].clear();
                rev_local_author_id[local_counter] = author;
                local_author_id[author] = local_counter;
                local_earliest[local_counter] = word_bucket[x][i].getTime();
                local_earliest_index[local_counter] = i;
				local_counter++;
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
                if(local_earliest[first] <= local_earliest[second] && local_earliest[second] == word_bucket[x][i].getTime()) {
					graph[first].push_back(second);
				}
            }
        }
    }
	int local_root_author = local_author_id[author_to_num[im.root_author]];
	memset(width, 0, sizeof width);
	BFS(local_root_author);
	int years = word_bucket[x][im.break_index].year - word_bucket[x][0].year;
	int months = word_bucket[x][im.break_index].month - word_bucket[x][0].month;
	months += 12 * years;
	for(int i = 1; i <= 10; i++) {
		fout_tree_properties << width[i] << " ";
	}
	cerr << "Local Counter: " << local_counter << endl;
	fout_tree_properties << im.break_index << " " << local_counter << " " << months << " " << im.label << endl;
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
	for(int i = 1; i <= 10; i++) {
		fout_tree_properties << "Width" << to_string(i) << " ";
	}
	fout_tree_properties << "PaperCount AuthorCount Months Label" << endl;
    for(InterestingMacro x : interesting_macros) {
        solve(x); 
    }

    return 0;
} 

