//
//  map_serializer.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "map_serializer.hpp"

void MapSerializer::map_serialize(const unordered_map<string, long long>& source_map, string path) {
    ofstream file(path, ios_base::binary | ios_base::app);
    for(auto i = source_map.begin(); i != source_map.end(); ++i) {
        
        int size = i->first.size();
        
        file.write((char *)&size, sizeof(int));
        file.write(i->first.c_str(), i->first.size());
        file.write((char *)&i->second, sizeof(long long));
    }
    file.close();
}

void MapSerializer::map_deserialize(unordered_map<string, long long>& target_map, string path) {
    ifstream file(path, ios_base::binary);

    while(file) {
        int size;
        long long id;
        file.read((char *)&size, sizeof(int));
        char *tmp_str = new char[size];
        
        file.read((char *)tmp_str, size);
        file.read((char *)&id, sizeof(long long));

        target_map[string(tmp_str, size)] = id;
        delete[] tmp_str;
    }
    file.close();
}

void MapSerializer::triple_serialize(vector<tuple<long long, long long, long long> >& triples, string path) {
    ofstream file(path, ios_base::binary | ios_base::app);
    for(auto i = triples.begin(); i != triples.end(); ++i) {
        long long tri_arr[3] = {get<0>(*i), get<1>(*i), get<2>(*i)};
        file.write((char *)tri_arr, sizeof(long long) * 3);
    }
    file.close();
}

void MapSerializer::triple_deserialize(vector<tuple<long long, long long, long long> >& triples, string path) {
    ifstream file(path, ios_base::binary);
    while (file)
    {
        long long tri_arr[3];
        file.read((char *)tri_arr, sizeof(long long) * 3);
        triples.push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));
    }
    file.close();
}