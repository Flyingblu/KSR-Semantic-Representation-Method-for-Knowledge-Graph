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
    RDFParser rdf("PATH_TO_RDF_FILE", "PATH_TO_SAVE_FILE");
    rdf.parse(-1, 1e8, true);
    rdf.retrivial();
    rdf.to_json("PATH_TO_JSON_FILE");
    return 0;
}
