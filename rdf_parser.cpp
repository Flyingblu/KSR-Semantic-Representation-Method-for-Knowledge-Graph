//
//  rdf_parser.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/9.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "rdf_parser.hpp"

using namespace std;

void RDFParser::put_to_map(map<string, long long>& target_map, string& key, long long& id_pos) {

    if(target_map.find(key) == target_map.end()) {

        target_map[key] = id_pos;
        id_pos += 1;
    }
}

void RDFParser::triple_parser(string& triple) {
    int state = 0;
    auto start = triple.begin();

    for(auto i = triple.begin(); i != triple.end(); i++) {

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

bool RDFParser::batch_parser(long long batch_size) {
    string line;
    bool end = true;

    for(long long i = 0; i < batch_size; ++i) {

        if(getline(*(this->rdf_file), line)) {
            this->triple_parser(line);

        } else {
            end = false;
            break;
        }
    }
    return end;
}

void RDFParser::parse(long long lines=-1, long long batch_size=1e8) {

    string line;

    if(lines == -1) {
        
        while(this->batch_parser(batch_size)) {
            this->to_json;
            this->entities.clear();
            this->properties.clear();
        }

    } else {
        long long pos = 0;
        bool end = true;

        while(end) {

            if(pos + batch_size < lines) {

                end = this->batch_parser(batch_size);
                pos += batch_size;

            } else {

                end = false;
                this->batch_parser(lines - pos);
            }

            this->to_json;
            this->entities.clear();
            this->properties.clear();
        }
    }
}

void RDFParser::to_json(string path) {
    ofstream json_file(path);
    nlohmann::json j;
    j["entities"] = this->entities;
    j["properties"] = this->properties;
    json_file << setw(4) << j;
    json_file.close();
}
