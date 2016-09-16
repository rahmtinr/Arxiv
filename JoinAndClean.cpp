#include<iostream>
#include<sstream>
#include<cstdio>
#include<cstdlib>
#include<algorithm>
#include<vector>
#include<set>
#include<map>
#include<iomanip>
#include<fstream>
#include<string>
using namespace std;

map<string, int> files;
map<string, vector<string>> paper_authors; 
string before0704(string s, string &month) {
	string code;
	string category;
	int bef = s.size() - 1;
	int counter = 0;
	for(int i = s.size() - 1; i >= 0; i--) {
		if(s[i] == '/' || (s[i] == '.' && counter == 0)) {
			counter++;
			if(counter == 2) {
				code = s.substr(i, bef - i);
				for(int j = 0; j < (int) code.length(); j++) {
					if(code[j] == '_') {
						code = code.substr(0, j);
						break;
					}
				}
			}
			if(counter == 3) { 
				category = s.substr(i + 1, bef - i - 1);
			}
			if(counter == 4) {
				month = s.substr(i + 1, bef - i - 1);
			}
			bef = i;
		}

	}
	if(counter < 4) {
		return "";
	}
	int x = stoi(month);
	string tag;
	if(x <  704 || x > 9000) {
		string temp = "";
		for(int i = 0; i < (int)category.length(); i++) {
			if(category[i] != '-'){
				temp += category[i];
			}
		}
		category = temp;
		tag = category + code;
	} else {
		tag = code.substr(1);
	}
	if(files.find(tag) == files.end()) {
		files[tag] = 0;
	}
	return tag;
}


void read_authors() {
	ifstream fin("cleaned_author.lis");
	string tag, author;
	while(fin >> tag >> author) {
		if(paper_authors.find(tag) == paper_authors.end()) {
			vector<string> v;
			paper_authors.insert(make_pair(tag, v));
		}
		vector<string>  v = paper_authors[tag];
		v.push_back(author);
		paper_authors[tag] = v;
	}
	/*
	for(auto p : paper_authors) {
		cout << p.first << " :::: ";
		for(string s : p.second) {
			cout << s << " --- ";
		}
		cout << endl;
	}
	*/
}

void read_macros() {
	string filename;
	string temp;
	string authors;
	string macro_name;
	string macro_body;
	string abstract;
	string title;
	string tag;
	string count;
	string month;

	ifstream fin("NEW_AllFeatures.txt");
	ofstream fout("TitlesAndAuthorsAndMonth.txt");	
//	ifstream fin("temp");
	bool print;
	while(getline(fin, filename)) {
		print = true;
		abstract = "";
		tag = before0704(filename, month);
		getline(fin, temp);
		getline(fin, authors);
		getline(fin, temp);
		vector<string> authors;
		while(true) {
			getline(fin, macro_name);
			if(macro_name == "Abstract") {
				break;
			}
			getline(fin, macro_body);
			getline(fin, count);
			if(count != "1") {
				files[tag]++;
			}
			if(paper_authors.find(tag) == paper_authors.end()) {
				print = false;
				continue;
			}
			authors = paper_authors[tag];
			if(authors.size() > 10) { 
				print = false;
				continue;
			}
			if(macro_body.length() > 1000) {
				print = false;
				continue;
			}
			/*
			cout << "macro_body: " << macro_body << endl;
			cout << "macro_name: " << macro_name << endl;
			cout << "paper_id: " << tag << endl;
			cout << "count: " << count << endl;
			string year;
			if(month[0] == '9') {
				year = "19" + month.substr(0,2);
			} else {
				year = "20" + month.substr(0,2);
			}
			cout << "date: 01 " << month.substr(2) << " " << year << endl;
			for(string author : authors) {
				cout << "author: " << author << endl;
			}
			cout << endl << endl;
			*/
		}
		while(getline(fin, temp)) {
			if(temp == "END OF ABSTRACT") {
				break;
			}
			abstract += temp;
		}
		getline(fin, title);
		if(title.length() > 600) {
			print = false;
		}
		getline(fin, temp);
		string token;
		{
			while(true) {
				string bef = title;
				stringstream ss(title);
				temp = "";
				while(ss >> token) { 
					if(token[0] == '\\' || token == "Title:") {
						continue;
					}
					string token_letter = "";
					for(int i = 0; i < (int)token.size(); i++) {
						if(token[i] == '{' || token[i] == '}' || token[i] == '[' || token[i] == ']' || token[i] =='(' || token[i] ==')') {
							temp += " ";
							continue;
						}
						if(token.substr(i, 9) == "\\footnote") {
							i+=8;
							temp += " ";
							continue;
						}
						if(token.substr(i, 7) == "\\thanks") {
							i += 6;
							temp += " ";
							continue;
						}
						token_letter += token[i];
					}
					temp += " " + token_letter;
				}
				title = temp;
				if(title == bef) {
					break;
				}
			}
		}
		{
			title = "";
			stringstream ss(temp);
			while(ss >> token) {
				title += token + " ";
			}
			if(title.length() == 0) { 
//				print = false;
			}
		}
		if(print == true) {
			if(tag.length() == 0) { 
				cerr << filename << endl;
			}
			fout << "Tag: " << tag << endl;
			fout << "Title: " << title << endl;
			authors = paper_authors[tag];
			for(string author : authors) {
				fout << "Author: " << author << endl;
			}
			fout << "Year: " << month << endl;
			fout << endl;
		}
	/*	
		if(files.size() > 2 ){
			 break;
		}
	*/	
	}
	cerr << files.size() << endl;
	for(auto p : files) { 
		if(p.second == 0) {
			files.erase(p.first);
		}
	}
	cerr << files.size() << endl;
}

int main() {
	read_authors();
	read_macros();
	return 0;
}
