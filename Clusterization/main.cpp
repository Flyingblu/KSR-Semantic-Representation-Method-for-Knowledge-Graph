#include "clusterization.hpp"
#include <iostream>
#include <fstream>
int main()
{
    cluster* c = new cluster("/PATH/TO/TRIPLE.DATA", "/PATH/TO/LOGFILE", ENTITIES_SIZE);
    c->clusterizing();
    c->logging(false, false, false, false, true);
    delete c;
    data_filter d_filter("LOAD_TRIPLEPATH", "LOAD_ENTITIES_PATH", "SAVE_PATH");
    d_filter.load();
    d_filter.log();
    return 0;
}
