#ifndef clusterization_hpp
#define clusterization_hpp

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <iostream>
using namespace std;
class cluster
{
    public:
        cluster(string read_path, string save_path, unsigned int reserve_num): 
                reader(new ifstream(read_path, ios::binary)), 
                save_path(save_path), 
                cunt(reserve_num, 1), 
                cunt_entities(reserve_num, 0){

            cout << "Initializing ... " << endl;
            us.resize(reserve_num);
            for (int i = 0; i < reserve_num; i++)
            {
                us[i] = i;
            }
            
        };
        ~cluster()
        {
            delete this->reader;
        };
        void clusterizing();    
        void logging();
        //void logging_small_cluster(unsigned int);
        vector<unsigned int> getunionset();
        vector<unsigned int> getuscount();
        

    private:
        ifstream* reader;
        vector<unsigned int> us;
        vector<unsigned int> cunt;
        vector<unsigned int> cunt_entities;
        set <int> cluster = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        string save_path; 
        unsigned int find(unsigned int id);
        void join(unsigned int idl, unsigned int idr);
};
#endif