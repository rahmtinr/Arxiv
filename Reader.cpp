#include"Reader.h"

void Read(string filename) {
    ifstream fin(filename);
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
        }
		if(macro.authors.size() > 0 && macro.authors.size() < 50 && temp != "") {
            macros.push_back(macro);
        }
        getline(fin, s); // you have two empty lines
    }
    cerr << " number of authors: " << author_counter << endl;
}

