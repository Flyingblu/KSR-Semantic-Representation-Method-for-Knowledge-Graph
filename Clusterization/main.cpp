#include "clusterization.hpp"
#include <iostream>
#include <fstream>
int main()
{
    cluster c("/PATH/TO/TRIPLE.DATA", "/PATH/TO/LOGFILE", ENTITIES_SIZE);
    c.clusterizing();
    c.logging(true);

    return 0;
}
