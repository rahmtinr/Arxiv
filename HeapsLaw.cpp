#include"HeapsLaw.h"
#if MACRO_HEAPS_LAW_DEF
void HeapsLawForAMacro(int x) { // Heaps law and name change 
    if(word_bucket[x].size() == 0){
        return;
    }
    if(!GoodMacro(word_bucket[x][0], word_bucket[x].size(), rev_macro_to_num[x].length())) { // macro needs to be used and should have a length
        return;
	}
	int local_author_count = 1;
	initialize(local_author_count, x);
	cerr << x << " " << local_author_count << " ---- " << word_bucket[x].size() << endl;
	if(TYPE == "") {
		if(local_author_count < 20) {
			return;
		}
	} else if( TYPE == "-narrow") {
		if (local_author_count > 250 || local_author_count < 20) {
			return;
		}
	} else if( TYPE == "-wide" ) {
		if(local_author_count < 250) {
			return;
		}
	} else {
		cerr << "Problem with the arguments" << endl;
		return;
	}


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
}
#endif

#if AUTHOR_HEAPS_LAW_DEF
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
    ofstream fout_sample_author_macros(folder + "SampleAuthorMacroUsage.txt");
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
