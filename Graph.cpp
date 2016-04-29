#include"Graph.h"

pair<double, double> PreProcessGraphFeatures(int x, Macro macro, int local_author_count) {
    vector<int> graph[10000 + 10];
    for(int i = 1; i <local_author_count; i++) {
        graph[i].clear();
    }
    for(int i = 0; i < (int) macros.size(); i++) { // this only counts for only the co-authorships people using this macro
        if(macros[i] == macro) {
            break;
        }
        for(int author1 :  macros[i].authors) {
            int local_author1 = local_author_id[author1];
            for(int author2 :  macros[i].authors) {
                int local_author2 = local_author_id[author2];
                if(local_author1 == local_author2) {
                    continue;
                }
                graph[local_author1].push_back(local_author2);
            }
        }
    }
    for(int i = 1; i < local_author_count; i++) {
        sort(graph[i].begin(), graph[i].end());
        graph[i].erase(unique(graph[i].begin(), graph[i].end() ), graph[i].end());
    }
    //    degree_author1 = graph[local_author_id[macro.authors[0]]].size()
    //        //    //    degree_author2 = graph[local_author_id[macro.authors[1]]].size()
    //            //    //    centrality could be computed on graph!
    return make_pair(0.0, 0.0);
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

