#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"

using namespace std;

int main() {
    srand(time(nullptr));
    Dataset data("Freebase-3b", "/home/anabur/data/save/", "3b/", "3b/", "/home/anabur/Github/logs/loading_log/", false);
    TaskType task = General;
    MFactorE model(data, task, "/home/anabur/Github/logs/training_log/", 3, 0.01, 1.5, 1, 4);
    return 0;
}