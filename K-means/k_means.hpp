#ifndef k_means_hpp
#define k_means_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "../RDF_parser/progress_bar.hpp"
#include <random>
#include <set>
using namespace std;

struct cluster
{
    unsigned int id = 0;
    unsigned int cunt = 0;
};
class k_means
{
    public:
        k_means(){};
        k_means(unsigned int cluster_num, unsigned int significant_num, unsigned int init_num):
        cluster_num(cluster_num), 
        init_num(init_num),
        id(significant_num),
        initialization(init_num),
        k_means_cluster(init_num, vector<cluster>(cluster_num)),
        connection_table(significant_num, vector<unsigned int>(significant_num, 0)),
        connection_table_new(init_num, vector<vector<unsigned int>>(cluster_num, vector<unsigned int>(cluster_num, 0))),
        score(init_num)
        {
            for (int i = 0; i < init_num; ++i)
            {
                score[i].id = i;
            }
            cout << "Initializing Done ..." << endl;
            
        };    

        void load_id(string);
        void load_table(string);
        void k_means_clusterizing();
        unsigned int center_point(vector<unsigned int>&);
        void count_connection(vector<cluster>&, unordered_map<unsigned int, vector<unsigned int>>&, int);
        void log(string);

    private:
        vector<cluster> id;
        vector<vector<unsigned int>> initialization;
        vector<vector<unsigned int>> connection_table;
        vector<vector<vector<unsigned int>>> connection_table_new;
        vector<vector<cluster>> k_means_cluster; // stored every random cluster_id
        vector<cluster> score;
        unsigned int cluster_num;
        unsigned int init_num;

};










#endif