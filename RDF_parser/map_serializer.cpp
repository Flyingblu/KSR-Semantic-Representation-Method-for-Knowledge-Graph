//
//  map_serializer.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "map_serializer.hpp"

void MapSerializer::map_serialize(const unordered_map<string, unsigned int>& source_map, string path, string log_path) {
    ofstream file(path, ios_base::binary);
    ProgressBar prog_bar("Serializing map to binary file:", source_map.size(), log_path);
    prog_bar.progress_begin();

    size_t map_size = source_map.size();
    file.write((char *)&map_size, sizeof(size_t));

    for(auto i = source_map.begin(); i != source_map.end(); ++i) {
        
        int size = i->first.size();
        
        file.write((char *)&size, sizeof(int));
        file.write(i->first.c_str(), size);
        file.write((char *)&i->second, sizeof(unsigned int));

        prog_bar.progress += 1;
    }
    file.close();
    prog_bar.progress_end();
}

void MapSerializer::map_deserialize(unordered_map<string, unsigned int>& target_map, string path, string log_path) {
    ifstream file(path, ios_base::binary);

    size_t map_size;
    file.read((char *)&map_size, sizeof(size_t));
    ProgressBar prog_bar("Deserializing binary file to map:", map_size, log_path);
    prog_bar.progress_begin();

    for(prog_bar.progress = 0; prog_bar.progress < map_size && file; ++prog_bar.progress) {
        int size;
        unsigned int id;
        file.read((char *)&size, sizeof(int));
        char *tmp_str = new char[size];
        
        file.read((char *)tmp_str, size);
        file.read((char *)&id, sizeof(unsigned int));

        target_map[string(tmp_str, size)] = id;
        delete[] tmp_str;
    }
    file.close();
    prog_bar.progress_end();
    if(prog_bar.progress != map_size) {
        cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
    }
}

void MapSerializer::triple_serialize(vector<tuple<unsigned int, unsigned int, unsigned int> >& triples, string path, string log_path) {
    ofstream file(path, ios_base::binary);
    ProgressBar prog_bar("Serializing triples to binary file:", triples.size(), log_path);
    prog_bar.progress_begin();
    
    size_t vector_size = triples.size();
    file.write((char *)&vector_size, sizeof(size_t));

    for(auto i = triples.begin(); i != triples.end(); ++i) {

        unsigned int tri_arr[3] = {get<0>(*i), get<1>(*i), get<2>(*i)};
        file.write((char *)tri_arr, sizeof(unsigned int) * 3);

        prog_bar.progress += 1;
    }
    file.close();
    prog_bar.progress_end();
}

void MapSerializer::triple_deserialize(vector<tuple<unsigned int, unsigned int, unsigned int> >& triples, string path, string log_path) {
    ifstream file(path, ios_base::binary);

    size_t vector_size;
    file.read((char *)&vector_size, sizeof(size_t));
    ProgressBar prog_bar("Deserializing binary file to triples:", vector_size, log_path);
    prog_bar.progress_begin();

    for(prog_bar.progress = 0; prog_bar.progress < vector_size && file; ++prog_bar.progress) {

        unsigned int tri_arr[3];
        file.read((char *)tri_arr, sizeof(unsigned int) * 3);
        triples.push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));
    }
    file.close();
    prog_bar.progress_end();

    if(prog_bar.progress != vector_size) {
        cerr << "triple_deserialize: Something wrong in binary file reading. " << endl;
    }
}

void MapSerializer::map_compare(const unordered_map<string, unsigned int>& a, const unordered_map<string, unsigned int>& b, string path, string name)
{
    ofstream file(path + name + "_compare", ios::out);

    
    map <string, unsigned int> map_a(a.begin(), a.end());
    map <string, unsigned int> map_b(b.begin(), b.end());

    file << "unordered_map " << name << "_a size : " << a.size() << endl;
    file << "unordered_map " << name << "_b size : " << b.size() << endl;
    file << endl;
    file << "map " << name << "_a size : " << map_a.size() << endl;
    file << "map " << name << "_b size : " << map_b.size() << endl;
    file << endl;

    auto iter_a = map_a.begin();
    auto iter_b = map_b.begin();
    while(iter_a != map_a.end())
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
    if (iter_b != map_b.end())
    {
        file << "map " << name <<"_b is not itered to end.\n";
    }
    file.close();

}

void MapSerializer::triples_to_text(vector<tuple<unsigned int, unsigned int, unsigned int> >& triples, string path)
{
    ofstream file(path, ios::out);
    ProgressBar prog_bar("Serializing triples vector to text file:", triples.size());
    prog_bar.progress_begin();

    for (auto i = triples.begin(); i != triples.end(); i++)
    {
        file << get<0>(*i) << "\t" << get<1>(*i) << "\t" << get<2>(*i) << endl;
        prog_bar.progress += 1;
    }
    file.close();
    prog_bar.progress_end();
}

void MapSerializer::map_to_text(const unordered_map<string, unsigned int>& source_map, string path, bool inmap)
{
    ofstream file(path, ios::out);
    ProgressBar prog_bar("Serializing map to text file:", source_map.size());
    prog_bar.progress_begin();
    if (inmap)
    {
        printf("%c[2K", 27);
        cout << "Adding unordered_map into map" <<endl << flush;
        map <string, unsigned int> ordered_map(source_map.begin(), source_map.end());
        auto iter = ordered_map.begin();
        while(iter != ordered_map.end())
        {
            file << iter->first << "\t" << iter->second << endl;
            iter++;
            prog_bar.progress += 1;
        }

    }
    else
    {
        auto iter = source_map.begin();
    
        while(iter != source_map.end())
        {
            file << iter->first << "\t" << iter->second << endl;
            iter++;
            prog_bar.progress += 1;
        }
    }
    
    file.close();
    prog_bar.progress_end();
}
