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

class Rdf_parser {
public:
    Rdf_parser(string path): rdf_file(new ifstream(path)) {} ;
    ~Rdf_parser() {
        delete this->rdf_file;
    }
    void parse(int);
    void to_json(string);
private:
    ifstream * rdf_file;
    int ent_pos = 0;
    int prop_pos = 0;
    map<string, int> entities;
    map<string, int> properties;
    void put_to_map(map<string, int>&, string&, int&);
};

#endif /* rdf_parser_hpp */
