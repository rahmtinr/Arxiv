#include"Macro.h"

map<int, string> rev_macro_to_num;
map<int, string> rev_author_to_num;

int Macro::getTime() {
    return 500 * year + 40 * month + day;
}

string Macro::ToString() {
    string ret ="";
    ret += (rev_macro_to_num[macro_number] + " " + to_string(day) + "/" + to_string(month) + "/" + to_string(year) + "\n");
    for(int i = 0; i < (int)authors.size(); i++) {
        ret += rev_author_to_num[authors[i]];
        if(i < (int)authors.size() - 1){
            ret += ", ";
        }
    }
    ret += "\n";
    return ret;
}

