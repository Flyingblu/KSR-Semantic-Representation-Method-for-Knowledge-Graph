#include "triple_shuffle.hpp"
using namespace std;

int main()
{
    triple_shuffle tri_shuffle("/home/anabur/data/save/3b/triples_filtered.data", "/home/anabur/data/save/3b/triples_filtered");
    tri_shuffle.load();
    tri_shuffle.Shuffle();
    tri_shuffle.log();
    return 0;
}

