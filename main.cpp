//
//  main.cpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/9.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include "rdf_parser.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    RDFParser rdf("/PATH/TO/RDF_FILE", "/DIRECTORY/PATH/OF/TARGET_FILE/");
    rdf.parse(-1, 1e8, true);
    rdf.retrivial();
    rdf.to_json("/PATH/TO/JSON_FILE.json");
    return 0;
}
