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

using namespace std;

namespace MapSerializer {
    void map_serialize(const unordered_map<string, long long>&, string);
    void map_deserialize(unordered_map<string, long long>&, string);
}

#endif /* map_serializer_hpp */
