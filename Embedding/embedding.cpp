#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"

using namespace std;

int main()
{
    srand(time(nullptr));
    Dataset data("Freebase-3b", "/home/anabur/data/save/", "3b/", "3b/", "/home/anabur/Github/logs/loading_log/", false);
    TaskType task = General;
    MFactorE model(data, task, "/home/anabur/Github/logs/training_log/", 10, 0.01, 0.1, 0.01, 10, false);
    model.run(10000);
    model.test_link_prediction();
    return 0;
}