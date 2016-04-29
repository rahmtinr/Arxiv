#ifndef FITNESS_H
#define FITNESS_H

#include<fstream>
#include"Core.h"
#include"Graph.h"
#include<iostream>

#if K_2K_DEF
extern ofstream fout_k_2k[10];
extern ofstream fout_k_2k_thresholds;
void prediction_k_2k(int);
struct OutputElement {
    string output_string;
    int authors_count;
    bool operator < (const OutputElement &other) const {
        return authors_count < other.authors_count;
    }
};
extern vector<OutputElement> k_2k[10];
#endif 

#if LONGEVITY_DEF
extern ofstream fout_author_fitness[11];
void AuthorFitness();
#endif

#endif
