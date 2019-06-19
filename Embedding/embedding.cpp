#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"

using namespace std;

int main() {
    srand(time(nullptr));
    Dataset data("Freebase-3b", "/home/anabur/data/save/", "3b/", "3b/", false);
    return 0;
}