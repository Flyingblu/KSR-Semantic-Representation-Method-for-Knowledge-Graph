#pragma once
#include "Import.hpp"
#include "ModelConfig.hpp"
#include "../RDF_parser/progress_bar.hpp"

class DataModel
{
public:
	set<pair<pair<unsigned int, unsigned int>, unsigned int>> check_data_train;
	set<pair<pair<unsigned int, unsigned int>, unsigned int>> check_data_test;

public:
	vector<pair<pair<unsigned int, unsigned int>, unsigned int>> data_train;
	vector<pair<pair<unsigned int, unsigned int>, unsigned int>> data_test_true;

public:
	size_t entity_size;
	size_t relation_size;

public:
	vector<char> relation_type;

public:
	vector<double> relation_tph;
	vector<double> relation_hpt;
public: 
	mutex data_train_mtx;
	mutex check_train_mtx;
	mutex rel_heads_mtx;
	mutex rel_tails_mtx;

public:
	DataModel(const Dataset *dataset, bool do_load_testing, size_t entity_size, size_t relation_size): entity_size(entity_size), relation_size(relation_size)
	{
		// TODO: these two maps seem to be huge, see if we can optimize it.
		map<unsigned int, map<unsigned int, vector<unsigned int>>> rel_heads;
		map<unsigned int, map<unsigned int, vector<unsigned int>>> rel_tails;
		load_training(dataset->base_dir + dataset->training, rel_heads, rel_tails, 3);
		relation_hpt.resize(relation_size);
		relation_tph.resize(relation_size);
		for (auto i = 0; i != relation_size; ++i)
		{
			double sum = 0;
			double total = 0;
			for (auto ds = rel_heads[i].begin(); ds != rel_heads[i].end(); ++ds)
			{
				++sum;
				total += ds->second.size();
			}
			relation_tph[i] = total / sum;
		}
		for (auto i = 0; i != relation_size; ++i)
		{
			double sum = 0;
			double total = 0;
			for (auto ds = rel_tails[i].begin(); ds != rel_tails[i].end(); ++ds)
			{
				++sum;
				total += ds->second.size();
			}
			relation_hpt[i] = total / sum;
		}

		if (do_load_testing)
		{
			load_testing(dataset->base_dir + dataset->testing, data_test_true);
		}

		double threshold = 1.5;
		relation_type.resize(relation_size);

		for (auto i = 0; i < relation_size; ++i)
		{
			if (relation_tph[i] < threshold && relation_hpt[i] < threshold)
			{
				relation_type[i] = 1;
			}
			else if (relation_hpt[i] < threshold && relation_tph[i] >= threshold)
			{
				relation_type[i] = 2;
			}
			else if (relation_hpt[i] >= threshold && relation_tph[i] < threshold)
			{
				relation_type[i] = 3;
			}
			else
			{
				relation_type[i] = 4;
			}
		}
	}

	/**
	 * @brief Load the training dataset into memory. The dataset needs to be a binary file. It 
	 * starts with the total number
	 * of triples it contains, followed with each head-relation-tail triples expressed in three 
	 * unsigned int. The dataset generated by RDF_parser can be directly loaded by this method. 
	 * 
	 * @param file_path : File path to triple data file
	 * 
	 */

	void load_training(const string &file_path,
					   map<unsigned int, map<unsigned int, vector<unsigned int>>> &rel_heads,
					   map<unsigned int, map<unsigned int, vector<unsigned int>>> &rel_tails, int num_stream)
	{
		vector<ifstream*> streams(num_stream); 
		for (int i = 0; i < num_stream; i++)
		{
			ifstream* fin = new ifstream(file_path, ios_base::binary);
			streams[i] = fin;
		}
		
		size_t triple_size;
		streams[0]->read((char *)&triple_size, sizeof(size_t));
		data_train.reserve(triple_size);

		size_t start = 1 * sizeof (size_t);
		size_t range = triple_size / num_stream * sizeof(unsigned int) * 3;
		

		thread* threads[num_stream];
		for (int i = 0; i < num_stream; ++i)
		{	
			if( i == num_stream - 1) 
			{
				threads[i] = new thread(&DataModel::load_train_slice, this, ref(streams[i]), start, triple_size * sizeof(unsigned int) * 3 - start, ref(rel_heads), ref(rel_tails), i);
				continue;
			}
			threads[i] = new thread(&DataModel::load_train_slice, this, ref(streams[i]), start, range, ref(rel_heads), ref(rel_tails), i);
			start += range;
		}
		
		
		for (int i = 0; i < num_stream; ++i)
		{
			threads[i]->join();
			delete threads[i];
			delete streams[i];
		}
	}

	void load_testing(const string &file_path,
					  vector<pair<pair<unsigned int, unsigned int>, unsigned int>> &vin_true)
	{
		ifstream triple_file(file_path, ios_base::binary);

		size_t triple_size;
		triple_file.read((char *)&triple_size, sizeof(size_t));
		// TODO: change api
		ProgressBar prog_bar("Deserializing binary file to triples:", triple_size);
		prog_bar.progress_begin();

		for (prog_bar.progress = 0; prog_bar.progress < triple_size && triple_file; ++prog_bar.progress)
		{

			unsigned int tri_arr[3];
			triple_file.read((char *)tri_arr, sizeof(unsigned int) * 3);

			vin_true.push_back(make_pair(make_pair(tri_arr[0], tri_arr[2]), tri_arr[1]));
			check_data_test.insert(make_pair(make_pair(tri_arr[0], tri_arr[2]), tri_arr[1]));

		}
		triple_file.close();
		prog_bar.progress_end();

		if (prog_bar.progress != triple_size)
		{
			cerr << "triple_deserialize: Something wrong in binary file reading. " << endl;
		}
	}

	void sample_false_triplet(
		const pair<pair<unsigned int, unsigned int>, unsigned int> &origin,
		pair<pair<unsigned int, unsigned int>, unsigned int> &triplet) const
	{

		double prob = relation_hpt[origin.second] / (relation_hpt[origin.second] + relation_tph[origin.second]);

		triplet = origin;
		while (true)
		{
			if (rand() % 1000 < 1000 * prob)
			{
				triplet.first.second = rand() % entity_size;
			}
			else
			{
				triplet.first.first = rand() % entity_size;
			}

			if (check_data_train.find(triplet) == check_data_train.end())
				return;
		}
	}

	void load_train_slice(ifstream* fin, size_t start, size_t range, 
						  map<unsigned int, map<unsigned int, vector<unsigned int>>> &rel_heads, 
						  map<unsigned int, map<unsigned int, vector<unsigned int>>> &rel_tails, 
						  int slice_index)
	{
		fin->seekg(start, std::ios_base::beg);
		for (unsigned int i = 0; i < range; ++i)
		{
			unsigned int tri_arr[3];
			fin->read((char *)tri_arr, sizeof(unsigned int) * 3);

			auto pr = make_pair(make_pair(tri_arr[0], tri_arr[2]), tri_arr[1]);

			data_train_mtx.lock();
			data_train.push_back(pr);
			data_train_mtx.unlock();

			check_train_mtx.lock();
			check_data_train.insert(pr);
			check_train_mtx.unlock();

			rel_heads_mtx.lock();
			rel_heads[tri_arr[1]][tri_arr[0]].push_back(tri_arr[2]);
			rel_heads_mtx.unlock();

			rel_tails_mtx.lock();
			rel_tails[tri_arr[1]][tri_arr[2]].push_back(tri_arr[0]);
			rel_tails_mtx.unlock();
		}
		fin->close();
		cout << "read slice " << slice_index << "completed" << endl;
	}
};