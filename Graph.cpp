#include"Graph.h"

const int MAX_LOCAL_N = 70000 + 10;
vector<int> graph[MAX_LOCAL_N];
bool mark[MAX_LOCAL_N];
int dist[3][MAX_LOCAL_N];
int Q[MAX_LOCAL_N];
int between_cent[3];
void bfs(int x, int author1, int author2, int max_index) {
    memset(mark, 0, sizeof mark);
    memset(dist[0], -1, sizeof dist[0]);
    int head = 0, tail = 1;
    Q[head] = x;
    dist[0][x] = 0;
    while(head < tail) {
        int temp = Q[head];
        head++;
        for(int y : graph[temp]) {
            if(mark[y] == 1) {
                continue;
            }
            mark[y] = 1;
            Q[tail++] = y;
            dist[0][y] = dist[0][temp] + 1;
        }
    }
    int temp = 0;
    if(x == author1) {
        temp = 1;
    }
    if(x == author2) {
        temp = 2;
    }
    if(temp > 0) {
        for(int i = 1; i < max_index; i++) {
            dist[temp][i] = dist[0][i];
        }
    } else {
        for(int j = 1; j <= 2; j++) {
            if(j == temp) {
                continue;
            }
            for(int i = 1; i < max_index; i++) {
                if(dist[0][i] == dist[j][i] + dist[j][x]) {
                    between_cent[j]++;
                }
            }
        }
    }
}


GraphFeatures PreProcessLocalGraphFeatures(int x, Macro macro, int local_author_count, int index1, int index2) {
    cerr << "graph local: " << local_author_count << endl;
    for(int i = 1; i <= local_author_count; i++) {
        graph[i].clear();
    }
    between_cent[2] = between_cent[1] = 0;
    memset(dist, -1, sizeof dist);
    int max_index = -1;
    for(int i = 0; i < (int) word_bucket[x].size(); i++) { // this only counts for only the co-authorships people using this macro
        if(word_bucket[x][i] == macro) {
            break;
        }
        for(int author1 :  word_bucket[x][i].authors) {
            int local_author1 = local_author_id[author1];
            max_index = max(max_index, local_author1);
            for(int author2 :  word_bucket[x][i].authors) {
                int local_author2 = local_author_id[author2];
                if(local_author1 == local_author2) {
                    continue;
                }
                graph[local_author1].push_back(local_author2);
            }
        }
    }
    for(int i = 1; i <= max_index; i++) {
        sort(graph[i].begin(), graph[i].end());
        graph[i].resize(unique(graph[i].begin(), graph[i].end()) - graph[i].begin());
    }
    //    degree_author1 = graph[local_author_id[macro.authors[0]]].size()
    //    degree_author2 = graph[local_author_id[macro.authors[1]]].size()
    //    centrality could be computed on graph!

    int author1 = local_author_id[macro.authors[index1]];
    int author2 = local_author_id[macro.authors[index2]];
    bfs(author1, author1, author2, max_index + 1);
    bfs(author2, author1, author2, max_index + 1);

    for(int i = 1; i <= max_index; i++) {
        if(i == author1 || i == author2) {
            continue;
        }
        bfs(i,author1, author2, max_index + 1);
    }
    GraphFeatures graph_features;
    graph_features.degree[0] = graph[author1].size();
    graph_features.degree[1] = graph[author2].size();
    if(max_index > 2) {
        graph_features.cent[0] = between_cent[1] / ((double)(max_index - 1) * (max_index - 2));
        graph_features.cent[1] = between_cent[2] / ((double)(max_index - 1) * (max_index - 2));
    }
    return graph_features;
}

GraphFeatures PreProcessGlobalGraphFeatures(int x, Macro macro, int local_author_count, int index1, int index2) {
    cerr << " graph Global:" << local_author_count << endl;
    for(int i = 1; i <= local_author_count; i++) {
        graph[i].clear();
    }
    set<int> valid_authors;
    between_cent[2] = between_cent[1] = 0;
    memset(dist, -1, sizeof dist);
    int max_index = -1;
    for(int i = 0; i < (int) word_bucket[x].size(); i++) { // this only counts for only the co-authorships people using this macro
        if(word_bucket[x][i] == macro) {
            break;
        }
        for(int author1 :  word_bucket[x][i].authors) {
            int local_author1 = local_author_id[author1];
            max_index = max(max_index, local_author1);
            valid_authors.insert(author1);
        }
    }
    for(int i = 1; i < macro_counter; i++) {
        for(Macro macro2 : word_bucket[i]) {
            if(macro < macro2) {
                break;
            }
            for(int author1 : macro2.authors) {
                if(valid_authors.find(author1) == valid_authors.end()) {
                    continue;
                }
                for(int author2 : macro2.authors) {
                    if(author1 == author2 || valid_authors.find(author2) == valid_authors.end()) {
                        continue;
                    }
                    int local_author1 = local_author_id[author1];
                    int local_author2 = local_author_id[author2];
                    graph[local_author1].push_back(local_author2);
                }

            }
        }
    }
    for(int i = 1; i <= max_index; i++) {
        sort(graph[i].begin(), graph[i].end());
        graph[i].resize(unique(graph[i].begin(), graph[i].end()) - graph[i].begin());
    }
    //    degree_author1 = graph[local_author_id[macro.authors[0]]].size()
    //    degree_author2 = graph[local_author_id[macro.authors[1]]].size()
    //    centrality could be computed on graph!

    int author1 = local_author_id[macro.authors[index1]];
    int author2 = local_author_id[macro.authors[index2]];
    bfs(author1, author1, author2, max_index + 1);
    bfs(author2, author1, author2, max_index + 1);

    for(int i = 1; i <= max_index; i++) {
        if(i == author1 || i == author2) {
            continue;
        }
        bfs(i,author1, author2, max_index + 1);
    }
    GraphFeatures graph_features;
    graph_features.degree[0] = graph[author1].size();
    graph_features.degree[1] = graph[author2].size();
    if(max_index > 2) {
        graph_features.cent[0] = between_cent[1] / ((double)(max_index - 1) * (max_index - 2));
        graph_features.cent[1] = between_cent[2] / ((double)(max_index - 1) * (max_index - 2));
    }
    return graph_features;
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

