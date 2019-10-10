#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <boost/progress.hpp>

using namespace std;

void data_filter(string triple_path, string entity_path, string log_path)
{
    ifstream triple_reader(triple_path, ios_base::binary);
    ifstream entity_reader(entity_path, ios_base::binary);
    ofstream triple_writer(log_path, ios_base::binary);

    unordered_set<unsigned int>bad_ent;
    vector<tuple<unsigned int, unsigned int, unsigned int>> triples;
    size_t triple_size;
    triple_reader.read((char*)&triple_size, sizeof(size_t));
    size_t set_size;
    entity_reader.read((char*)&set_size, sizeof(size_t));
    cout << "triple_size : " << triple_size << endl;
    cout << "entity_size : " << set_size << endl;
    bad_ent.reserve(set_size);

    cout << "Loading bad_entity ..." << endl;
    boost::progress_display entity_load_progress(set_size);

    for (unsigned int i = 0; i < set_size; ++i) 
    {
        unsigned int entity_id;
        entity_reader.read((char*)&entity_id, sizeof(unsigned int));
        bad_ent.insert(entity_id);
        ++entity_load_progress;
    }
    entity_reader.close();

    cout << "Filtering triples ..." << endl;
    boost::progress_display triples_filter_progress(triple_size);
    unsigned int count = 0;
    unsigned int steps = 0;
    for (unsigned int i = 0; i < triple_size; ++i)
    {
        unsigned int tri_arr[3];
        triple_reader.read((char*)tri_arr, sizeof(unsigned int) * 3);
        ++triples_filter_progress;
        ++steps;
        if (bad_ent.find(tri_arr[0]) != bad_ent.end() || bad_ent.find(tri_arr[2]) != bad_ent.end())
        {
            count++;
            continue;
            
        }
        else
        {
            triples.push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));
        }
        
    }
    triple_reader.close();

    cout << "Abandoned triples size : " << count << endl;
    cout << "new triples size : " << triples.size() << endl;
    cout << "steps : " << steps << endl;
    
    size_t new_triple_size = triples.size();
    triple_writer.write((char*)&new_triple_size, sizeof(size_t));
    cout << "Logging new triples ..." << endl;
    boost::progress_display new_triples_logging_progress(new_triple_size);
    
    for (auto i: triples)
    {
        unsigned int tri_arr[3] = {get<0>(i), get<1>(i), get<2>(i)};
        triple_writer.write((char*)tri_arr, sizeof(unsigned int) * 3);
        ++new_triples_logging_progress;
    }
    triple_writer.close();
}

int main()
{
    data_filter("/home/anabur/data/save/3b/triples_shuffled.data", "/home/anabur/data/save/3b/entities_less5.data", "/home/anabur/data/save/3b/triple_shuffled_filter_test.data");
}