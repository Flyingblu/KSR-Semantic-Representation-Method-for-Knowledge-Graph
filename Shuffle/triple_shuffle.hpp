#ifndef triple_shuffle_hpp
#define triple_shuffle_hpp
#include <vector>
#include <tuple>
#include <string>
#include <fstream>
using namespace std;

class triple_shuffle {

public:
    triple_shuffle(string load_path, string save_path):file(load_path, ios::binary), save_path(save_path){};
    void load();
    void Shuffle();
    void log();
    
private:
    ifstream file;
    string save_path;
    vector<tuple<unsigned int, unsigned int, unsigned int>> triples;
};
#endif