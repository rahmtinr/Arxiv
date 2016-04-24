#include<iostream>
#include<fstream>
#include<set>
#include<vector>
#include<sstream>
using namespace std;

string month[14] = {"bla bla", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

class Date {
    public:
        Date() {
            month = 0;
            year = 0;
            day = 0;
        }
        int month;
        int day;
        int year;

};

string RemoveAllSpaces(string s) {
    string ret ="";
    for(int i = 0; i < (int)s.length(); i++) {
        if(s[i] == ' ') {
            continue;
        }
        ret += s[i];
    }
    return ret;
}
string trim(string s) {
    string ret ="";
    for(int i = 0; i < (int)s.length(); i++) {
        if(s[i] == ' ') {
            continue;
        }
        s = s.substr(i);
        break;
    }
    int count = 0;
    for(int i = 0; i < (int)s.length(); i++) {
        if(s[i] == '(') {
            count++;
        } else if(s[i] == ')'){
            count--;
        }
        if(count == 0) {
            ret += s[i];
        }
    }
    s = ret;
    ret.clear();
    for(int i = 0; i < (int)s.length(); i++) {
        if(i + 5 < (int)s.length() && s.substr(i, 5) == " and ") {
            ret += ',';
            i += 4;
        }
        ret += s[i];
    }
    return ret;
}

string RemoveEscapes(string s) {
    string ret = "";
    for(int i = 0; i < (int)s.length() - 1; i++) {
        if(s[i] == '\\' && s[i + 1] == '\\') {
            i++;
            continue;
        }
        ret += s[i];
    }
    ret += s[s.length() - 1];
    // remove comment
    s = ret;
    ret.clear();
    for(int i = 0; i < (int)s.length() - 1; i++) {
        if(s[i] == '%') {
            return ret;
        }
        if(s[i] == '\\' && s[i + 1] == '%')  {
            ret += "\\%";
            i++;
            continue;
        }
        ret += s[i];
    }
    ret +=s[s.length() - 1];
    return ret;
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        cerr << "number of arguments not correct: " << argc << endl; 
        return 0;
    }
    string abstract_file = argv[1];
    string tex_file = argv[2];
    ifstream fin_abstract(abstract_file.c_str());
    ifstream fin_tex(tex_file.c_str());
    vector<string> authors;
    set<string> macros;
    Date date;
    {
        string s;
        string author_pattern = "Authors:";
        string date_pattern = "Date:";
        while(getline(fin_abstract, s)) {
            stringstream ss(trim(s));
            ss >> s;
            if(s.substr(0, author_pattern.size()) == author_pattern) {
                while(getline(ss, s, ',')) {
                    authors.push_back(trim(s));
                }
                continue;
            }
            if(s.substr(0, date_pattern.size()) == date_pattern) {
                string week_day;
                int dd = 0;
                string mm_str;
                int mm = 0;
                int yyyy;
                ss >> week_day >> dd >> mm_str >> yyyy;
                for(int i = 1; i < 13; i++){ 
                    if(mm_str == month[i]) {
                        mm = i;
                        break;
                    }
                    if(i == 12) {
                        cerr << "RIDI -------------------------------------------------------------------:" << i << endl;
                    }
                }

                //            cerr << dd << "/" << mm << "/" << yyyy << endl;
                date.day = dd;
                date.month = mm;
                date.year = yyyy;
                continue;
            }
        }
    }
    {
        string s;
        string new_command_pattern = "\\newcommand";
        //        string renew_command_pattern = "\\renewcommand";
        string def_pattern = "\\def";
        string defined_pattern = "\\defined";
        string natural;
        // Done with the abstract file
        int line = 0;
        while(getline(fin_tex, s)) {
            line++;
//            cerr << line << endl;
            natural = s;
            string temp = "";
            for(int i = 0; i < (int)s.length(); i++) {
                if(s[i] == ' '){
                    continue;
                }
                temp = s.substr(i);
                break;
            }
            s = temp;
            if(s == "") { // all space
                continue;
            }
            for(int i = s.length() - 1; i >= 0; i--) {
                if(s[i] == ' ') {
                    continue;
                }
                s = s.substr(0, i + 1);
                break;
            }
            s = RemoveEscapes(s);
            bool check = false;
            if(s.substr(0, new_command_pattern.size()) == new_command_pattern) {
                s = s.substr(new_command_pattern.size());
                check = true;
            }

            if(s.substr(0, def_pattern.size()) == def_pattern && s.substr(0, defined_pattern.length()) != defined_pattern) {
                s = s.substr(def_pattern.size());
                check = true;
            }
            /*
               if(s.substr(0, renew_command_pattern.size()) == renew_command_pattern) {
               s = s.substr(renew_command_pattern.size());   
               check = true;
               }
             */
            if(check == true) {
                int counter = 0;
                while(s.find("{") == string::npos && getline(fin_tex, temp)) {  // for def or newcommand you need to have "{" so read till you get there
                    line++;
                    temp = RemoveEscapes(temp);
                    s += " " + temp;
                }
                int line_counter = 0;
                do { // read until the bracket ends
                    temp = s;
                    if(counter > 0 && getline(fin_tex, temp)) {
                        line++;
                        temp = RemoveEscapes(temp);
                        s += " " + temp;
                    }
                    line_counter++;
                    if(line_counter > 5) {
                    //    break;
                    }
                    for(int i = 0; i < (int)temp.length(); i++) {
                        if(temp[i] == '{' && (i == 0 || temp[i - 1] !='\\')) {
                            counter++;
                        }
                        if(temp[i] == '}' && (i == 0 || temp[i - 1] != '\\')) {
                            counter--;
                        }
                    }
                } while(counter > 0);
                if(line_counter > 5 || counter > 0){ // file ended before I figured out or the macro length is more than 5
//                    continue;
                }

                int index = s.length() - 1;
                while(index > 0 && (s[index] != '}' || (s[index] =='}' && s[index - 1] =='\\'))) { // find the first closing bracket
                    index--;
                }
                if(index <= 0) {
                    continue;
                }
                if(s[index] == '}') {
                    index--;
                    counter++;
                    while(counter > 0 && index >= 0) {
                        if(s[index] == '}' && (index == 0 || s[index - 1] != '\\')){
                            counter++;
                        }
                        if(s[index] == '{' && (index == 0 || s[index - 1] !='\\')){
                            counter--;
                        }
                        index--;
                    }
                    if(counter == 0) {
                        s = s.substr(index + 2, s.length() - index - 3); // remove the first and last bracket for the pure macro
                        if(s != "") {
                            macros.insert(RemoveAllSpaces(s));
                        }
                    } else { // index should be negative then
                        cerr << "WRONG input" << endl;
                        getchar();
                    }
                } else {
                    cerr << " WHAT " << endl;
                    cerr << s << " " << natural << endl;
                    getchar();
                    cerr << "Weird" << endl;
                }
                continue;
            }
        }
    }
    //    print date , authors and macros!
    ofstream fout("../../../Macros.txt", ios::out | ios::app);
    for(string s : macros) {
        fout << "Macro: " << s << endl;
        fout << "Date: " << date.day << " " << date.month << " " << date.year << endl;
        for(string a : authors) {
            if(a == "") {
                continue;
            }
            fout << "Author: " << a << endl;
        }
        fout << endl;
        fout << endl;
    }
    return 0;
}
