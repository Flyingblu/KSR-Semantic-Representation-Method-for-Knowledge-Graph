#include <iostream>
#include "progress_bar.hpp"
#include <fstream>
#include <string>

using namespace std;

int main() {
    ifstream rdf_file("/home/anabur/data/freebase-rdf-latest");
    string line;
    ProgressBar prog_bar("Lines passed:");
    prog_bar.progress_begin();
    while(getline(rdf_file, line)) {
        prog_bar.progress += 1;
    }
    prog_bar.progress_end();
    ofstream line_file("/home/anabur/Github/logs/num_line");
    line_file << prog_bar.progress;
    line_file.close();
    return 0;
}
