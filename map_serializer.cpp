//
//  map_serializer.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "map_serializer.hpp"

void MapSerializer::map_serialize(const unordered_map<string, unsigned int>& source_map, string path) {
    ofstream file(path, ios_base::binary);
    ProgressBar prog_bar("Serializing map to binary file:", source_map.size());
    unsigned int cnt = 0;

    size_t map_size = source_map.size();
    file.write((char *)&map_size, sizeof(size_t));

    for(auto i = source_map.begin(); i != source_map.end(); ++i) {
        
        ++cnt;
        int size = i->first.size();
        
        file.write((char *)&size, sizeof(int));
        file.write(i->first.c_str(), size);
        file.write((char *)&i->second, sizeof(unsigned int));

        if(cnt % 100 == 0) {
            prog_bar.progress_increment(100);
        }
    }
    file.close();
    prog_bar.progress_end();
}

void MapSerializer::map_deserialize(unordered_map<string, unsigned int>& target_map, string path) {
    ifstream file(path, ios_base::binary);

    size_t map_size;
    file.read((char *)&map_size, sizeof(size_t));
    ProgressBar prog_bar("Deserializing binary file to map:", map_size);

    size_t i;

    for(i = 0; i < map_size && file; ++i) {
        int size;
        unsigned int id;
        file.read((char *)&size, sizeof(int));
        char *tmp_str = new char[size];
        
        file.read((char *)tmp_str, size);
        file.read((char *)&id, sizeof(unsigned int));

        target_map[string(tmp_str, size)] = id;
        delete[] tmp_str;

        if(i % 100 == 0) {
            prog_bar.progress_increment(100);
        }
    }
    file.close();
    prog_bar.progress_end();
    if(i != map_size) {
        cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
    }
}

void MapSerializer::triple_serialize(vector<tuple<unsigned int, unsigned int, unsigned int> >& triples, string path) {
    ofstream file(path, ios_base::binary);
    ProgressBar prog_bar("Serializing triples to binary file:", triples.size());
    unsigned int cnt = 0;
    
    size_t vector_size = triples.size();
    file.write((char *)&vector_size, sizeof(size_t));

    for(auto i = triples.begin(); i != triples.end(); ++i) {

        ++cnt;
        unsigned int tri_arr[3] = {get<0>(*i), get<1>(*i), get<2>(*i)};
        file.write((char *)tri_arr, sizeof(unsigned int) * 3);

        if (cnt % 100 == 0) {
            prog_bar.progress_increment(100);
        }
    }
    file.close();
    prog_bar.progress_end();
}

void MapSerializer::triple_deserialize(vector<tuple<unsigned int, unsigned int, unsigned int> >& triples, string path) {
    ifstream file(path, ios_base::binary);

    size_t vector_size;
    file.read((char *)&vector_size, sizeof(size_t));
    ProgressBar prog_bar("Deserializing binary file to triples:", vector_size);
    size_t cnt = 0;

    for(cnt=0; cnt < vector_size && file; ++cnt) {

        unsigned int tri_arr[3];
        file.read((char *)tri_arr, sizeof(unsigned int) * 3);
        triples.push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));

        if (cnt % 100 == 0) {
            prog_bar.progress_increment(100);
        }
    }
    file.close();
    prog_bar.progress_end();
    if(cnt != vector_size) {
        cerr << "triple_deserialize: Something wrong in binary file reading. " << endl;
    }
}

void MapSerializer::map_compare(const unordered_map<string, long long>& a, const unordered_map<string, long long>& b, string path, string name)
{
    ofstream file(path + name + "_compare", ios::out);

    auto iter_a = a.begin();
    auto iter_b = b.begin();

    file << "unordered_map " << name << "_a size : " << a.size() << endl;
    file << "unordered_map " << name << "_b size : " << b.size() << endl;

    while(iter_a != a.end())
    {
        if (iter_a->first != iter_b->first || iter_a->second != iter_b->second)
        {
                file << iter_a->first << "\t" << iter_a->second << endl;
                file << iter_b->first << "\t" << iter_b->second << endl;
                file << endl;

        }
        iter_a++;
        iter_b++;
        
    }
    if (iter_b != b.end())
    {
        file << "unordered_map " << name <<"_b is not itered to end.\n";
    }
    file.close();

}

void MapSerializer::map_to_text(const unordered_map<string, long long>& source_map, string path)
{
    ofstream file(path, ios::out);
    ProgressBar prog_bar("Serializing map to text file:", source_map.size());
    auto iter = source_map.begin();
    long long cnt = 0;

    while(iter != source_map.end())
    {
        ++cnt;
        file << iter->first << "\t" << iter->second << endl;
        iter++;
        if(cnt % 100 == 0) {
            prog_bar.progress_increment(100);
        }
    }
    file.close();
    prog_bar.progress_end();
}
