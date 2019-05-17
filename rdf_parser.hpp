//
//  rdf_parser.hpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/9.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#ifndef rdf_parser_hpp
#define rdf_parser_hpp

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <iomanip>
#include <tuple>
#include "json.h"
#include "map_serializer.hpp"
#include "progress_bar.hpp"

using namespace std;

class RDFParser {
public:
    
    RDFParser(string read_path, string save_path): rdf_file(new ifstream(read_path)), save_path(save_path) {} ;
    ~RDFParser() {
        delete this->rdf_file;
    }
    
    void parse(unsigned int lines=-1, bool save_file=false);
    void retrivial();
    void to_json(string);
    void to_text(string, bool);
    void clear_data();
    unordered_map<string, unsigned int> entities;
    unordered_map<string, unsigned int> properties;
    vector<tuple<unsigned int, unsigned int, unsigned int> > triples;
    string save_path;
    
private:
    
    ifstream * rdf_file;
    unsigned int ent_pos = 0;
    unsigned int prop_pos = 0;
    unsigned int put_to_map(unordered_map<string, unsigned int>&, string&, unsigned int&);
    void triple_parser(string&);
    
};

#endif /* rdf_parser_hpp */
