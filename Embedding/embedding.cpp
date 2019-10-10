#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"

using namespace std;

int main()
{
    srand(time(nullptr));
    Dataset data("3b_reindexed", "/home/anabur/data/save/", "3b_reindexed/training.data", "3b_reindexed/testing.data", "/home/anabur/Github/logs/loading_log/", false);
    TaskType task = LinkPredictionHead;
    MFactorE model(data, task, "/home/anabur/Github/logs/training_log/", 10, 0.01, 0.1, 0.01, 10, true);
    model.run(10000, 39);
    model.save("/home/anabur/data/model/3b_reindexed.model");
    // model.load("/home/anabur/data/model/latest-lexemes.model");
    model.test_link_prediction(10, 0, 39);
    return 0;
}
