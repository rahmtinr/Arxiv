#ifndef MACRO_H
#define MACRO_H

#include<vector>
#include<string>
#include<map>
using namespace std;
extern map<int, string> rev_macro_to_num;
extern map<int, string> rev_author_to_num;

struct Macro {
    int macro_number;
    std::string name;
    std::string paper_id;
    vector<int> authors;
    vector<int> experience;
    vector<int> co_authors_count_used_previously;
    vector<string> categories;
    int day, month, year;
    int count;
    bool operator == (const Macro &other) const { // published same day
        return ((year == other.year) && (month == other.month) && (day == other.day));
    }
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
    
    string ToString();
    int getTime();

};

#endif
