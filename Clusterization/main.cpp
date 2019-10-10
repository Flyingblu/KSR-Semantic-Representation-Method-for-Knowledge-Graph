#include "clusterization.hpp"
#include <iostream>
#include <fstream>
int main()
{
    cluster* c = new cluster("/home/anabur/data/save/3b/triples_filtered_shuffled.data","/home/anabur/data/save/3b/entities_more5.data", "/home/anabur/Github/logs/Clusterization_f_s_2", 119863005);
    c->clusterizing();
    //c->logging(true, false, false, false, false);
    delete c;
    //data_filter d_filter("LOAD_TRIPLEPATH", "LOAD_ENTITIES_PATH", "SAVE_PATH");
    //d_filter.load();
    //d_filter.log();
    return 0;
}
