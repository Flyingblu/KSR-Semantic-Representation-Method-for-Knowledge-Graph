#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <tuple>
#include <thread>
#include <mutex>
#include </home/anabur/Github/include/PB/progress_bar.hpp>
#define CLUSTER_NUM 8

using namespace std;

mutex prog_bar_mtx;

void get_central_entities(unordered_set<unsigned int> &central_entities, string path)
{
    ifstream file(path, ios_base::binary);
    size_t set_size;
    file.read((char *)&set_size, sizeof(size_t));
    cout << "central_entities_size : " << set_size << endl;
    central_entities.reserve(set_size);
    ProgressBar prog_bar("Deserializing binary file to map:", set_size);
    prog_bar.progress_begin();

    for (prog_bar.progress = 0; prog_bar.progress < set_size && file; ++prog_bar.progress)
    {
        unsigned int id;
        file.read((char *)&id, sizeof(id));
        central_entities.insert(id);
    }
    file.close();
    prog_bar.progress_end();
    if (prog_bar.progress != set_size)
    {
        cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
    }
}

void get_entities_cluster(vector<unordered_set<unsigned int>> &entities_clusters, string entites_cluster_base_dir, string central_entities_path, int cluster_num)
{

    unordered_set<unsigned int> central_entities;
    get_central_entities(central_entities, central_entities_path);

    entities_clusters.resize(cluster_num);
    for (int i = 0; i < cluster_num; ++i)
    {
        string s = to_string(i);
        ifstream entity_cluster_read(entites_cluster_base_dir + s.c_str() + ".data", ios_base::binary);

        size_t cluster_size;
        entity_cluster_read.read((char *)&cluster_size, sizeof(size_t));

        ProgressBar prog_bar("loading entities cluster " + s + " ...", cluster_size);
        prog_bar.progress_begin();
        for (unsigned int j = 0; j < cluster_size; ++j)
        {
            unsigned int id;
            entity_cluster_read.read((char *)&id, sizeof(unsigned int));
            entities_clusters[i].insert(id);
            prog_bar.progress++;
        }
        prog_bar.progress_end();
        entities_clusters[i].insert(central_entities.begin(), central_entities.end());
        entity_cluster_read.close();
    }
}

void find_edge(vector<unordered_set<unsigned int>> &entities_clusters, vector<tuple<unsigned int, unsigned int, unsigned int>> &triples, vector<unsigned int> &triplet_cluster_size, int i, string triple_cluster_base_dir)
{
    string s = to_string(i);
    ofstream out(triple_cluster_base_dir + s.c_str() + "_triples.data", ios_base::binary);
    size_t triple_cluster_size = 0;
    out.write((char *)&triple_cluster_size, sizeof(size_t));
    for (auto j : triples)
    {
        unsigned int tri_arr[3] = {get<0>(j), get<1>(j), get<2>(j)};

        if (entities_clusters[i].count(tri_arr[0]) == 1 && entities_clusters[i].count(tri_arr[2]) == 1)
        {
            out.write((char *)tri_arr, sizeof(unsigned int) * 3);
            triple_cluster_size++;
        }
    }
    triplet_cluster_size[i] = triple_cluster_size;
    out.seekp(0, ios_base::beg);
    out.write((char *)&triple_cluster_size, sizeof(size_t));
    out.close();
    cout << "triples cluster " + s + " finished " << endl;
}

int main()
{

    vector<unordered_set<unsigned int>> entities_clusters;
    //vector<vector<tuple<unsigned int, unsigned int, unsigned int>>> triples_clusters(CLUSTER_NUM);
    vector<tuple<unsigned int, unsigned int, unsigned int>> triples(CLUSTER_NUM);
    vector<unsigned int> triplet_cluster_size(CLUSTER_NUM); //vector storing size of every triple cluster
    string triple_cluster_base_dir = "/home/anabur/data/save/3b_reindexed/triples_cluster/";

    get_entities_cluster(entities_clusters, "/home/anabur/data/save/3b_reindexed/cluster/", "/home/anabur/data/save/3b_reindexed/central_entities.data", CLUSTER_NUM);

    ifstream triple_read("/home/anabur/data/save/3b_reindexed/triples.data", ios_base::binary);

    size_t triple_size;
    triple_read.read((char *)&triple_size, sizeof(size_t));
    cout << "triple_size : " << triple_size << endl;
    triples.resize(triple_size);
    ProgressBar prog_bar("Loading triples ... ", triple_size);
    prog_bar.progress_begin();
    for (unsigned int i = 0; i < triple_size; ++i)
    {
        unsigned int tri_arr[3];
        triple_read.read((char *)tri_arr, sizeof(unsigned int) * 3);
        triples[i] = make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]);
        /*
        thread *threads[CLUSTER_NUM];
        

        for (int j = 0; j < CLUSTER_NUM; ++j)
        {
            
            thread *th = new thread(find_edge, ref(entities_clusters), ref(triples_clusters), j, ref(tri_arr));
            threads[j] = th;
        }
        for (int j = 0; j < CLUSTER_NUM; ++j)
        {
            (*threads[j]).join();
        }
        */
        /*
        for (int j = 0; j < CLUSTER_NUM; ++j)
        {
            if (entities_clusters[j].count(tri_arr[0]) == 1 && entities_clusters[j].count(tri_arr[2]) == 1)
            {
                triples_clusters[j].push_back(make_tuple(tri_arr[0], tri_arr[1], tri_arr[2]));
            }
        }
        */
        prog_bar.progress++;
    }
    triple_read.close();
    prog_bar.progress_end();

    thread *threads[CLUSTER_NUM];
    cout << "Dividing triples into clusters ..." << endl;
    for (int i = 0; i < CLUSTER_NUM; ++i)
    {

        thread *th = new thread(find_edge, ref(entities_clusters), ref(triples), ref(triplet_cluster_size), i, triple_cluster_base_dir);
        threads[i] = th;
    }
    for (int i = 0; i < CLUSTER_NUM; ++i)
    {
        (*threads[i]).join();
    }
    
    
    size_t total_cluster_size = 0;
    ofstream triples_cluster_size_write(triple_cluster_base_dir + "triples_size.txt");
    for (int i = 0; i < CLUSTER_NUM; ++i)
    {
        triples_cluster_size_write << i << ", " << triplet_cluster_size[i] << endl;
        cout << i << ", " << triplet_cluster_size[i] << endl;
        total_cluster_size += triplet_cluster_size[i];
    }
    triples_cluster_size_write.close();

    cout << "Total cluster size: " << total_cluster_size << endl;
    cout << "triple_size : " << triple_size << endl;
    if (total_cluster_size != triple_size)
    {
        cout << "Total_cluster_size is not matched with triples_size " << endl;
        return 0;
    }

    /*
    ProgressBar prog_bar2("Deserializing triples cluster to binary ...", total_cluster_size);
    prog_bar2.progress_begin();
    for (int i = 0; i < CLUSTER_NUM; ++i)
    {
        string s = to_string(i);
        ofstream triples_cluster_write(triples_cluster_base_dir + s.c_str() + "_triples.data", ios_base::binary);

        size_t triple_cluster_size = triples_clusters[i].size();
        triples_cluster_write.write((char *)&triple_cluster_size, sizeof(size_t));
        for (auto j : triples_clusters[i])
        {
            unsigned int tri_arr[3] = {get<0>(j), get<1>(j), get<2>(j)};
            triples_cluster_write.write((char *)tri_arr, sizeof(unsigned int) * 3);
            prog_bar2.progress++;
        }
        triples_cluster_write.close();
    }
    prog_bar2.progress_end();
    */
    return 0;
}
