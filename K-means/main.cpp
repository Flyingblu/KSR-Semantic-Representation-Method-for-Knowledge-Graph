#include <iostream>
#include <vector>
#include <fstream>
#include "k_means.hpp"
using namespace std;

int main()
{
    
    k_means k(8, 76, 10000000);
    k.load_id("PATH_ID");
    k.load_table("PATH_TABLE");
    k.concurrent_run();
    k.log("SAVE_OF_PATH");
    
    return 0;
}

