#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"
#define NUM_CLUSTER 8

using namespace std;

int main()
{
    srand(time(nullptr));
    vector<Dataset*> dataset;
    for (int i = 0; i < NUM_CLUSTER; ++i) {
        string file_name = to_string(i) + ".data";
        dataset.push_back(new Dataset("3b_reindexed", "/home/anabur/data/save/3b_reindexed/", "cluster_triples/" + file_name, "", "/home/anabur/Github/logs/loading_log/", false));
    }
    TaskType task = LinkPredictionHead;
    MFactorE model("/home/anabur/data/save/3b_reindexed/entities.data", "/home/anabur/data/save/3b_reindexed/properties.data", task, "/home/anabur/Github/logs/training_log/", 10, 0.01, 0.1, 0.01, 10, true);
    model.run(10000, 39, dataset);
    model.save("/home/anabur/data/model/3b_reindexed.model");
    // model.load("/home/anabur/data/model/latest-lexemes.model");
    model.test_link_prediction(10, 0, 39);
    return 0;
}
