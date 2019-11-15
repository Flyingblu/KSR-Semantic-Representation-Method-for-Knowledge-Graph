#include "Import.hpp"
#include "DataModel.hpp"
#include "LatentModel.hpp"
#include "ModelConfig.hpp"
#include "Model.hpp"
#define NUM_CLUSTER 8
#define NUM_FALSE 2

using namespace std;

int main()
{
    srand(time(nullptr));

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
        dataset.push_back(new Dataset("latest-lexemes" + to_string(i), "/home/anabur/data/save/latest-lexemes/", "triples_cluster/" + file_name, "", rel_type, training_false, "/home/anabur/Github/logs/loading_log/", false));
    }
    vector<string> training_false;

    //Single dataset
    /*
    vector<string> training_false;
    for (int j = 1; j < 1 + NUM_FALSE; ++j)
    {
        training_false.push_back("false_triplet/" + string("false_triplet_") + to_string(j) + ".data");
    }
    //dataset.push_back(new Dataset("latest_lexemes", "/home/anabur/data/save/latest-lexemes/", "training_.data", "", "training_rel_type.data", training_false, "/home/anabur/Github/logs/latest-lexemes_loading_log/", false));
    */

    Dataset *test_dataset = new Dataset("latest-lexemes", "/home/anabur/data/save/latest-lexemes/", "", "testing_.data", "training_rel_type.data", training_false, "/home/anabur/Github/logs/latest-lexemes_loading_log/", false);
    TaskType task = General;
    //MFactorE model("/home/anabur/data/save/latest-lexemes/entities.data", "/home/anabur/data/save/latest-lexemes/properties.data", task, "/home/anabur/Github/logs/latest-lexemes_log/", 5, 0.01, 0.1, 0.01, 10, &dataset, 1);
    MFactorE model("/home/anabur/data/save/latest-lexemes/entities.data", "/home/anabur/data/save/latest-lexemes/properties.data", task, "/home/anabur/Github/logs/latest-lexemes_log/", 5, 0.01, 0.1, 0.01, 10, nullptr, 3, test_dataset);
    //model.run(2000, 19, dataset);
    //model.save("/home/anabur/data/model/latest-lexemes/");
    model.load("/home/anabur/data/model/latest-lexemes/");
    model.test_link_prediction(10, 0, 19);
}