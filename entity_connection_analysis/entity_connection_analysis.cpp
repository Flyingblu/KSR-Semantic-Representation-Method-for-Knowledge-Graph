#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include "/home/anabur/Github/include/PB/progress_bar.hpp"
#define RADIUS 5
#define BATCH 10

using namespace std;

const string path_name = "3b_reindexed";

size_t make_graph_from_file(vector<vector<unsigned int>>& graph, string path_to_triples, string path_to_entities, string path_to_size);
void get_central_entities(unordered_set<unsigned int>& central_entities, string path);
void group_radius(size_t entity_size, unsigned int seed, vector<vector<unsigned int>>& graph, unordered_set<unsigned int>& central_entities, vector<unsigned int>& result);
void save_graph_size(vector<vector<unsigned int>>& graph);
void get_graph_size(vector<vector<unsigned int>>& graph, string path);

int main()
{
	vector<vector<unsigned int>> graph;
	unordered_set<unsigned int> central_entities;
	vector<vector<unsigned int> > results(BATCH, vector<unsigned int>());
	vector<thread*> threads;
	size_t entity_size = make_graph_from_file(graph, "/home/anabur/data/save/" + path_name + "/triples.data", "/home/anabur/data/save/" + path_name + "/entities.data", "/home/anabur/data/save/" + path_name + "/graph_size.data");
	// save_graph_size(graph);
	get_central_entities(central_entities, "/home/anabur/data/save/" + path_name + "/central_entities.data");
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> rand_num(0, graph.size());

	ofstream csv("analysis.csv");
	csv << "Run,";
	for (int i = 0; i < RADIUS + 1; ++i)
	{
		csv << "Layer " << i << ",";
	}
	csv << endl;
	for (int q = 0; q < BATCH; ++q)
	{
		int seed = rand_num(rng);
		while (central_entities.find(seed) != central_entities.end())
		{
			seed = rand_num(rng);
		}

		threads.push_back(new thread(group_radius, entity_size, seed, ref(graph), ref(central_entities), ref(results[q])));
	}

	for (auto i : threads)
	{
		i->join();
	}
	for (int i = 0; i < results.size(); ++i) {
		csv << i << ",";
		for (auto j : results[i])
		{
			csv << j << ",";
		}
		csv << endl;
	}
	csv.close();
	return 0;
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

void save_graph_size(vector<vector<unsigned int>>& graph) {
	ofstream file("/home/anabur/data/save/" + path_name + "/graph_size.data", ios::binary);
	cout << "Saving graph size..." << endl;
	size_t graph_size = graph.size();
	file.write((char*)&graph_size, sizeof(graph_size));
	for (auto i: graph) {
		size_t size = i.size();
		file.write((char*)&size, sizeof(size));
	}
	file.close();
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

void group_radius(size_t entity_size, unsigned int seed,  vector<vector<unsigned int>>& graph, unordered_set<unsigned int>& central_entities, vector<unsigned int>& result)
{
	bool* cluster = new bool[entity_size];
	for (unsigned int i = 0; i < graph.size(); ++i) {
		cluster[i] = false;
	}

	cluster[seed] = true;
	unordered_set<unsigned int> lc_0;
	unordered_set<unsigned int> lc_1;
	unordered_set<unsigned int>* layer_cluster_0 = &lc_0;
	unordered_set<unsigned int>* layer_cluster_1 = &lc_1;
	layer_cluster_0->insert(seed);

	unsigned int last_layer_size = 0;
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

				if (cluster[*k] == false)
				{
					cluster[*k] = true;
					if (i == RADIUS - 1) {
						++last_layer_size;
					} else {
						layer_cluster_1->insert(*k);
					}
				}
			}
		}
		layer_cluster_0->clear();
		if (i == RADIUS - 1) {
			result.push_back(last_layer_size);
		} else {
			result.push_back(layer_cluster_1->size());
		}
	}
}