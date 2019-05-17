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
    rdf.parse(-1, true);
    rdf.to_text("/DIRECTORY/PATH/OF/TARGET_FILE/rdf_", true);
    rdf.clear_data();
    rdf.retrivial();
    rdf.to_text("/DIRECTORY/PATH/OF/TARGET_FILE/rdf_1_", true);
    //MapSerializer::map_compare(rdf.entities, rdf_1.entities,"/DIRECTORY/PATH/OF/TARGET_FILE/" ,"entities");
    //MapSerializer::map_compare(rdf.properties, rdf_1.properties, "/DIRECTORY/PATH/OF/TARGET_FILE/", "properties");
    //rdf.to_json("/PATH/TO/JSON_FILE.json");
    return 0;
}
