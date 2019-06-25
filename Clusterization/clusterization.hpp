#ifndef clusterization_hpp
#define clusterization_hpp

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <iostream>
#include <tuple>
using namespace std;

class Entities
{
    public:
        Entities(){};
        Entities(unsigned int id): id(id), cunt_entities(0){};
        
        unsigned int id;
        unsigned int cunt_entities;
};
class cluster
{
    public:
        cluster(string read_path, string save_path, unsigned int reserve_num): 
                reader(new ifstream(read_path, ios::binary)), 
                save_path(save_path), 
                cunt(reserve_num, 1){

            cout << "Initializing ... " << endl;
            us.resize(reserve_num);
            for (int i = 0; i < reserve_num; ++i)
            {
                us[i] = i;
            }
            cunt_entities.resize(reserve_num);
            for (unsigned int i = 0; i < reserve_num; ++i) 
            {
                Entities entities(i);
                cunt_entities[i] = entities;
            }
            
        };
        ~cluster()
        {
            delete this->reader;
        };
        void clusterizing();    
        void logging(bool, bool, bool, bool, bool);
        void log_cluster();
        void log_entities_fre(bool);
        template <class T, typename Proc>
        void vector_serializer(vector<T>& vec, string save_path, Proc p);
        vector<unsigned int> getunionset();
        vector<unsigned int> getuscount();
        

    private:
        ifstream* reader;
        vector<unsigned int> us;
        vector<unsigned int> cunt;
        vector<Entities> cunt_entities;
        //vector<vector<unsigned int>> connect;
        string save_path; 
        unsigned int find(unsigned int id);
        void join(unsigned int idl, unsigned int idr);
};

//delete all triples containing entites whose frequency are less or equal to 5
class data_filter
{
    public:
        data_filter();
        data_filter(string load_path_of_triples, string load_path_of_entites, string save_path):
        load_path_triples(load_path_of_triples), 
        load_path_entites(load_path_of_entites),
        save_path(save_path)
        {};
        void load();
        void log();
    private:
        string load_path_triples;
        string load_path_entites;
        string save_path;
        vector<tuple<unsigned int, unsigned int, unsigned int>> triples;
        set<unsigned int> bad_ent; //entities less than 5

};
#endif