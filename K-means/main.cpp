#include <iostream>
#include <vector>
#include <fstream>
#include "k_means.hpp"
using namespace std;

int main()
{
    k_means k(12, 76, 100);
    k.load_id("C:\\Users\\NTB11\\Desktop\\cluster_count.csv");
    k.load_table("C:\\Users\\NTB11\\Desktop\\cluster_connection_table.csv");
    k.k_means_clusterizing();
    k.log("C:\\Users\\NTB11\\Desktop\\k_means_cluster_random");
    return 0;
}

