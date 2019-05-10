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

void triple_parser(string triple) {
}

int main(int argc, const char * argv[]) {
    RDFParser rdf("/home/flyingblu/Downloads/latest-lexemes.nt");
    rdf.parse();
    rdf.to_json("/home/flyingblu/Downloads/out.json");
    return 0;
}
