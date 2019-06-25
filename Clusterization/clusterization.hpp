#ifndef clusterization_hpp
#define clusterization_hpp

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <iostream>
#include <tuple>
#include <unordered_map>
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
        cluster(string read_path, string entities_path, string save_path, unsigned int reserve_num): 
                reader(new ifstream(read_path, ios::binary)), 
                save_path(save_path){

            cout << "Initializing ... " << endl;
            ifstream file(entities_path, ios::binary);
            us.reserve(reserve_num);
            //cunt_entities.reserve(reserve_num);
            size_t map_size;
            file.read((char*)& map_size, sizeof(size_t));
            for (int i = 0; i < map_size; ++i)
            {
                unsigned int entities_id;
                file.read((char*)& entities_id, sizeof(unsigned int));
                us[entities_id] = entities_id;
                cunt[entities_id] = 1;
                Entities entities(entities_id);
                //cunt_entities[entities_id] = entities;
            }
        };
        ~cluster()
        {
            delete this->reader;
        };
        void clusterizing();    
        void logging(bool, bool, bool, bool, bool);
        void log_cluster();
        void cluster_connect();
        template <class T, typename Proc>
        void vector_serializer(vector<T>& vec, string save_path, Proc p);
        unordered_map<unsigned int, unsigned int> getunionset();
        unordered_map<unsigned int, unsigned int> getuscount();
        

    private:
        ifstream* reader;
        unordered_map<unsigned int, unsigned int> us;
        unordered_map<unsigned int, unsigned int> cunt;
        //unordered_map<unsigned int, Entities> cunt_entities;
        unordered_map<unsigned int, unordered_map<unsigned int, unsigned int>> connect; 
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