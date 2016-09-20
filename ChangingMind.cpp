#include<iostream>
#include<map>
#include<set>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
#include"HeapsLaw.h"
#include"Core.h"
#include"Macro.h"
#include"Graph.h"
#include"Fitness.h"
#include"Reader.h"

using namespace std;

set<pair<int, string> > macroBody_MacroName;
set<string> different_documents;
#if SOLVE_DEF
bool solve(int x) {
    if(word_bucket[x].size() == 0){ 
        return false;
    }
    if(!GoodMacro(word_bucket[x][0], macro_paper_count[x], rev_macro_to_num[x].length())) { // macro needs to be used and should have a length
        return false;
    }
    int local_author_count = 1;
    initialize(local_author_count, x);
    cerr << local_author_count << " ---- " << TYPE <<" -"<< endl;
    if(TYPE == "") { 
        if(local_author_count < 20) {
            cerr << local_author_count << endl;
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
    cerr << "2" << endl; 

//  Most authoratitive author prediction
    for(int i = 0; i < (int)word_bucket[x].size(); i++) {
        int winner = -1;
        Macro macro = word_bucket[x][i];
        bool check = true;
        int count_equal_names = 0;
        int null_names = 0;
        int min_index = 0, max_index = 0, most_recent_index = 1;
        vector<Macro> macros_used;
        macros_used.clear();
        macros_used.push_back(macro);
        vector<int> indecies_that_have_past;
        indecies_that_have_past.clear();
        for(int j = 0; j < (int)macro.authors.size() && check == true; j++) {
            int author_j = local_author_id[macro.authors[j]];
            if(person_pointer[author_j] <= person_pointer[local_author_id[macro.authors[min_index]]]) {
                min_index = j;
            }
            if(person_pointer[author_j] > 0) {
                indecies_that_have_past.push_back(j);
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
        bool same_date = false;
        if(check == true && count_equal_names == 1 && (int)indecies_that_have_past.size() == N_TO_1 && (null_names == 0 || (null_names == 1 && SMART == "_smart" && (int)macro.authors.size() == N_TO_1))) {
//            cerr << " salam " << endl;
            for(int j = 1; j <= (int)indecies_that_have_past.size(); j++) {
                if(macros_used[j] == macros_used[most_recent_index]) { // only checks the date
                    continue;
                }
                if(macros_used[j] < macros_used[most_recent_index]) {
                    most_recent_index = j - 1;
                }
            }
//            cerr << " finished here " << endl;
            for(int j = 1; j <(int) macros_used.size(); j++) { 
                if(macros_used[0] == macros_used[j]) {
                    same_date = true;
                    break;
                }
            }
//            cerr << " YOHOO " << endl;
            if(same_date == false) {
//                cerr << " aha " << endl;
                GraphFeatures local_graph_features = PreProcessLocalGraphFeatures(x, word_bucket[x][i], local_author_count, indecies_that_have_past[0], indecies_that_have_past[1]);
//                cerr << "YAY local done! " << endl;
                GraphFeatures global_graph_features = PreProcessGlobalGraphFeatures(x, word_bucket[x][i], local_author_count, indecies_that_have_past[0], indecies_that_have_past[1]);
//                cerr << "YAY global done! " << endl;
                fout_N_becomes_1 << RemoveSpaces(rev_macro_to_num[macro.macro_number]) << ": ";
                for(int k = 0; k < (int)macros_used.size(); k++) {
                    fout_N_becomes_1 << macros_used[k].name << " ";
                }
                fout_N_becomes_1 << macro.paper_id << " ";
                for(int k = 1; k < (int)macros_used.size(); k++) {
                    string author_k = rev_author_to_num[macros_used[0].authors[indecies_that_have_past[k-1]]];
                    fout_N_becomes_1 << RemoveSpaces(author_k) << " " << macros_used[k].paper_id << " " << macros_used[0].experience[indecies_that_have_past[k - 1]] << " " ;
                }
                fout_N_becomes_1 << endl;

                string macro_body = rev_macro_to_num[macro.macro_number];
                int special_characters = 0;
                int dollar_signs = 0;
                int back_slashes = 0;
                int depth = 0, max_depth = 0;
                for(int i = 0; i < (int)macro_body.length(); i++) {
                    if(macro_body[i] == '{' || macro_body[i] =='#') {
                        special_characters++;
                    }
                    if(macro_body[i] == '$') {
                        dollar_signs++;
                    }
                    if(macro_body[i] == '\\') {
                        back_slashes++;
                    }
                    if(macro_body[i] == '{') {
                        depth++;
                        max_depth = max(depth, max_depth);
                    }
                    if(macro_body[i] == '}') {
                        depth--;
                    }
                }

//                cerr << " going into printing" << endl;
                int loop_counter = -1;
                for(int k : indecies_that_have_past) {
                    loop_counter++;
                    int is_max = 0;
                    int is_min = 0;
                    int is_most_recent = 0;
                    if(min_index == k) {
                        is_min = 1;
                    }
                    if(max_index == k) {
                        is_max = 1;
                    }
                    if(most_recent_index == loop_counter) {
                        is_most_recent = 1;
                    }
                    int local_author = local_author_id[macro.authors[k]]; 
                    double denom = (double)(max(1, person_pointer[local_author] - 1));
/*
                    fout_learning << macro.experience[k] << ", " << person_pointer[local_author] << ", " << change[local_author] / denom << ", "; 
                    fout_learning << is_max << ", " << is_min << ", " << is_most_recent << ", ";
*/
                    pair<int, int> my_pair = make_pair(macro.authors[indecies_that_have_past[0]], macro.authors[indecies_that_have_past[1]]);
                    if(N_TO_1 == 2 && unique_authorPair.find(my_pair) == unique_authorPair.end()) {
//                        cerr << " HERE IS THE K I want to print" << k << " ::::: " << macro.experience[k] << endl;
                        fout_learning_unique_authorPair << macro.experience[k] << ", " << person_pointer[local_author] << ", " << change[local_author] / denom << ", "; 
                        fout_learning_unique_authorPair << is_max << ", " << is_min << ", " << is_most_recent << ", " << macros_used[loop_counter + 1].name.length() << ", ";
                        fout_learning_unique_authorPair << macro.co_authors_count_used_previously[k] << ", ";
                        fout_learning_unique_authorPair << local_graph_features.degree[loop_counter] << ", " << local_graph_features.cent[loop_counter] << ", ";
                        fout_learning_unique_authorPair << global_graph_features.degree[loop_counter] << ", " << global_graph_features.cent[loop_counter] << ", ";
                    }
/*                    
                    if(N_TO_1 == 2 && unique_paper.find(macro.paper_id) == unique_paper.end()) {
                        fout_learning_unique_paper << macro.experience[k] << ", " << person_pointer[local_author] << ", " << change[local_author] / denom << ", "; 
                        fout_learning_unique_paper << is_max << ", " << is_min << ", " << is_most_recent << ", " << macros_used[loop_counter + 1].name.length() << ", ";
                        fout_learning_unique_paper << macro.co_authors_count_used_previously[k] << ", ";
                    }
*/
                }
//                fout_learning << winner << endl; 
                pair<int, int> my_pair = make_pair(macro.authors[indecies_that_have_past[0]], macro.authors[indecies_that_have_past[1]]);
                if(N_TO_1 == 2 && unique_authorPair.find(my_pair) == unique_authorPair.end()) {
                    fout_learning_unique_authorPair << dollar_signs << ", " << back_slashes << ", " << max_depth << ", ";
                    fout_learning_unique_authorPair << rev_macro_to_num[macro.macro_number].length() << ", " << winner << endl;  
                }
                unique_authorPair.insert(my_pair);

/*
                if(N_TO_1 == 2 && unique_paper.find(macro.paper_id) == unique_paper.end()) {
                    fout_learning_unique_paper << winner << endl; 
                }
                unique_paper.insert(macro.paper_id);
*/
                data_points++;
            }
        }
//        cerr << i << " got here!" << endl;
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
        cerr << i << " finished!" << endl;
    }
    return true;
}
#endif

int main(int argc, char *argv[]) {
    if(SMART != "_smart" && SMART != "_nosmart") {
        cerr << " THERE IS A PROBLEM WITH THE SMART const variable" << endl;
        return 0;
    }
	string input_file;
	if(argc != 2) {
		cerr << " YOU HAVE NOT SET THE BODY PARAMETER" << endl;
		return 0;
	}
	cerr << argv[0] << " " << argv[1] << endl;
	if(string(argv[1]) == "body")  { // different names hit same body
		folder = "RawOutput/Body/";
		input_file = "All_Arxiv_Macros_body.txt";
	} else {
		folder = "RawOutput/Name/";
		input_file = "All_Arxiv_Macros.txt";
	}
	/*
	{
        ifstream fin_important_macros("Macros_with_length_above_4.txt");
        string s;
        while(getline(fin_important_macros, s)) {
            important_macros.insert(s);
        }
    }
    */
    Read(input_file);
    sort(macros.begin(), macros.end());
    /*
    { // DATA INFORMATION
        cerr << "Number of all macros: " << macros.size() << endl;
        cerr << "Number of unique macros: " << macro_counter << endl;
        int all_authors_count = 0;
        for(int i = 0; i <(int) macros.size(); i++) {
            macroBody_MacroName.insert(make_pair(macros[i].macro_number, macros[i].name));
            all_authors_count += macros[i].authors.size();
            different_documents.insert(macros[i].paper_id);
        }
        cerr << "Average number of macro names per macro: " <<  macroBody_MacroName.size() / (double)macro_counter << endl;
        cerr << "Average number of authors per paper: " << all_authors_count / (double)macros.size() << endl;
        cerr << "Number of documents with macros: " << different_documents.size() << endl;
        different_documents.clear();
        macroBody_MacroName.clear();
    }
    */
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
// #if false
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

    // Global is number of papers before, Local is number of times used this macro, has changed is 1 if different name, Completely new if first time name has came up

//    fout_learning.open("RawOutput/" + to_string(N_TO_1) + "_to_1_learning" + TYPE + SMART + ".txt");
//    fout_learning_unique_paper.open("RawOutput/" + to_string(N_TO_1) + "_to_1_learning" + TYPE + "-unique_paper" + SMART + ".txt");

    fout_learning_unique_authorPair.open(folder + to_string(N_TO_1) + "_to_1_learning" + TYPE + "-unique_authorPair" + SMART + ".txt");

    fout_N_becomes_1.open(folder + to_string(N_TO_1) + "_to_1_samples" + TYPE + SMART + ".txt");
    fout_N_becomes_1 << "MacroBody FinalName ";
    for(int i = 1; i <= N_TO_1; i++) {
        fout_N_becomes_1 << "Author" + to_string(i) + "MacroName ";
    }
    fout_N_becomes_1 << "FinalPaperId ";
    for(int i = 1; i <= N_TO_1; i++) {
        fout_N_becomes_1 << "Author" + to_string(i) + "Name" + to_string(i) + " PrevPaperAuthor" + to_string(i) + " GlobalExperience" + to_string(i);
    }
    fout_N_becomes_1 << endl;
    for(int i = 1; i <= N_TO_1; i++) {
        string temp = to_string(i);
//        fout_learning << "GlobalCurExp" + temp + ", LocalCurExp" + temp + ", LocalFracChange" + temp + ", IsMax" + temp +", IsMin" + temp + ", " + "IsMostRecent" + temp + ", "; 
//        fout_learning_unique_paper << "GlobalCurExp" + temp + ", LocalCurExp" + temp + ", LocalFracChange" + temp + ", IsMax" + temp +", IsMin" + temp + ", " + "IsMostRecent" + temp + ", "; 
 
        fout_learning_unique_authorPair << "GlobalCurExp" + temp + ", LocalCurExp" + temp + ", LocalFracChange" + temp + ", IsMax" + temp + ", IsMin" + temp + ", " + "IsMostRecent" + temp + ", ";
        fout_learning_unique_authorPair << "MacroNameLength" + temp + ", " + "CoAuthorCountUsedMacro" + temp + ", LocalDegree" + temp + ", BetweenCentLocal" + temp + ", "  ; 
        fout_learning_unique_authorPair << "GlobalDegree" + temp + ", BetweenCentGlobal" + temp + ", "  ; 
    }
//    fout_learning << "Label" << endl;
//    fout_learning_unique_paper << "Label" << endl;
    fout_learning_unique_authorPair << "NumberOfDollarSigns, NumberOfBackSlashes, CurlyBraceMaxDepth, ";
    fout_learning_unique_authorPair << "MacroBodyLength, Label" << endl;
    cerr << "count of unique macros: " << macro_counter << endl;
    for(int i = 1; i < (int)macro_counter; i++) {
       if( i % 10000 == 0 ) {
           cerr << i << endl;
       }
       solve(i); 
    }
    cerr << "ending solve" << endl;
#endif

#if MACRO_HEAPS_LAW_DEF
    // HEAPS LAW EACH MACRO BEING A BOOK AND NAMES BEING WORDS
    fout_heaps_law.open(folder + "heaps_law" + TYPE + ".txt");
    fout_heaps_law << "FinalLocalExperience Types Token" << endl;
    fout_experience_changed_name.open(folder + "Experience_changed_name" + TYPE + ".txt");
    fout_experience_changed_name << "FinalGlobalExperience FinalLocalExperience GlobalCurrentExperience LocalCurrentExperience HasChanged CompletelyNew" << endl;
    for(int i = 1; i < (int)macro_counter; i++) {
        if( i % 10000 == 0 ) {
            cerr << i << endl;
        }
        HeapsLawForAMacro(i); 
    }
#endif


#if AUTHOR_HEAPS_LAW_DEF
    // HEAPS LAW EACH PERSON BEING A BOOK
    fout_author_heaps.open(folder + "author_heaps.txt");
    fout_author_heaps << "Macros UniqueMacros" << endl;
    HeapsLawForAPerson();
#endif

#if K_2K_DEF
    for(int i = 1; i < 10; i++) {
        fout_k_2k[i].open(folder +"k_2k_prediction_" + to_string(5 * (1 << (i - 1)))  + ".txt");
        fout_k_2k[i] << "NumberofAuthorsOnFirstPaper, NumberOfPapers, GlobalMedian, GlobalMean, LocalClustering, GlobalClustering, ";
        fout_k_2k[i] << "MacroBodyLength, NumberOfSpecialCharacters, UniqueNames, ";
        fout_k_2k[i] << "NumberOfDollarSigns, NumberOfBackSlashes, MaxCurlyBracesDepth, Label" << endl;
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

