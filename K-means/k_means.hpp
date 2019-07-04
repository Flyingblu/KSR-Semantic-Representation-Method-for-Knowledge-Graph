#ifndef k_means_hpp
#define k_means_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "../RDF_parser/progress_bar.hpp"
using namespace std;

class k_means
{
    public:
        k_means();
        k_means(string save_path, unsigned int cluster_num, unsigned int significant_num):
        save_path(save_path),
        cluster_num(cluster_num), 
        id(significant_num, 0),
        connection_table(significant_num, vector<unsigned int>(significant_num, 0))
        {
            cout << "Initializing Done ..." << endl;
            
        };    

        void load_id(string);
        void load_table(string);
        void cluster();
        unsigned int center_point(vector<unsigned int>&);
        void count_connection(vector<unsigned int>&, unordered_map<unsigned int, vector<unsigned int>>&);
        void log(string save_path);

    private:
        vector<unsigned int> id;
        vector<vector<unsigned int>> connection_table;
        unordered_map<unsigned int, unordered_map<unsigned int, unsigned int>> connection_table_new;
        unsigned int cluster_num;
        string save_path;
};










#endif