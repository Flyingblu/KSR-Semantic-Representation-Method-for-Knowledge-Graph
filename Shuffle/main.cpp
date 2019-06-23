#include "triple_shuffle.hpp"
using namespace std;

int main()
{
    triple_shuffle tri_shuffle("LOAD_PATH", "SAVE_PATH");
    tri_shuffle.load();
    tri_shuffle.Shuffle();
    tri_shuffle.log();
    return 0;
}

