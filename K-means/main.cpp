#include <iostream>
#include <vector>
#include <fstream>
#include "k_means.hpp"
using namespace std;

int main()
{
    k_means k(8, 76, 1000000);
    k.load_id("/home/anabur/data/save/3b/analysis/cluster_count.csv");
    k.load_table("/home/anabur/data/save/3b/analysis/cluster_connection_table.csv");
    k.concurrent_run();
    k.log("/home/anabur/data/save/3b/analysis/k_means_cluster_random_1M_1");
    return 0;
}

