#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include "RDF_parser/progress_bar.hpp"

using namespace std;

void load_entities(vector<unsigned int>*& target, string entities_path, string log_path="") {
    ifstream file(entities_path, ios_base::binary);

    size_t map_size;
    file.read((char *)&map_size, sizeof(size_t));
    cout << "Creating vector, size: " << map_size << endl;
    target = new vector<unsigned int>(map_size, 0);
    file.close();
}

unsigned int count_entities(vector<unsigned int>& entities, string triples_path, string log_path="") {
    ifstream file(triples_path, ios_base::binary);

    size_t triple_size;
    file.read((char *)&triple_size, sizeof(size_t));

    ProgressBar prog_bar("Counting entities", triple_size, log_path);
    prog_bar.progress_begin();

    for(prog_bar.progress = 0; prog_bar.progress < triple_size && file; ++prog_bar.progress) {

        unsigned int tri_arr[3];
        file.read((char *)tri_arr, sizeof(unsigned int) * 3);
        ++entities[tri_arr[0]];
        ++entities[tri_arr[2]];
    }
    file.close();
    prog_bar.progress_end();

    if(prog_bar.progress != triple_size) {
        cerr << "triple_deserialize: Something wrong in binary file reading. " << endl;
    }
    return triple_size;
}

void save_count_result(string save_path, vector<unsigned int>& source) {

    ofstream fout(save_path, ios_base::binary);
    size_t total_size = source.size();
    fout.write((char *)&total_size, sizeof(total_size));

    ProgressBar prog_bar("Saving counting results", total_size);
    prog_bar.progress_begin();

    unsigned int tmp;
    for (prog_bar.progress = 0; prog_bar.progress < source.size(); ++prog_bar.progress) {
        tmp = source[prog_bar.progress];
        fout.write((char *)&tmp, sizeof(tmp));
    }
    prog_bar.progress_end();
    fout.close();
}

unsigned int load_count_result(string load_path, string triple_path, vector<unsigned int>*& target) {
    ifstream fin(load_path, ios_base::binary);
    size_t total_size;
    fin.read((char *)&total_size, sizeof(total_size));

    cout << "Creating vector of size: " << total_size << endl;
    target = new vector<unsigned int>(total_size);

    ProgressBar prog_bar("Loading counting results", total_size);
    prog_bar.progress_begin();

    unsigned int tmp;
    for (prog_bar.progress = 0; prog_bar.progress < total_size; ++prog_bar.progress) {
        fin.read((char *)&tmp, sizeof(tmp));
        (*target)[prog_bar.progress] = tmp;
    }
    prog_bar.progress_end();
    fin.close();

    ifstream file(triple_path, ios_base::binary);

    size_t triple_size;
    file.read((char *)&triple_size, sizeof(size_t));
    return triple_size;
}

void entities_to_multimap(vector<unsigned int>& source, multimap<unsigned int, unsigned int, greater<unsigned int> >& target, string log_path="") {
    ProgressBar prog_bar("Transfering vector to multimap", source.size(), log_path);
    prog_bar.progress_begin();

    for (prog_bar.progress = 0; prog_bar.progress < source.size(); ++prog_bar.progress) {
        target.insert(make_pair(source[prog_bar.progress], prog_bar.progress));
    }
    source.clear();
    prog_bar.progress_end();
}

void erase_trivial_entities(multimap<unsigned int, unsigned int, greater<unsigned int> >& entities, unsigned int total_size) {
    // Since the head and tail of triples are all counted, a triple is counted twice, 
    // so the target size is 2 times 20% of total triple size
    unsigned int target_size = total_size * 0.2 * 2;
    unsigned int count = 0;
    auto it_begin = entities.begin();

    ProgressBar prog_bar("Calculating occurance");
    prog_bar.progress = 0;
    prog_bar.progress_begin();

    for (;it_begin != entities.end(); ++it_begin) {
        count += it_begin->first;
        if (count >= target_size) {
            break;
        }
        ++prog_bar.progress;
    }
    ++it_begin;
    entities.erase(it_begin, entities.end());
    prog_bar.progress_end();
}

void check_result_entities_occurance(multimap<unsigned int, unsigned int, greater<unsigned int> >& entities, string save_path) {
    ProgressBar prog_bar("Checking occurance", entities.size());
    prog_bar.progress = 0;
    prog_bar.progress_begin();

    ofstream fout(save_path);

    unsigned int count;
    for (auto i = entities.begin(); i != entities.end(); ++i) {
        count += i->first;
        fout << i->second << "," << i->first << endl;
        ++prog_bar.progress;
    }
    fout << count;
    prog_bar.progress_end();
    cout << endl << "Total occurance: " << count << endl;
}

void make_connection_table(vector<vector<unsigned int> >*& table, unordered_map<unsigned int, unsigned int>& id_to_index, multimap<unsigned int, unsigned int, greater<unsigned int> > entities) {
    cout << "Making connection table..." << endl;
    table = new vector<vector<unsigned int> >(entities.size() + 1, vector<unsigned int>(entities.size() + 1, 0));
    int cnt = 0;
    for (auto i = entities.begin(); i != entities.end(); ++i, ++cnt) {
        id_to_index[i->second] = cnt;
        (*table)[0][cnt + 1] = i->second;
        (*table)[cnt + 1][0] = i->second;
    }
}

void build_connection_table(vector<vector<unsigned int> >& table, unordered_map<unsigned int, unsigned int>& id_to_index, string triple_path, string log_path="") {
    ifstream file(triple_path, ios_base::binary);

    size_t triple_size;
    file.read((char *)&triple_size, sizeof(size_t));

    ProgressBar prog_bar("Building connection table", triple_size, log_path);
    prog_bar.progress_begin();

    for(prog_bar.progress = 0; prog_bar.progress < triple_size && file; ++prog_bar.progress) {

        unsigned int tri_arr[3];
        file.read((char *)tri_arr, sizeof(unsigned int) * 3);
        auto first_entity = id_to_index.find(tri_arr[0]);
        auto second_entity = id_to_index.find(tri_arr[2]);
        if (first_entity == id_to_index.end() || second_entity == id_to_index.end()) {
            continue;
        }

        if (first_entity->first > second_entity->first) {
            ++table[second_entity->second + 1][first_entity->second + 1];
        } else {
            ++table[first_entity->second + 1][second_entity->second + 1];
        }
    }
    file.close();
    prog_bar.progress_end();

    if(prog_bar.progress != triple_size) {
        cerr << "triple_deserialize: Something wrong in binary file reading. " << endl;
    }
}

void table_to_csv(vector<vector<unsigned int> >& table, string save_path) {

    cout << "Saving to file..." << endl;
    ofstream fout(save_path);

    for (auto i = table.begin(); i != table.end(); ++i) {
        for (auto j = i->begin(); j != i->end(); ++j) {
            if (j == i->begin()) {
                fout << *j;
                continue;
            }
            fout << "," << *j;
        }
        fout << endl;
    }
}

int main() {
    multimap<unsigned int, unsigned int, greater<unsigned int> > entities;
    vector<unsigned int>* entities_vector;
    unordered_map<unsigned int, unsigned int> id_to_index;
    vector<vector<unsigned int> >* connection_table;
    unsigned int total_size;

    total_size = load_count_result("/home/anabur/data/save/3b/occurance.data", "/home/anabur/data/save/3b/triples.data", entities_vector);
    entities_to_multimap(*entities_vector, entities);
    erase_trivial_entities(entities, total_size);
    check_result_entities_occurance(entities, "/home/anabur/data/save/3b/top_entities.csv");
    make_connection_table(connection_table, id_to_index, entities);
    build_connection_table(*connection_table, id_to_index, "/home/anabur/data/save/3b/triples.data");
    table_to_csv(*connection_table, "/home/anabur/data/save/3b/connection_table.csv");
    return 0;
}

