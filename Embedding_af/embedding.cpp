#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel_af.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"
#define NUM_CLUSTER 8
#define NUM_FALSE 2

using namespace std;

int main()
{
    srand(time(nullptr));
    string name_dataset = "3b_reindexed";

    //Cluster dataset
    vector<Dataset *> dataset;
    for (int i = 0; i < NUM_CLUSTER; ++i)
    {
        vector<string> training_false;
        for (int j = 1; j < 1 + NUM_FALSE; ++j)
        {
            training_false.push_back("false_triplet/" + to_string(i) + "_false_triplet_" + to_string(j) + ".data");
        }
        string file_name = to_string(i) + "_triples.data";
        string rel_type = "training_rel_type.data";
        dataset.push_back(new Dataset(name_dataset + to_string(i), "/home/anabur/data/save/" + name_dataset + "/", "triples_cluster/" + file_name, "", rel_type, training_false, "/home/anabur/Github/logs/loading_log/", false));
    }
    vector<string> training_false;

    //Single dataset
    /*
    vector<string> training_false;
    for (int j = 1; j < 1 + NUM_FALSE; ++j)
    {
        training_false.push_back("false_triplet/" + string("false_triplet_") + to_string(j) + ".data");
    }
    //dataset.push_back(new Dataset(name_dataset, "/home/anabur/data/save/" + name_dataset + "/", "training_.data", "", "training_rel_type.data", training_false, "/home/anabur/Github/logs/latest-lexemes_loading_log/", false));
    */

    Dataset *test_dataset = new Dataset(name_dataset, "/home/anabur/data/save/" + name_dataset + "/", "", "testing_.data", "training_rel_type.data", training_false, "/home/anabur/Github/logs/" + name_dataset + "_loading_log/", false);
    TaskType task = General;
    //MFactorE model("R^(/home/anabur/data/save/)" + name_dataset + "/entities.data", "/home/anabur/data/save/" + name_dataset + "/properties.data", task, "/home/anabur/Github/logs/" + name_dataset + "_log/","/home/anabur/data/model/" + name_dataset + "/", 5, 0.01, 0.1, 0.01, 10, &dataset, 3);
    //MFactorE model("/home/anabur/data/save/" + name_dataset + "/entities.data", "/home/anabur/data/save/" + name_dataset + "/properties.data", task, "/home/anabur/Github/logs/" + name_dataset + "_log/","/home/anabur/data/model/" + name_dataset + "/", 5, 0.01, 0.1, 0.01, 10, nullptr, 3, test_dataset);
    //model.load("/home/anabur/data/model/" + name_dataset + "/");
    //model.run(2000, 19, dataset);
    //model.save("/home/anabur/data/model/" + name_dataset + "/");
    //model.load("/home/anabur/data/model/" + name_dataset +"/");
    //model.test_link_prediction(10, 0, 19);
}