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
#include "json.h"
#include "map_serializer.hpp"
#include <tuple>

using namespace std;

class RDFParser {
public:
    
    RDFParser(string read_path, string save_path): rdf_file(new ifstream(read_path)), save_path(save_path) {} ;
    ~RDFParser() {
        delete this->rdf_file;
    }
    
    void parse(long long lines=-1, long long batch_size=1e8, bool save_file=false);
    void retrivial();
    void to_json(string);
    
    string save_path;
    
private:
    
    ifstream * rdf_file;
    long long ent_pos = 0;
    long long prop_pos = 0;
    unordered_map<string, long long> entities;
    unordered_map<string, long long> properties;
    vector<tuple<long long, long long, long long> > triples;
    long long put_to_map(unordered_map<string, long long>&, string&, long long&);
    void triple_parser(string&);
    bool batch_parser(long long);
    
};

#endif /* rdf_parser_hpp */
