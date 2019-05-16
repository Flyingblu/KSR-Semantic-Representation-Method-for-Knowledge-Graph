//
//  rdf_parser.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/9.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include "rdf_parser.hpp"

using namespace std;

unsigned int RDFParser::put_to_map(unordered_map<string, unsigned int>& target_map, string& key, unsigned int& id_pos) {

    auto i = target_map.find(key);
    if(i == target_map.end()) {

        target_map[key] = id_pos;
        id_pos += 1;
        return id_pos - 1;
    }
    return i->second;
}

void RDFParser::triple_parser(string& triple) {
    int state = 0;
    int f_state = 0;
    unsigned int triple_arr[3];
    auto add_to_map = [&](string::iterator start, string::iterator end) -> void {
        string tmp(start, end);
        switch (state)
        {
        case 0:
            triple_arr[state] = this->put_to_map(this->entities, tmp, this->ent_pos);
            break;
        
        case 1:
            triple_arr[state] = this->put_to_map(this->properties, tmp, this->prop_pos);
            break;

        case 2:
            triple_arr[state] = this->put_to_map(this->entities, tmp, this->ent_pos);
            break;

        default:
            break;

        }
        ++state;
        return;
    };

    auto start = triple.begin();

    for(auto i = triple.begin(); i != triple.end(); i++) {

        if (f_state == 0) {

            if (*i == '<') {

                f_state = 1;
                start = i;

            } else if (*i == '"') {

                f_state = 2;
                start = i;

            }
        } else if (f_state == 1) {

            if (*i == ' ' || *i == '\t') {

                f_state = 0;
                add_to_map(start, i);

                if (state == 3) {
                    this->triples.push_back(make_tuple(triple_arr[0], triple_arr[1], triple_arr[2]));
                    break;
                }

            }
        } else if (f_state == 2) {

            if (*i == '"') {

                f_state = 1;

            } else if (*i == '\\') {

                f_state = 3;

            }
        } else if (f_state == 3) {

            f_state = 2;

        }
    }
}

void RDFParser::parse(unsigned int lines, bool save_file) {

    string line;
    ProgressBar prog_bar("Triples parsed:");
    unsigned int cnt_lines;

    if(lines == -1) {
        // lines is -1 means parsing the entire file
        cnt_lines = 0;
        while(getline(*(this->rdf_file), line)) {

            ++cnt_lines;
            this->triple_parser(line);

            if(cnt_lines % 100 == 0) {
                prog_bar.progress_increment(100);
            }
        }
    } else {
        // When lines is not -1 it means parsing that many lines
        //Here cnt_lines starts counting at 1 to make the progress bar behave normally
        for(cnt_lines = 1;getline(*(this->rdf_file), line) && cnt_lines <= lines; ++cnt_lines) {

            this->triple_parser(line);

            if(cnt_lines % 100 == 0) {
                prog_bar.progress_increment(100);
            }
        }
    }

    if(save_file) {
            MapSerializer::map_serialize(this->entities, this->save_path + "entities.data");
            MapSerializer::map_serialize(this->properties, this->save_path + "properties.data");
            MapSerializer::triple_serialize(this->triples, this->save_path + "triples.data");
    }

    prog_bar.progress_end();
}

void RDFParser::to_json(string path) {

    cout << "Saving to JSON..." << endl;

    ofstream json_file(path);
    nlohmann::json j;
    j["entities"] = this->entities;
    j["properties"] = this->properties;
    j["triples"] = this->triples;
    json_file << setw(4) << j;
    json_file.close();
}

void RDFParser::to_text(string path)
{
    MapSerializer::map_to_text(this->entities, path + "entities.text");
    MapSerializer::map_to_text(this->properties, path + "properties.text");
}

void RDFParser::clear_data() {
    this->entities.clear();
    this->properties.clear();
    this->triples.clear();
}

void RDFParser::retrivial() {
    MapSerializer::map_deserialize(this->entities, this->save_path + "entities.data");
    MapSerializer::map_deserialize(this->properties, this->save_path + "properties.data");
    MapSerializer::triple_deserialize(this->triples, this->save_path + "triples.data");
}
