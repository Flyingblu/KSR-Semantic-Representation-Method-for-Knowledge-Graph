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
#include <map>
#include <string>
#include <iomanip>
#include "json.h"

using namespace std;

class RDFParser {
public:
    RDFParser(string path): rdf_file(new ifstream(path)) {} ;
    ~RDFParser() {
        delete this->rdf_file;
    }
    void parse(long long lines=-1, long long batch_size=1e8);
    void to_json(string);
private:
    ifstream * rdf_file;
    long long ent_pos = 0;
    long long prop_pos = 0;
    map<string, long long> entities;
    map<string, long long> properties;
    void put_to_map(map<string, long long>&, string&, long long&);
    void triple_parser(string&);
    bool batch_parser(long long);
};

#endif /* rdf_parser_hpp */
