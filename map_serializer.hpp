//
//  map_serializer.hpp
//  Trival_RDF_parser
//
//  Created by 王嘉睿 on 2019/5/10.
//  Copyright © 2019 Jerry's World. All rights reserved.
//

#ifndef map_serializer_hpp
#define map_serializer_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <vector>
#include "progress_bar.hpp"

using namespace std;

namespace MapSerializer {
    void map_serialize(const unordered_map<string, unsigned int>&, string);
    void map_deserialize(unordered_map<string, unsigned int>&, string);
    void triple_serialize(vector<tuple<unsigned int, unsigned int, unsigned int> >&, string);
    void triple_deserialize(vector<tuple<unsigned int, unsigned int, unsigned int> >&, string);
    void map_compare(const unordered_map<string, unsigned int>& , const unordered_map<string, unsigned int>&, string, string);
    void map_to_text(const unordered_map<string, unsigned int>&, string);
}

#endif /* map_serializer_hpp */
