#include "triple_shuffle.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include "../RDF_parser/progress_bar.hpp"
using namespace std;

void triple_shuffle::load()
{
    if (file.is_open())
    {
        size_t vector_size;
        file.read((char*)& vector_size, sizeof(size_t));
        cout << "Resizeing vector ..." << endl;
        triples.reserve(vector_size);

        ProgressBar pbar("Desrializing binary to triples ... ", vector_size);
        pbar.progress_begin();
        for (pbar.progress = 0; pbar.progress < vector_size && !file.eof(); ++pbar.progress)
        {
            unsigned int tri_arr[3];
            file.read((char *)tri_arr, sizeof(unsigned int) * 3);
            triples.push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));
        }
        pbar.progress_end();
        file.close();
    }
    else
    {
        cout << "file open error" << endl;
    }
    
}

void triple_shuffle::Shuffle()
{
    size_t vector_size = triples.size();
    cout << "shuffleing ..." << endl;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(triples.begin(), triples.end(), default_random_engine(seed));
}

void triple_shuffle::log()
{
    ofstream file(save_path + "_shuffled.data", ios::binary);
    ProgressBar prog_bar("Serializing triples to binary file:", triples.size());
    prog_bar.progress_begin();
    
    size_t vector_size = triples.size();
    file.write((char *)&vector_size, sizeof(size_t));

    for(auto i = triples.begin(); i != triples.end(); ++i) {

        unsigned int tri_arr[3] = {get<0>(*i), get<1>(*i), get<2>(*i)};
        file.write((char *)tri_arr, sizeof(unsigned int) * 3);

        prog_bar.progress += 1;
    }
    file.close();
    prog_bar.progress_end();
}

