# RDF_parser

## File composition
`main.cpp`: It is a sample program to use this library. <br>
`rdf_parser.cpp`: This file defines api of the RDF parser. <br>
`rdf_parser.hpp`: This file implements the apis defined in the above file. <br>
`map_serializer.hpp`: This file defines some helper functions to save and restore certain types of map to binary files. <br>
`map_serializer.cpp`: This file implements the functions defined in the above file. <br>

### main.cpp
The main file contains a program to read from dataset, output result maps to binary files, restore them and output to a human readable json file. The purpose of this program is to check if the apis work properly on given datasets. <br>

## Compilation
Please make sure you have [nlohmann/json](https://github.com/nlohmann/json) before compiling. <br>
To compile, just run `make` in the repository directory. 
