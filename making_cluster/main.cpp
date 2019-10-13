#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <string>
#include <mutex>
#include <cstdlib>
#include "/home/anabur/Github/include/PB/progress_bar.hpp"
#define CLUSTER_NUM 8
#define RADIUS 8

using namespace std;

const string PATH_NAME = "3b_reindexed";
const string BASE_DIR = "/home/anabur/data/save/" + PATH_NAME;
const unsigned int CLUSTER_SIZE_LOWER = 1e7;
const unsigned int CLUSTER_SIZE_UPPER = 2e7;
const int MAX_ROUND = 100;
const int LAST_MAX_ROUND = 100;

size_t make_graph_from_file(vector<vector<unsigned int>>& graph, string path_to_triples, string path_to_entities, string path_to_size);
void get_central_entities(unordered_set<unsigned int>& central_entities, string path);
void get_useless_entities(unordered_set<unsigned int>& useless_entities, string path);
void get_graph_size(vector<vector<unsigned int>>& graph, string path);
void group_radius(int cluster_no, unsigned int seed, bool* cluster, mutex& clu_mut,  vector<vector<unsigned int>>& graph, unordered_set<unsigned int>& central_entities, string base_dir, vector<unsigned int>& total_cluster_size);
void remove_cluster(int cluster_id, mutex& clu_mut, string base_dir, bool* cluster);
size_t compare_save_largest_cluster(int cluster_id, string base_dir, bool first_time);

int main() {
    vector<vector<unsigned int>> graph;
	unordered_set<unsigned int> central_entities;
	unordered_set<unsigned int> useless_entities;
	vector<vector<unsigned int> > results(CLUSTER_NUM, vector<unsigned int>());
    size_t entity_size = make_graph_from_file(graph, BASE_DIR + "/training.data", BASE_DIR + "/entities.data", BASE_DIR + "/graph_size.data");
    get_central_entities(central_entities, BASE_DIR + "/central_entities.data");
	get_useless_entities(useless_entities, BASE_DIR + "/garbage_entity/orphan_and_leaves.data");
    bool* cluster = new bool[entity_size];
    mutex cluster_mutex;
    vector<thread*> threads;
	vector<unsigned int> cluster_size(CLUSTER_NUM);
	for (unsigned int i = 0; i < entity_size; ++i) {
		cluster[i] = false;
	}

    std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> rand_num(0, graph.size());
    for (int i = 0; i < CLUSTER_NUM; ++i) {
        int seed = rand_num(rng);
        cluster_mutex.lock();
		while (useless_entities.find(seed) != useless_entities.end() || central_entities.find(seed) != central_entities.end() || cluster[seed] == true)
		{
			seed = rand_num(rng);
		}
        cluster[seed] = true;
        cluster_mutex.unlock();
        threads.push_back(new thread(group_radius, i, seed, cluster, ref(cluster_mutex), ref(graph), ref(central_entities), BASE_DIR, ref(cluster_size)));
    }

	int retry_cluster_num = 2;
	int cnt = 0;
	bool is_last = false;
	int last_id;
	while (retry_cluster_num > 1) {
		retry_cluster_num = 0;
		for (int i = 0; i < threads.size(); ++i) {
			if (threads[i] == nullptr) {
				continue;
			}

			threads[i]->join();
			delete threads[i];
			threads[i] = nullptr;
			if (cnt < MAX_ROUND && (cluster_size[i] < CLUSTER_SIZE_LOWER || cluster_size[i] > CLUSTER_SIZE_UPPER)) {
				++retry_cluster_num;
				last_id = i;
				remove_cluster(i, cluster_mutex, BASE_DIR, cluster);
				int seed = rand_num(rng);
				cluster_mutex.lock();
				while (useless_entities.find(seed) != useless_entities.end() || central_entities.find(seed) != central_entities.end() || cluster[seed] == true)
				{
					seed = rand_num(rng);
				}
				cluster[seed] = true;
        		cluster_mutex.unlock();
				threads[i] = new thread(group_radius, i, seed, cluster, ref(cluster_mutex), ref(graph), ref(central_entities), BASE_DIR, ref(cluster_size));
			} else {
				cout << "One finished. " << endl;
			}
		}
		++cnt;
		cout << cnt << "th round completed. " << endl;
	}

	if (retry_cluster_num == 1) {
		threads[last_id]->join();
	}

	cnt = 0;
	bool* tmp_cluster;
	while (retry_cluster_num == 1 && cnt < LAST_MAX_ROUND) {
		cout << "Last cluster mode for cluster " << last_id << " : " << cnt << endl;
		cluster_size[last_id] = compare_save_largest_cluster(last_id, BASE_DIR, !cnt);
		int seed = rand_num(rng);
		while (central_entities.find(seed) != central_entities.end() || cluster[seed] == true)
		{
			seed = rand_num(rng);
		}
		tmp_cluster = new bool[entity_size];
		for (unsigned int i = 0; i < entity_size; ++i) {
			tmp_cluster[i] = cluster[i];
		}
		group_radius(last_id, seed, tmp_cluster, cluster_mutex, graph, central_entities, BASE_DIR, cluster_size);
		delete tmp_cluster;
		if (cluster_size[last_id] > CLUSTER_SIZE_LOWER && cluster_size[last_id] < CLUSTER_SIZE_UPPER) {
			cout << "Found acceptable last cluster, exiting..." << endl;
			break;
		}
		++cnt;
	}
	if (cluster_size[last_id] < CLUSTER_SIZE_LOWER || cluster_size[last_id] > CLUSTER_SIZE_UPPER) {
		cluster_size[last_id] = compare_save_largest_cluster(last_id, BASE_DIR, !cnt);
	}

	ofstream summary_file(BASE_DIR + "/cluster/summary.csv");
	for (auto i: cluster_size) {
		summary_file << i << ",";
	}
	summary_file.close();
}

size_t make_graph_from_file(vector<vector<unsigned int>>& graph, string path_to_triples, string path_to_entities, string path_to_size="")
{
	ifstream triple_file(path_to_triples, ios_base::binary);
	ifstream entity_file(path_to_entities, ios_base::binary);

	size_t triple_size;
	triple_file.read((char*)& triple_size, sizeof(size_t));
	size_t entity_size;
	entity_file.read((char*)& entity_size, sizeof(size_t));
	entity_file.close();
	cout << "triple_size:" << triple_size << endl;
	cout << "entity_size:" << entity_size << endl;
	graph.resize(entity_size);
	if (path_to_size != "") {
		get_graph_size(graph, path_to_size);
	}
	ProgressBar prog_bar("Deserializing binary file to map:", triple_size);
	prog_bar.progress_begin();

	for (prog_bar.progress = 0; prog_bar.progress < triple_size && triple_file; ++prog_bar.progress)
	{
		unsigned int tri_arr[3];
		triple_file.read((char*)tri_arr, sizeof(unsigned int) * 3);
		graph[tri_arr[0]].push_back(tri_arr[2]);
		graph[tri_arr[2]].push_back(tri_arr[0]);
	}
	triple_file.close();

	prog_bar.progress_end();
	if (prog_bar.progress != triple_size)
	{
		cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
	}
	return entity_size;
}

void get_central_entities(unordered_set<unsigned int>& central_entities, string path)
{
	ifstream file(path, ios_base::binary);
	size_t set_size;
	file.read((char*)& set_size, sizeof(size_t));
	central_entities.reserve(set_size);
	ProgressBar prog_bar("Deserializing binary file to map:", set_size);
	prog_bar.progress_begin();

	for (prog_bar.progress = 0; prog_bar.progress < set_size && file; ++prog_bar.progress)
	{
		unsigned int id;
		file.read((char*)& id, sizeof(id));
		central_entities.insert(id);
	}
	file.close();
	prog_bar.progress_end();
	if (prog_bar.progress != set_size)
	{
		cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
	}
}

void get_useless_entities(unordered_set<unsigned int>& useless_entities, string path)
{
	ifstream file(path, ios_base::binary);
	size_t set_size;
	file.read((char*)& set_size, sizeof(size_t));
	useless_entities.reserve(set_size);
	ProgressBar prog_bar("Deserializing binary file to map:", set_size);
	prog_bar.progress_begin();

	for (prog_bar.progress = 0; prog_bar.progress < set_size && file; ++prog_bar.progress)
	{
		unsigned int id;
		file.read((char*)& id, sizeof(id));
		useless_entities.insert(id);
	}
	file.close();
	prog_bar.progress_end();
	if (prog_bar.progress != set_size)
	{
		cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
	}
}

void get_graph_size(vector<vector<unsigned int>>& graph, string path) {
	ifstream file(path, ios::binary);
	cout << "Getting graph size..." << endl;
	size_t graph_size;
	file.read((char*)&graph_size, sizeof(graph_size));
	if (graph_size != graph.size()) {
		cerr << "Data does not match with graph size. " << endl;
		exit;
	}

	for (auto i = 0; i < graph_size; ++i) {
		size_t size;
		file.read((char*)&size, sizeof(size));
		graph[i].reserve(size);
	}
	file.close();
}

void group_radius(int cluster_no, unsigned int seed, bool* cluster, mutex& clu_mut, vector<vector<unsigned int>>& graph, unordered_set<unsigned int>& central_entities, string base_dir, vector<unsigned int>& total_cluster_size)
{
    
	ofstream cluster_file(base_dir + "/cluster/" + to_string(cluster_no) + ".data", ios::binary);
    ofstream cluster_layer_size(base_dir + "/cluster/" + to_string(cluster_no) + ".csv");
	unordered_set<unsigned int> lc_0;
	unordered_set<unsigned int> lc_1;
	unordered_set<unsigned int>* layer_cluster_0 = &lc_0;
	unordered_set<unsigned int>* layer_cluster_1 = &lc_1;
    vector<unsigned int> result;
    size_t cluster_size = 1;
	layer_cluster_0->insert(seed);
    cluster_file.write((char*)&cluster_size, sizeof(cluster_size));
    cluster_file.write((char*)&seed, sizeof(seed));

	for (int i = 0; i < RADIUS; ++i)
	{
		cout << seed << ", RADIUS = " << i + 1 << endl;
		if (i % 2) {
			layer_cluster_0 = &lc_1;
			layer_cluster_1 = &lc_0;
		}
		else {
			layer_cluster_0 = &lc_0;
			layer_cluster_1 = &lc_1;
		}
		for (auto j = layer_cluster_0->begin(); j != layer_cluster_0->end(); j++)
		{
			for (auto k = graph[*j].begin(); k != graph[*j].end(); ++k)
			{
				if (central_entities.find(*k) != central_entities.end())
				{
					continue;
				}
                clu_mut.lock();
				if (cluster[*k] == false)
				{
					cluster[*k] = true;
                    clu_mut.unlock();
					layer_cluster_1->insert(*k);
				} else {
                    clu_mut.unlock();
                }
			}
		}
		layer_cluster_0->clear();
		result.push_back(layer_cluster_1->size());
        for (auto node: *layer_cluster_1) {
            cluster_file.write((char*)&node, sizeof(node));
        }
	}
    for (auto i: result) {
        cluster_size += i;
    }
    cluster_file.seekp(ios::beg);
    cluster_file.write((char*)&cluster_size, sizeof(cluster_size));
    cluster_file.close();
	total_cluster_size[cluster_no] = cluster_size;
    for (auto i: result) {
        cluster_layer_size << i << ",";
    }
    cluster_layer_size.close();
}

void remove_cluster(int cluster_id, mutex& clu_mut, string base_dir, bool* cluster) {
	ifstream cluster_file(base_dir + "/cluster/" + to_string(cluster_id) + ".data", ios::binary);
	cout << "Removing cluster: " << cluster_id << endl;
	size_t cluster_size;
	cluster_file.read((char*)&cluster_size, sizeof(cluster_size));

	for (size_t i = 0; i < cluster_size; ++i) {
		unsigned int entity_id;
		cluster_file.read((char*)&entity_id, sizeof(entity_id));
		clu_mut.lock();
		cluster[entity_id] = false;
		clu_mut.unlock();
	}
	cluster_file.close();
}

size_t compare_save_largest_cluster(int cluster_id, string base_dir, bool first_time) {
	if (first_time) {
		ifstream latest_file(base_dir + "/cluster/" + to_string(cluster_id) + ".data", ios::binary);
		size_t latest_size;
		latest_file.read((char*)&latest_size, sizeof(latest_size));
		latest_file.close();
		rename((base_dir + "/cluster/" + to_string(cluster_id) + ".data").c_str(), (base_dir + "/cluster/" + to_string(cluster_id) + ".largest").c_str());
		return latest_size;
	}

	ifstream latest_file(base_dir + "/cluster/" + to_string(cluster_id) + ".data", ios::binary);
	size_t latest_size;
	latest_file.read((char*)&latest_size, sizeof(latest_size));
	ifstream largest_file(base_dir + "/cluster/" + to_string(cluster_id) + ".largest", ios::binary);
	size_t largest_size;
	largest_file.read((char*)&largest_size, sizeof(largest_size));
	latest_file.close();
	largest_file.close();
	if (largest_size < latest_size) {
		remove((base_dir + "/cluster/" + to_string(cluster_id) + ".largest").c_str());
		rename((base_dir + "/cluster/" + to_string(cluster_id) + ".data").c_str(), (base_dir + "/cluster/" + to_string(cluster_id) + ".largest").c_str());
	}
	return largest_size > latest_size ? largest_size: latest_size;
}
