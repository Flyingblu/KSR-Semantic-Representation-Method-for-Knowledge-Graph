#include <fstream>
#include <iostream>
#include <unordered_map>
#include "/home/anabur/Github/include/PB/progress_bar.hpp"

using namespace std;

int main() {
	ifstream entity_with_string("/home/anabur/data/save/3b/entities.data", ios_base::binary);
	ifstream triple_file("/home/anabur/data/save/3b/triple_shuffled_filtered.data", ios_base::binary);
	ifstream entity_file("/home/anabur/data/save/3b/entities_more5.data", ios_base::binary);
	ofstream new_triple("/home/anabur/data/save/3b_reindexed/triples.data", ios_base::binary);
	ofstream new_entity("/home/anabur/data/save/3b_reindexed/entities.data", ios_base::binary);
	
	unordered_map<unsigned int, unsigned int> entity_map;

	size_t entity_size;
	entity_file.read((char*)& entity_size, sizeof(size_t));
	new_entity.write((char*)& entity_size, sizeof(size_t));
	entity_map.reserve(entity_size);
	cout << "Entity size: " << entity_size << endl;
	ProgressBar prog_bar("Reading and indexing entity file", entity_size);
	prog_bar.progress_begin();

	unsigned int cnt = 0;
	for (prog_bar.progress = 0; prog_bar.progress < entity_size && entity_file; ++prog_bar.progress) {
		unsigned int id;
		entity_file.read((char*)& id, sizeof(unsigned int));

		entity_map[id] = cnt;
		++cnt;
	}
	prog_bar.progress_end();
	if (prog_bar.progress != entity_size) {
		if(!entity_file) {
			cerr << "Entity file error. " << endl;
		} else if (!new_entity) {
			cerr << "New entity file error. " << endl;
		} else {
			cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
		}
	}
	entity_file.close();

	size_t total_entity_size;
	entity_with_string.read((char*)& total_entity_size, sizeof(size_t));
	cout << "Entity size: " << total_entity_size << endl;
	ProgressBar prog_bar1("Reading and writing entity file", total_entity_size);
	prog_bar1.progress_begin();

	for (prog_bar1.progress = 0; prog_bar1.progress < total_entity_size && entity_with_string; ++prog_bar1.progress) {
		int size;
		unsigned int id;
		entity_with_string.read((char*)& size, sizeof(int));
		char* tmp_str = new char[size];

		entity_with_string.read((char*)tmp_str, size);
		entity_with_string.read((char*)& id, sizeof(unsigned int));

		auto new_id = entity_map.find(id);
		if (new_id != entity_map.end()) {
			new_entity.write((char*)& size, sizeof(int));
			new_entity.write((char*)tmp_str, size);
			new_entity.write((char*)&new_id->second, sizeof(unsigned int));
		}

		delete[] tmp_str;
	}
	prog_bar1.progress_end();
	if (prog_bar1.progress != total_entity_size) {
		if(!entity_with_string) {
			cerr << "Entity file error. " << endl;
		} else if (!new_entity) {
			cerr << "New entity file error. " << endl;
		} else {
			cerr << "map_deserialize: Something wrong in binary file reading. " << endl;
		}
	}
	entity_with_string.close();
	new_entity.close();

	size_t vector_size;
	triple_file.read((char*)& vector_size, sizeof(size_t));
	new_triple.write((char*)& vector_size, sizeof(size_t));
	cout << "Triple size: " << vector_size << endl;
	ProgressBar prog_bar2("Reading and writing triple file", vector_size);
	prog_bar2.progress_begin();

	for (prog_bar2.progress = 0; prog_bar2.progress < vector_size && triple_file; ++prog_bar2.progress) {

		unsigned int tri_arr[3];
		triple_file.read((char*)tri_arr, sizeof(unsigned int) * 3);
		tri_arr[0] = entity_map[tri_arr[0]];
		tri_arr[2] = entity_map[tri_arr[2]];
		new_triple.write((char*)tri_arr, sizeof(unsigned int) * 3);
	}
	triple_file.close();
	new_triple.close();
	prog_bar2.progress_end();

	if (prog_bar2.progress != vector_size) {
		cerr << "triple_deserialize: Something wrong in binary file reading. " << endl;
	}
}