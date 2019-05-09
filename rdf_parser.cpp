//
//  rdf_parser.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/9.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "rdf_parser.hpp"

using namespace std;

void Rdf_parser::put_to_map(map<string, int>& target_map, string& key, int& id_pos) {
    if(target_map.find(key) == target_map.end()) {
        target_map[key] = id_pos;
        id_pos += 1;
    }
}

void Rdf_parser::parse(int lines = -1) {
    string line;
    if(lines == -1) {
        for(int i = 0;getline(*(this->rdf_file), line); i++) {
        int state = 0;
        auto start = line.begin();
        for(auto i = line.begin(); i != line.end(); i++) {
            if(*i == ' ') {
                string tmp(start, i);
                if(state == 0) {
                    this->put_to_map(this->entities, tmp, this->ent_pos);
                    start = i + 1;
                    state += 1;
                } else if(state == 1) {
                    this->put_to_map(this->properties, tmp, this->prop_pos);
                    start = i + 1;
                    state += 1;
                } else if(state == 2) {
                    this->put_to_map(this->entities, tmp, this->ent_pos);
                    break;
                }
            }
        }
    }
    } else {
        for(int i = 0; i < lines && getline(*(this->rdf_file), line); i++) {
        int state = 0;
        auto start = line.begin();
        for(auto i = line.begin(); i != line.end(); i++) {
            if(*i == ' ') {
                string tmp(start, i);
                if(state == 0) {
                    this->put_to_map(this->entities, tmp, this->ent_pos);
                    start = i + 1;
                    state += 1;
                } else if(state == 1) {
                    this->put_to_map(this->properties, tmp, this->prop_pos);
                    start = i + 1;
                    state += 1;
                } else if(state == 2) {
                    this->put_to_map(this->entities, tmp, this->ent_pos);
                    break;
                }
            }
        }
    }
    }
}

void Rdf_parser::to_json(string path) {
    ofstream json_file(path);
    nlohmann::json j;
    j["entities"] = this->entities;
    j["properties"] = this->properties;
    json_file << setw(4) << j;
    json_file.close();
}
