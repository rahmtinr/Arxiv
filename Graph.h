#ifndef GRAPH_H
#define GRAPH_H
#include"Core.h"
#include<set>
#include<algorithm>
#include<cstring>
#include<iostream>
using namespace std;

struct GraphFeatures {
    int degree[2];
    double cent[2];
};

extern vector<int> graph[70000 + 10];
extern bool mark[70000 + 10];
GraphFeatures PreProcessLocalGraphFeatures(int, Macro, int, int, int);
GraphFeatures PreProcessGlobalGraphFeatures(int, Macro, int, int, int);
pair<double, double> ClusteringCoeff(int);
#endif
