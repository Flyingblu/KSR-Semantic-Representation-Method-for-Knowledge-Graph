#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>
#include </home/anabur/Github/include/PB/progress_bar.hpp>

using namespace std;

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
	cout << "central_entities_size : " << set_size << endl;
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


int main()
{
    ofstream orphan_file("/home/anabur/data/save/latest-lexemes/garbage_entity/orphan_entities.data", ios_base::binary);
    ofstream leaves_file("/home/anabur/data/save/latest-lexemes/garbage_entity/leaves_entities.data", ios_base::binary);
    ofstream orphan_leaves_file("/home/anabur/data/save/latest-lexemes/garbage_entity/orphan_and_leaves.data", ios_base::binary);
    vector<vector<unsigned int>> graph;

    unordered_set<unsigned int> orphan_entities;
    unordered_set<unsigned int> leaves_entities;
    unordered_set<unsigned int> central_entities;

    make_graph_from_file(graph, "/home/anabur/data/save/latest-lexemes/triples.data", "/home/anabur/data/save/latest-lexemes/entities.data", "/home/anabur/data/save/latest-lexemes/graph_size.data");
    get_central_entities(central_entities, "/home/anabur/data/save/latest-lexemes/central_entities.data");

    size_t graph_size = graph.size();
	
	ProgressBar pro_bar1("Finding Orphan and Leaves entities ...", graph_size);
	pro_bar1.progress_begin();
    for (unsigned int i = 0; i < graph_size; ++i)
    {
        pro_bar1.progress++;
        if (graph[i].size() == 0)
        {
            orphan_entities.insert(i);
            continue;
        }
        unsigned int cnt = 0;

        for(auto j: graph[i])
        {
            if (central_entities.count(j) == 1)
            {
                ++cnt;
            }
        }
        if (cnt == graph[i].size())
        {
            leaves_entities.insert(i);
        }
    }
	pro_bar1.progress_end();

    size_t orphan_size = orphan_entities.size();
    orphan_file.write((char*)&orphan_size, sizeof(size_t));
	cout << "orphan size: " << orphan_size << endl;

    size_t leaves_size = leaves_entities.size();
    leaves_file.write((char*)&leaves_size, sizeof(size_t));
	cout << "leaves size: " << leaves_size << endl;

    size_t orphan_leaves_size = orphan_size + leaves_size;
    orphan_leaves_file.write((char*)&orphan_leaves_size, sizeof(size_t));
    cout << "orphan_leaves_size: " << orphan_leaves_size << endl;
    
    ProgressBar pro_bar2("Deserializing orphan to binary ... ", orphan_size);
	pro_bar2.progress_begin();
    for (auto i: orphan_entities)
    {
        orphan_file.write((char*)&i, sizeof(unsigned int));
        orphan_leaves_file.write((char*)&i, sizeof(unsigned int));
		pro_bar2.progress++;
    }
    orphan_file.close();
	pro_bar2.progress_end();
    
	ProgressBar pro_bar3("Deserializing Leaves to binary ... ", leaves_size);
	pro_bar3.progress_begin();
    for (auto i: leaves_entities)
    {
        leaves_file.write((char*)&i, sizeof(unsigned int));
        orphan_leaves_file.write((char*)&i, sizeof(unsigned int));
		pro_bar3.progress++;
	}
    leaves_file.close();
    orphan_leaves_file.close();
	pro_bar3.progress_end();
}